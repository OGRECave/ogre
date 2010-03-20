/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreShaderGLSLESProgramWriter.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreStringConverter.h"
#include "OgreShaderGenerator.h"
#include "OgreRoot.h"

namespace Ogre {
    namespace RTShader {

        String GLSLESProgramWriter::TargetLanguage =  "glsles";

        // Uniform comparer
        struct CompareUniformByName : std::binary_function<UniformParameterPtr, String, bool>
        {
            bool operator()( const UniformParameterPtr& uniform, const String& name ) const 
            {
                    return uniform->getName() == name;
            }
        };

        //-----------------------------------------------------------------------
        GLSLESProgramWriter::GLSLESProgramWriter()
        {
            mGLSLVersion = 100;
            initializeStringMaps();
        }

        //-----------------------------------------------------------------------
        GLSLESProgramWriter::~GLSLESProgramWriter()
        {

        }

        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::initializeStringMaps()
        {
            // Basic glsl es types
            mGpuConstTypeMap[GCT_FLOAT1] = "float";
            mGpuConstTypeMap[GCT_FLOAT2] = "vec2";
            mGpuConstTypeMap[GCT_FLOAT3] = "vec3";
            mGpuConstTypeMap[GCT_FLOAT4] = "vec4";
            mGpuConstTypeMap[GCT_SAMPLER2D] = "sampler2D";
            mGpuConstTypeMap[GCT_SAMPLERCUBE] = "samplerCube";
            mGpuConstTypeMap[GCT_MATRIX_2X2] = "mat2";
            mGpuConstTypeMap[GCT_MATRIX_3X3] = "mat3";
            mGpuConstTypeMap[GCT_MATRIX_4X4] = "mat4";
            mGpuConstTypeMap[GCT_INT1] = "int";
            mGpuConstTypeMap[GCT_INT2] = "int2";
            mGpuConstTypeMap[GCT_INT3] = "int3";
            mGpuConstTypeMap[GCT_INT4] = "int4";

            // Custom vertex attributes defined http://www.ogre3d.org/docs/manual/manual_21.html
            mContentToPerVertexAttributes[Parameter::SPC_POSITION_OBJECT_SPACE] = "vertex";
            mContentToPerVertexAttributes[Parameter::SPC_NORMAL_OBJECT_SPACE] = "normal";
            mContentToPerVertexAttributes[Parameter::SPC_TANGENT] = "tangent";
            mContentToPerVertexAttributes[Parameter::SPC_BINORMAL] = "binormal";

            mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE0] = "uv0";
            mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE1] = "uv1";
            mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE2] = "uv2";
            mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE3] = "uv3";
            mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE4] = "uv4";
            mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE5] = "uv5";
            mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE6] = "uv6";
            mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE7] = "uv7";
            mContentToPerVertexAttributes[Parameter::SPC_COLOR_DIFFUSE] = "colour";
            mContentToPerVertexAttributes[Parameter::SPC_COLOR_SPECULAR] = "secondary_colour";
        }

        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::writeSourceCode(std::ostream& os, Program* program)
        {
            GpuProgramType gpuType = program->getType();
            if(gpuType == GPT_GEOMETRY_PROGRAM)
            {
                OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, 
                    "Geometry Programs not supported in GLSL ES writer ", 
                    "GLSLESProgramWriter::writeSourceCode" );	
            }

            // Clear out old input params
            mFragInputParams.clear();

            const ShaderFunctionList& functionList = program->getFunctions();
            ShaderFunctionConstIterator itFunction;

            const UniformParameterList& parameterList = program->getParameters();
            UniformParameterConstIterator itUniformParam = parameterList.begin();
            
            // Write the current version (this forces the driver to fulfill the glsl es standard)
            os << "#version "<< mGLSLVersion << std::endl;

            // Default precision declaration is required in fragment and vertex shaders.
            os << "precision highp float;" << std::endl;
            os << "precision highp int;" << std::endl;
            os << "precision lowp sampler2D;" << std::endl;
            os << "precision lowp samplerCube;" << std::endl;

            // Generate source code header.
            writeProgramTitle(os, program);
            os<< std::endl;

            // Embed dependencies.
            writeProgramDependencies(os, program);
            os << std::endl;

            // Generate global variable code.
            writeUniformParametersTitle(os, program);
            os << std::endl;

            // Write the uniforms 
            for (itUniformParam = parameterList.begin();  itUniformParam != parameterList.end(); ++itUniformParam)
            {
                ParameterPtr pUniformParam = *itUniformParam;

                os << "uniform\t";
                os << mGpuConstTypeMap[pUniformParam->getType()];
                os << "\t";
                os << pUniformParam->getName();
                os << ";" << std::endl;
            }
            os << std::endl;			

            // Write program function(s).
            for (itFunction = functionList.begin(); itFunction != functionList.end(); ++itFunction)
            {
                Function* curFunction = *itFunction;

                writeFunctionTitle(os, curFunction);
                
                // Clear output mapping this map is used when we use glsl built in types
                mInputToGLStatesMap.clear();

                // Write inout params and fill mInputToGLStatesMap
                writeInputParameters(os, curFunction, gpuType);
                writeOutParameters(os, curFunction, gpuType);
                            
                // The function name must always main.
                os << "void main() {" << std::endl;

                // Write local parameters.
                const ShaderParameterList& localParams = curFunction->getLocalParameters();
                ShaderParameterConstIterator itParam = localParams.begin();
                ShaderParameterConstIterator itParamEnd = localParams.end();

                for (; itParam != itParamEnd; ++itParam)
                {
                    os << "\t";
                    writeLocalParameter(os, *itParam);			
                    os << ";" << std::endl;						
                }
                os << std::endl;			

                // Sort function atoms.
                curFunction->sortAtomInstances();
                
                const FunctionAtomInstanceList& atomInstances = curFunction->getAtomInstances();
                FunctionAtomInstanceConstIterator itAtom = atomInstances.begin();
                FunctionAtomInstanceConstIterator itAtomEnd = atomInstances.end();

                for (; itAtom != itAtomEnd; ++itAtom)
                {		
                    FunctionInvocation*  pFuncInvoc = (FunctionInvocation*)*itAtom;
                    FunctionInvocation::OperandVector::iterator itOperand = pFuncInvoc->getOperandList().begin();
                    FunctionInvocation::OperandVector::iterator itOperandEnd = pFuncInvoc->getOperandList().end();

                    // Local string stream
                    std::stringstream localOs;

                    // Write function name			
                    localOs << "\t" << pFuncInvoc->getFunctionName() << "(";

                    for (; itOperand != itOperandEnd; )
                    {
                        Operand op = *itOperand;
                        Operand::OpSemantic opSemantic = op.getSemantic();
                        String paramName = op.getParameter()->getName();
                        Parameter::Content content = op.getParameter()->getContent();

                        // Check if we write to an varying because the are only readable in fragment programs 
                        if (opSemantic == Operand::OPS_OUT || opSemantic == Operand::OPS_INOUT)
                        {	
                            // Is the written parameter a varying 
                            bool isVarying = false;

                            // Check if we write to an varying because the are only readable in fragment programs 
                            if (gpuType == GPT_FRAGMENT_PROGRAM)
                            {	
                                StringVector::iterator itFound = std::find(mFragInputParams.begin(), mFragInputParams.end(), paramName);	
                                if(itFound != mFragInputParams.end())
                                {						
                                    // Declare the copy variable
                                    String newVar = "local_" + paramName;
                                    String tempVar = paramName;
                                    isVarying = true;

                                    // We stored the original values in the mFragInputParams thats why we have to replace the first var with o
                                    // because all vertex output vars are prefixed with o in glsl the name has to match in the fragment program.
                                    tempVar.replace(tempVar.begin(), tempVar.begin() + 1, "o");

                                    // Declare the copy variable and assign the original
                                    os << "\t" << mGpuConstTypeMap[op.getParameter()->getType()] << " " << newVar << " = " << tempVar << ";\n" << std::endl;	

                                    // From now on we replace it automatic 
                                    mInputToGLStatesMap[paramName] = newVar;

                                    // Remove the param because now it is replaced automatic with the local variable
                                    // (which could be written).
                                    mFragInputParams.erase(itFound++);
                                }
                            }
                            
                            // If its not a varying param check if a uniform is written
                            if(!isVarying)
                            {
                                UniformParameterList::const_iterator itFound = std::find_if( parameterList.begin(), parameterList.end(), std::bind2nd( CompareUniformByName(), paramName ) );
                                if(itFound != parameterList.end())
                                {	
                                    // Declare the copy variable
                                    String newVar = "local_" + paramName;

                                    // now we check if we already declared a uniform redirector var
                                    if(mInputToGLStatesMap.find(newVar) == mInputToGLStatesMap.end())
                                    {
                                        // Declare the copy variable and assign the original
                                        os << "\t" << mGpuConstTypeMap[itFound->get()->getType()] << " " << newVar << " = " << paramName << ";\n" << std::endl;	

                                        // From now on we replace it automatic 
                                        mInputToGLStatesMap[paramName] = newVar;
                                    }
                                }
                            }

                        }

                        if(mInputToGLStatesMap.find(paramName) != mInputToGLStatesMap.end())
                        {
                            int mask = op.getMask(); // our swizzle mask

                            // Here we insert the renamed param name
                            localOs << mInputToGLStatesMap[paramName];

                            if(mask != Operand::OPM_ALL)
                            {
                                localOs << "." << Operand::getMaskAsString(mask);
                            }	
                            // Now that every texcoord is a vec4 (passed as vertex attributes) we
                            // have to swizzle them according the desired type.
                            else if(gpuType == GPT_VERTEX_PROGRAM &&
                                    content == Parameter::SPC_TEXTURE_COORDINATE0 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE1 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE2 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE3 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE4 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE5 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE6 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE7 )
                            {
                                // Now generate the swizzle mask according
                                // the type.
                                switch(op.getParameter()->getType())
                                {
                                case GCT_FLOAT1:
                                    localOs << ".x";
                                    break;
                                case GCT_FLOAT2:
                                    localOs << ".xy";
                                    break;
                                case GCT_FLOAT3:
                                    localOs << ".xyz";
                                    break;
                                case GCT_FLOAT4:
                                    localOs << ".xyzw";
                                    break;

                                default:
                                    break;
                                }
                            }						
                        }
                        else
                        {
                            localOs << op.toString();
                        }
                        
                        ++itOperand;

                        // Prepare for the next operand
                        if (itOperand != itOperandEnd)
                        {
                            localOs << ", ";
                        }
                    }

                    // Write function call closer.
                    localOs << ");" << std::endl;
                    localOs << std::endl;
                    os << localOs.str();
                }
                os << "}" << std::endl;
            }
            os << std::endl;
        }

        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::writeInputParameters(std::ostream& os, Function* function, GpuProgramType gpuType)
        {
            const ShaderParameterList& inParams = function->getInputParameters();

            ShaderParameterConstIterator itParam = inParams.begin();
            ShaderParameterConstIterator itParamEnd = inParams.end();

            for ( ; itParam != itParamEnd; ++itParam)
            {		
                ParameterPtr pParam = *itParam;
                Parameter::Content paramContent = pParam->getContent();
                String paramName = pParam->getName();

                if (gpuType == GPT_FRAGMENT_PROGRAM)
                {					
                    // push fragment inputs they all could be written (in glsl you can not write
                    // input params in the fragment program)
                    mFragInputParams.push_back(paramName);

                    // In the vertex and fragment program the variable names must match.
                    // Unfortunately now the input params are prefixed with an 'i' and output params with 'o'.
                    // Thats why we are using a map for name mapping (we rename the params which are used in function atoms).
                    paramName.replace(paramName.begin(), paramName.begin() + 1, "o");	
                    mInputToGLStatesMap[pParam->getName()] = paramName;

                    os << "varying\t";
                    os << mGpuConstTypeMap[pParam->getType()];
                    os << "\t";	
                    os << paramName;
                    os << ";" << std::endl;	
                }
                else if (gpuType == GPT_VERTEX_PROGRAM && 
                         mContentToPerVertexAttributes.find(paramContent) != mContentToPerVertexAttributes.end())
                {
                    // Due the fact that glsl does not have register like cg we have to rename the params
                    // according their content.
                    mInputToGLStatesMap[paramName] = mContentToPerVertexAttributes[paramContent];
                    os << "attribute\t"; 

                    // All uv texcoords passed by OGRE are vec4
                    if (paramContent == Parameter::SPC_TEXTURE_COORDINATE0 ||
                        paramContent == Parameter::SPC_TEXTURE_COORDINATE1 ||
                        paramContent == Parameter::SPC_TEXTURE_COORDINATE2 ||
                        paramContent == Parameter::SPC_TEXTURE_COORDINATE3 ||
                        paramContent == Parameter::SPC_TEXTURE_COORDINATE4 ||
                        paramContent == Parameter::SPC_TEXTURE_COORDINATE5 ||
                        paramContent == Parameter::SPC_TEXTURE_COORDINATE6 ||
                        paramContent == Parameter::SPC_TEXTURE_COORDINATE7)
                    {
                        os << "vec4";
                    }
                    else
                    {
                        os << mGpuConstTypeMap[pParam->getType()];
                    }
                    os << "\t";	
                    os << mContentToPerVertexAttributes[paramContent];
                    os << ";" << std::endl;	
                }
                else
                {
                    os << "uniform \t ";
                    os << mGpuConstTypeMap[pParam->getType()];
                    os << "\t";	
                    os << paramName;
                    os << ";" << std::endl;	
                }							
            }
        }

        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::writeOutParameters(std::ostream& os, Function* function, GpuProgramType gpuType)
        {
            const ShaderParameterList& outParams = function->getOutputParameters();

            ShaderParameterConstIterator itParam = outParams.begin();
            ShaderParameterConstIterator itParamEnd = outParams.end();

            for ( ; itParam != itParamEnd; ++itParam)
            {
                ParameterPtr pParam = *itParam;

                if(gpuType == GPT_VERTEX_PROGRAM)
                {
                    // GLSL vertex program has to write always gl_Position
                    if(pParam->getContent() == Parameter::SPC_POSITION_PROJECTIVE_SPACE)
                    {
                        mInputToGLStatesMap[pParam->getName()] = "gl_Position";
                    }
                    else
                    {
                        os << "varying\t";
                        os << mGpuConstTypeMap[pParam->getType()];
                        os << "\t";
                        os << pParam->getName();
                        os << ";" << std::endl;	
                    }
                }
                else if(gpuType == GPT_FRAGMENT_PROGRAM &&
                        pParam->getSemantic() == Parameter::SPS_COLOR)
                {					
                    // GLSL ES fragment program has to always write gl_FragColor
                    mInputToGLStatesMap[pParam->getName()] = "gl_FragColor";						
                }
            }
        }

        //-----------------------------------------------------------------------
        // Here's the gist of how and what we're doing here.
        // First, identify which fixed function libraries we need.  We already have a list of functions that we need to find.
        // Then for each library file we perform the following steps:
        // 1. Go through the source line by line to find function signatures.
        // 2. Once we have found one, compare it to the list of functions that we are searching for
        // 3. If a match is found then continue reading through the file until we reach the end of the function.
        // 4. When we reach the last closing brace, write the function and its signature to the output stream
        // 5. Go back to step 1 until we have found all the functions
        //
        void GLSLESProgramWriter::writeProgramDependencies(std::ostream& os, Program* program)
        {
            os << "//-----------------------------------------------------------------------------" << std::endl;
            os << "//                         PROGRAM DEPENDENCIES"                                 << std::endl;
            os << "//-----------------------------------------------------------------------------" << std::endl;

            StringVector forwardDecl; // Holds all generated function declarations 
            const ShaderFunctionList& functionList = program->getFunctions();
            ShaderFunctionConstIterator itFunction;

            // Iterate over all functions in the current program (in our case this is always the main() function)
            for ( itFunction = functionList.begin(); itFunction != functionList.end(); ++itFunction)
            {
                Function* curFunction = *itFunction;
                const FunctionAtomInstanceList& atomInstances = curFunction->getAtomInstances();
                FunctionAtomInstanceConstIterator itAtom = atomInstances.begin();
                FunctionAtomInstanceConstIterator itAtomEnd = atomInstances.end();

                // Now iterate over all function atoms
                for ( ; itAtom != itAtomEnd; ++itAtom)
                {	
                    // Skip non function invocation atoms.
                    if ((*itAtom)->getFunctionAtomType() != FunctionInvocation::Type)
                        continue;

                    FunctionInvocation* pFuncInvoc = static_cast<FunctionInvocation*>(*itAtom);			
                    FunctionInvocation::OperandVector::iterator itOperator = pFuncInvoc->getOperandList().begin();
                    FunctionInvocation::OperandVector::iterator itOperatorEnd = pFuncInvoc->getOperandList().end();

                    // Start with function declaration 
                    String funcDecl = pFuncInvoc->getReturnType() + " " + pFuncInvoc->getFunctionName() + "(";

                    // Now iterate overall operands
                    for (; itOperator != itOperatorEnd; )
                    {
                        ParameterPtr pParam = (*itOperator).getParameter();				
                        Operand::OpSemantic opSemantic = (*itOperator).getSemantic();
                        int opMask = (*itOperator).getMask();
                        GpuConstantType gpuType = GCT_UNKNOWN;

                        // Write the semantic in, out, inout
                        switch(opSemantic)
                        {
                        case Operand::OPS_IN:
                            funcDecl += "in ";
                            break;

                        case Operand::OPS_OUT:
                            funcDecl += "out ";
                            break;

                        case Operand::OPS_INOUT:
                            funcDecl += "inout ";
                            break;

                        default:
                            break;
                        }				
                        
                        // Swizzle masks are only defined for types like vec2, vec3, vec4.
                        if (opMask == Operand::OPM_ALL)
                        {
                            gpuType = pParam->getType();
                        }
                        else
                        {
                            // Now we have to convert the mask to operator
                            gpuType = Operand::getGpuConstantType(opMask);
                        }

                        // We need a valid type otherwise glsl compilation will not work
                        if (gpuType == GCT_UNKNOWN)
                        {
                            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                                "Can not convert Operand::OpMask to GpuConstantType", 
                                "GLSLESProgramWriter::writeProgramDependencies" );	
                        }

                        // Write the operand type.
                        funcDecl += mGpuConstTypeMap[gpuType];

                        ++itOperator;

                        // Prepare for the next operand
                        if (itOperator != itOperatorEnd)
                        {
                            funcDecl += ", ";
                        }
                    }
                    // Write function call closer.
                    funcDecl += ");\n";

                    // Push the generated declaration into the vector
                    // duplicate declarations will be removed later.
                    forwardDecl.push_back(funcDecl);
                }
            }

            // Now remove duplicate declaration, first we have to sort the vector.
            std::sort(forwardDecl.begin(), forwardDecl.end());
            StringVector::iterator endIt = std::unique(forwardDecl.begin(), forwardDecl.end()); 
            
            // Parse the source shader and write out only the needed functions
            // Iterate through each library
            for(unsigned int i = 0; i < program->getDependencyCount(); ++i)
            {
                const String& curDependency = program->getDependency(i);

                StringMap functionCache;
                functionCache.clear();

                // GLSL ES does not have the transpose function.  If we are using the
                // FFP Texturing library there is a good chance that we'll need it.
                if(curDependency == "FFPLib_Texturing")
                {
                    os << "void transpose(in mat4 mView, out mat4 mOut)\n";
                    os << "{\n";
                    os << "\tmOut[0][0] = mView[0][0];\n";
                    os << "\tmOut[1][0] = mView[0][1];\n";
                    os << "\tmOut[2][0] = mView[0][2];\n";
                    os << "\tmOut[3][0] = mView[0][3];\n";
                    os << "\n";
                    os << "\tmOut[0][1] = mView[1][0];\n";
                    os << "\tmOut[1][1] = mView[1][1];\n";
                    os << "\tmOut[2][1] = mView[1][2];\n";
                    os << "\tmOut[3][1] = mView[1][3];\n";
                    os << "\n";
                    os << "\tmOut[0][2] = mView[2][0];\n";
                    os << "\tmOut[1][2] = mView[2][1];\n";
                    os << "\tmOut[2][2] = mView[2][2];\n";
                    os << "\tmOut[3][2] = mView[2][3];\n";
                    os << "\n";
                    os << "\tmOut[0][3] = mView[3][0];\n";
                    os << "\tmOut[1][3] = mView[3][1];\n";
                    os << "\tmOut[2][3] = mView[3][2];\n";
                    os << "\tmOut[3][3] = mView[3][3];\n";
                    os << "}\n\n";
                }

                DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(curDependency + 
                                                                                         "." +
                                                                                         getTargetLanguage());
                String line;
                while(!stream->eof())
                {
                    // Grab a line
                    line = stream->getLine();

                    // Ignore empty lines and comments
                    if(line.length() > 0)
                    {
                        // Strip whitespace
                        StringUtil::trim(line);

                        // If we find a multiline comment, run through till we get to the end 
                        if(line.at(0) == '/' && line.at(1) == '*')
                        {
                            bool endFound = false;
                            
                            while(!endFound)
                            {
                                // Get the next line
                                line = stream->getLine();

                                // Skip empties
                                if(line.length() > 0)
                                {
                                    // Look for the ending sequence.
                                    String::size_type comment_pos = line.find("*/", 0);
                                    if(comment_pos != String::npos)
                                    {
                                        endFound = true;
                                    }
                                }
                            }
                        }
                        else if(line.length() > 1 && line.at(0) != '/' && line.at(1) != '/')
                        {
                            String function;

                            // Break up the line.
                            StringVector tokens = StringUtil::tokenise(line, " (\n\r");

                            // Always copy #defines
                            if(tokens[0] == "#define")
                            {
                                // Add the line in
                                os << line;
                                
                                // Also add a newline because it got stripped out a few lines up
                                os << "\n";
                                
                                // Move on to the next line in the shader
                                continue;
                            }
                            
                            // Try to identify a function definition
                            // First, look for a return type
                            if(isBasicType(tokens[0]))
                            {
                                String functionName;
                                
                                // Return type
                                function += tokens[0];
                                function += " ";
                                
                                // Function name
                                function += tokens[1];
                                function += "(";
                                functionName = tokens[1];
                                
                                bool foundEndOfSignature = false;
                                // Now look for all the parameters, they may span multiple lines
                                while(!foundEndOfSignature)
                                {
                                    // First we want to get everything right of the paren
                                    StringVector paramTokens;
                                    String::size_type lparen_pos = line.find('(', 0);
                                    if(lparen_pos != String::npos)
                                    {
                                        StringVector tokens = StringUtil::split(line, "(");
                                        paramTokens = StringUtil::split(tokens[1], ",");
                                    }
                                    else
                                    {
                                        paramTokens = StringUtil::split(line, ",");
                                    }

                                    StringVector::iterator itParam;
                                    for(itParam = paramTokens.begin(); itParam != paramTokens.end(); ++itParam)
                                    {
                                        function += *itParam;

                                        String::size_type rparen_pos = itParam->find(')', 0);
                                        if(rparen_pos == String::npos)
                                            function += ", ";
                                    }

                                    String::size_type space_pos = line.find(')', 0);
                                    if(space_pos != String::npos)
                                    {
                                        foundEndOfSignature = true;
                                        function += "\n";
                                    }
                                    line = stream->getLine();
                                }
                                
                                // Check to see if this is a function that we actually need.
                                // This will require a little alteration of the function signature to remove parameter names
                                
                                String funcDecl;
                                StringVector tokens = StringUtil::split(function, "(");
                                funcDecl = tokens[0];
                                funcDecl += "(";

                                StringVector paramTokens = StringUtil::split(tokens[1], ",");
                                StringVector::iterator itParam;
                                for(itParam = paramTokens.begin(); itParam != paramTokens.end(); ++itParam)
                                {
                                    StringVector paramPieces = StringUtil::split(*itParam, " ");
                                    // If there are no spaces than this might be an empty line, skip over it
                                    if(paramPieces.size() < 2)
                                        continue;

                                    funcDecl += paramPieces[0];
                                    funcDecl += " ";
                                    funcDecl += paramPieces[1];

                                    String::size_type rparen_pos = itParam->find(')', 0);
                                    if(rparen_pos == String::npos)
                                        funcDecl += ", ";
                                    else
                                        funcDecl += ");\n";
                                }

                                bool functionIsNeeded = false;
                                for (StringVector::iterator it = forwardDecl.begin(); it != endIt; it++)
                                {
                                    if(*it != funcDecl)
                                        continue;

                                    functionIsNeeded = true;
                                    break;
                                }
                                
                                // Ok, now if we have found the signature, iterate through the file until we find the end
                                // of the function.
                                bool foundEndOfBody = false;
                                size_t braceCount = 0;
                                while(!foundEndOfBody)
                                {
                                    function += "\t" + line + "\n";

                                    String::size_type brace_pos = line.find('{', 0);
                                    if(brace_pos != String::npos)
                                    {
                                        braceCount++;
                                    }

                                    // Look for non-builtin functions
                                    // Do so by assuming that these functions do not have several variations.
                                    // Also, because GLSL is C based, functions must be defined before they are used
                                    // so we can make the assumption that we already have this function cached.
                                    //
                                    // If we find a function, look it up in the map and write it out
                                    StringUtil::trim(line);
                                    StringVector tokens = StringUtil::split(line, "(");
                                    for (StringVector::iterator it = tokens.begin(); it != tokens.end(); it++)
                                    {
                                        StringVector moreTokens = StringUtil::split(*it, " ");
                                            
                                        if(!functionCache[moreTokens.back()].empty())
                                        {
                                            String func = functionCache[moreTokens.back()];
                                            os << functionCache[moreTokens.back()];
                                            
                                            // Because we don't want to write it out twice, remove from the cache
                                            functionCache.erase(moreTokens.back());
                                        }
                                    }
                                    
                                    brace_pos = line.find('}', 0);
                                    if(brace_pos != String::npos)
                                        braceCount--;

                                    if(braceCount == 0)
                                        foundEndOfBody = true;
                                    
                                    // Save function here to our cache
                                    functionCache[functionName] = function;

                                    line = stream->getLine();
                                }

                                if(functionIsNeeded)
                                    os << function;
                            }
                        }
                    }
                }
            }
        }

        bool GLSLESProgramWriter::isBasicType(String &type)
        {
            if(type == "void" ||
               type == "float" ||
               type == "vec2" ||
               type == "vec3" ||
               type == "vec4" ||
               type == "sampler2D" ||
               type == "samplerCube" ||
               type == "mat2" ||
               type == "mat3" ||
               type == "mat4" ||
               type == "int" ||
               type == "int2" ||
               type == "int3" ||
               type == "int4")
                return true;
            else
                return false;

        }

        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::writeLocalParameter(std::ostream& os, ParameterPtr parameter)
        {
            os << mGpuConstTypeMap[parameter->getType()];
            os << "\t";	
            os << parameter->getName();		
        }

    }
}
