/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
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
#include "OgreShaderProgram.h"

#include "OgreShaderFunctionAtom.h"
#include "OgreRoot.h"
#include "OgreString.h"
#include "OgreLogManager.h"

namespace Ogre {
    namespace RTShader {

        String GLSLESProgramWriter::TargetLanguage =  "glsles";

        //-----------------------------------------------------------------------
        GLSLESProgramWriter::GLSLESProgramWriter()
        {
            mGLSLVersion = Root::getSingleton().getRenderSystem()->getNativeShadingLanguageVersion();
            initializeStringMaps();
            mFunctionCacheMap.clear();
        }

        //-----------------------------------------------------------------------
        GLSLESProgramWriter::~GLSLESProgramWriter()
        {

        }

        FunctionInvocation * GLSLESProgramWriter::createInvocationFromString(const String & input)
        {
            String functionName, returnType;
            FunctionInvocation *invoc = NULL;

            // Get the function name and return type
            StringVector leftTokens = StringUtil::split(input, "(");
            StringVector leftTokens2 = StringUtil::split(leftTokens[0], " ");
            StringUtil::trim(leftTokens2[0]);
            StringUtil::trim(leftTokens2[1]);
            returnType      = leftTokens2[0];
            functionName    = leftTokens2[1];

            invoc = OGRE_NEW FunctionInvocation(functionName, 0, 0, returnType);

            // Split out the parameters
            StringVector parameters;
            String::size_type lparen_pos = input.find('(', 0);
            if(lparen_pos != String::npos)
            {
                StringVector tokens = StringUtil::split(input, "(");
                parameters = StringUtil::split(tokens[1], ",");
            }
            else
            {
                parameters = StringUtil::split(input, ",");
            }

            StringVector::const_iterator itParam;
            int i = 0;
            for(itParam = parameters.begin(); itParam != parameters.end(); ++itParam, i++)
            {
                StringUtil::replaceAll(*itParam, ")", "");
                StringUtil::replaceAll(*itParam, ",", "");
                StringVector paramTokens = StringUtil::split(*itParam, " ");
                // There should be three parts for each token
                // 1. The operand type(in, out, inout)
                // 2. The type
                // 3. The name
                if(paramTokens.size() == 3)
                {
                    StringUtil::trim(paramTokens[0]);
                    StringUtil::trim(paramTokens[1]);
                    StringUtil::trim(paramTokens[2]);

                    Operand::OpSemantic semantic = Operand::OPS_IN;
                    GpuConstantType gpuType = GCT_UNKNOWN;

                    if(paramTokens[0] == "in")
                    {
                        semantic = Operand::OPS_IN;
                    }
                    else if(paramTokens[0] == "out")
                    {
                        semantic = Operand::OPS_OUT;
                    }
                    else if(paramTokens[0] == "inout")
                    {
                        semantic = Operand::OPS_INOUT;
                    }

                    // Find the internal type based on the string that we're given.
                    GpuConstTypeToStringMapIterator typeMapIterator;
                    for(typeMapIterator = mGpuConstTypeMap.begin(); typeMapIterator != mGpuConstTypeMap.end(); ++typeMapIterator)
                    {
                        if((*typeMapIterator).second == paramTokens[1])
                        {
                            gpuType = (*typeMapIterator).first;
                            break;
                        }
                    }

                    // We need a valid type otherwise glsl compilation will not work
                    if (gpuType == GCT_UNKNOWN)
                    {
                        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                            "Can not convert Operand::OpMask to GpuConstantType", 
                            "GLSLESProgramWriter::createInvocationFromString" );    
                    }

                    if(gpuType == GCT_SAMPLER1D)
                        gpuType = GCT_SAMPLER2D;

                    ParameterPtr p = ParameterPtr(OGRE_NEW Parameter(gpuType, paramTokens[2],
                                                                     Parameter::SPS_UNKNOWN, i,
                                                                     Parameter::SPC_UNKNOWN));

                    invoc->pushOperand(p, semantic);
                }
            }

            return invoc;
        }
        
        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::discoverFunctionDependencies(const FunctionInvocation &invoc, FunctionVector &depVector)
        {
            // Uses recursion to find any functions that the supplied function invocation depends on
            FunctionMap::const_iterator itCache = mFunctionCacheMap.begin();
            String body = BLANKSTRING;

            // Find the function in the cache and retrieve the body
            for (; itCache != mFunctionCacheMap.end(); ++itCache)
            {
                if(!(invoc == (*itCache).first))
                    continue;

                body = (*itCache).second;
                break;
            }

            if(body != BLANKSTRING)
            {
                // Trim whitespace
                StringUtil::trim(body);
                StringVector tokens = StringUtil::split(body, "(");

                for (StringVector::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
                {
                    StringVector moreTokens = StringUtil::split(*it, " ");

                    if (!moreTokens.empty())
                    {
                    FunctionMap::const_iterator itFuncCache = mFunctionCacheMap.begin();

                        for (; itFuncCache != mFunctionCacheMap.end(); ++itFuncCache)
                        {

                            FunctionInvocation fi = itFuncCache->first;
                        
                            if(fi.getFunctionName() == moreTokens.back())
                            {
                                // Add the function declaration
                                depVector.push_back(FunctionInvocation((*itFuncCache).first));

                                discoverFunctionDependencies(itFuncCache->first, depVector);
                            }
                        }
                    }
                }
                
            }
            else
            {
                LogManager::getSingleton().logMessage("ERROR: Cached function not found " + invoc.getFunctionName());
            }
        }
        
        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::initializeStringMaps()
        {
            // Basic glsl es types
            mGpuConstTypeMap[GCT_FLOAT1] = "float";
            mGpuConstTypeMap[GCT_FLOAT2] = "vec2";
            mGpuConstTypeMap[GCT_FLOAT3] = "vec3";
            mGpuConstTypeMap[GCT_FLOAT4] = "vec4";
            mGpuConstTypeMap[GCT_SAMPLER1D] = "sampler2D";
            mGpuConstTypeMap[GCT_SAMPLER2D] = "sampler2D";
            mGpuConstTypeMap[GCT_SAMPLER3D] = "sampler3D";
            mGpuConstTypeMap[GCT_SAMPLER2DARRAY] = "sampler2DArray";
            mGpuConstTypeMap[GCT_SAMPLERCUBE] = "samplerCube";
            mGpuConstTypeMap[GCT_SAMPLER1DSHADOW] = "sampler1DShadow";
            mGpuConstTypeMap[GCT_SAMPLER2DSHADOW] = "sampler2DShadow";
            mGpuConstTypeMap[GCT_MATRIX_2X2] = "mat2";
            mGpuConstTypeMap[GCT_MATRIX_2X3] = "mat2x3";
            mGpuConstTypeMap[GCT_MATRIX_2X4] = "mat2x4";
            mGpuConstTypeMap[GCT_MATRIX_3X2] = "mat3x2";
            mGpuConstTypeMap[GCT_MATRIX_3X3] = "mat3";
            mGpuConstTypeMap[GCT_MATRIX_3X4] = "mat3x4";
            mGpuConstTypeMap[GCT_MATRIX_4X2] = "mat4x2";
            mGpuConstTypeMap[GCT_MATRIX_4X3] = "mat4x3";
            mGpuConstTypeMap[GCT_MATRIX_4X4] = "mat4";
            mGpuConstTypeMap[GCT_INT1] = "int";
            mGpuConstTypeMap[GCT_INT2] = "int2";
            mGpuConstTypeMap[GCT_INT3] = "int3";
            mGpuConstTypeMap[GCT_INT4] = "int4";
            mGpuConstTypeMap[GCT_UINT1] = "uint";
            mGpuConstTypeMap[GCT_UINT2] = "uint2";
            mGpuConstTypeMap[GCT_UINT3] = "uint3";
            mGpuConstTypeMap[GCT_UINT4] = "uint4";

            // Custom vertex attributes defined http://www.ogre3d.org/docs/manual/manual_21.html
            mContentToPerVertexAttributes[Parameter::SPC_POSITION_OBJECT_SPACE] = "vertex";
            mContentToPerVertexAttributes[Parameter::SPC_NORMAL_OBJECT_SPACE] = "normal";
            mContentToPerVertexAttributes[Parameter::SPC_TANGENT_OBJECT_SPACE] = "tangent";
            mContentToPerVertexAttributes[Parameter::SPC_BINORMAL_OBJECT_SPACE] = "binormal";

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
        void GLSLESProgramWriter::writeSourceCode(
            std::ostream& os, 
            Program* program)
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
            os << "#version "<< mGLSLVersion;

            // Starting with ES 3.0 the version must contain the string "es" after the version number with a space separating them
            if(mGLSLVersion > 100)
                os << " es";

            os << std::endl;

            // Default precision declaration is required in fragment and vertex shaders.
            os << "precision highp float;" << std::endl;
            os << "precision highp int;" << std::endl;

            // Redefine texture functions to maintain reusability
            if(mGLSLVersion > 100)
            {
                os << "#define texture2D texture" << std::endl;
                os << "#define texture3D texture" << std::endl;
                os << "#define textureCube texture" << std::endl;
            }

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
                if (pUniformParam->isArray() == true)
                {
                    os << "[" << pUniformParam->getSize() << "]";   
                }
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

                if (gpuType == GPT_FRAGMENT_PROGRAM)
                {
                    os << "\tvec4 outputColor;" << std::endl;
                }
                else if (gpuType == GPT_VERTEX_PROGRAM)
                {
                    os << "\tvec4 outputPosition;" << std::endl;
                }

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
                    FunctionInvocation::OperandVector::const_iterator itOperandEnd = pFuncInvoc->getOperandList().end();

                    // Local string stream
                    StringStream localOs;

                    // Write function name          
                    localOs << "\t" << pFuncInvoc->getFunctionName() << "(";

                    int curIndLevel = 0;
                    for (; itOperand != itOperandEnd; )
                    {
                        Operand op = *itOperand;
                        Operand::OpSemantic opSemantic = op.getSemantic();
                        String paramName = op.getParameter()->getName();
                        Parameter::Content content = op.getParameter()->getContent();

                        // Check if we write to a varying because the are only readable in fragment programs 
                        if (opSemantic == Operand::OPS_OUT || opSemantic == Operand::OPS_INOUT)
                        {   
                            // Is the written parameter a varying 
                            bool isVarying = false;

                            // Check if we write to a varying because the are only readable in fragment programs 
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

                        String newParam;
                        if(mInputToGLStatesMap.find(paramName) != mInputToGLStatesMap.end())
                        {
                            int mask = op.getMask(); // our swizzle mask
                            
                            // Here we insert the renamed param name
                            newParam = mInputToGLStatesMap[paramName];

                            if(mask != Operand::OPM_ALL)
                            {
                                newParam += "." + Operand::getMaskAsString(mask);
                            }   
                            // Now that every texcoord is a vec4 (passed as vertex attributes) we
                            // have to swizzle them according the desired type.
                            else if(gpuType == GPT_VERTEX_PROGRAM &&
                                    (content == Parameter::SPC_TEXTURE_COORDINATE0 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE1 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE2 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE3 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE4 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE5 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE6 ||
                                    content == Parameter::SPC_TEXTURE_COORDINATE7) )
                            {
                                // Now generate the swizzle mask according
                                // the type.
                                switch(op.getParameter()->getType())
                                {
                                case GCT_FLOAT1:
                                    newParam += ".x";
                                    break;
                                case GCT_FLOAT2:
                                    newParam += ".xy";
                                    break;
                                case GCT_FLOAT3:
                                    newParam += ".xyz";
                                    break;
                                case GCT_FLOAT4:
                                    newParam += ".xyzw";
                                    break;

                                default:
                                    break;
                                }
                            }                       
                        }
                        else
                        {
                            newParam = op.toString();
                        }
                        
                        ++itOperand;

                        // Prepare for the next operand
                        localOs << newParam;

                        // Prepare for the next operand
                        ushort opIndLevel = 0;
                        if (itOperand != itOperandEnd)
                        {
                            opIndLevel = itOperand->getIndirectionLevel();
                        }

                        if (curIndLevel != 0)
                        {
                            localOs << ")";
                        }

                        if (curIndLevel < opIndLevel)
                        {
                            while (curIndLevel < opIndLevel)
                            {
                                ++curIndLevel;
                                localOs << "[";
                            }
                        }
                        else //if (curIndLevel >= opIndLevel)
                        {
                            while (curIndLevel > opIndLevel)
                            {
                                --curIndLevel;
                                localOs << "]";
                            }
                            if (opIndLevel != 0)
                            {
                                localOs << "][";
                            }
                            else if (itOperand != itOperandEnd)
                            {
                                localOs << ", ";
                            }
                        }
                        if (curIndLevel != 0)
                        {
                            localOs << "int(";
                        }
                    }

                    // Write function call closer.
                    localOs << ");" << std::endl;
                    localOs << std::endl;
                    os << localOs.str();
                }
                
                if (gpuType == GPT_FRAGMENT_PROGRAM)
                {
                    // GLSL fragment program has to write always gl_FragColor (but this is also deprecated after version 130)
                    // Always add gl_FragColor as an output.  The name is for compatibility.
                    if(mGLSLVersion > 100)
                    {
                        os << "\tfragColour = outputColor;" << std::endl;
                    }
                    else
                    {
                        os << "\tgl_FragColor = outputColor;" << std::endl;
                    }
                }
                else if (gpuType == GPT_VERTEX_PROGRAM)
                {
                    os << "\tgl_Position = outputPosition;" << std::endl;
                }
                
                os << "}" << std::endl;
            }
            os << std::endl;
        }

        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::writeInputParameters(
            std::ostream& os, 
            Function* function, 
            GpuProgramType gpuType)
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

                    if(mGLSLVersion > 100)
                        os << "in\t";
                    else
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
                    if(mGLSLVersion > 100)
                        os << "in\t";
                    else
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
        void GLSLESProgramWriter::writeOutParameters(
            std::ostream& os,
            Function* function, 
            GpuProgramType gpuType)
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
                        mInputToGLStatesMap[pParam->getName()] = "outputPosition";
                    }
                    else
                    {
                        if(mGLSLVersion > 100)
                            os << "out\t";
                        else
                            os << "varying\t";
                        os << mGpuConstTypeMap[pParam->getType()];
                        os << "\t";
                        os << pParam->getName();
                        if (pParam->isArray() == true)
                        {
                            os << "[" << pParam->getSize() << "]";  
                        }
                        os << ";" << std::endl; 
                    }
                }
                else if(gpuType == GPT_FRAGMENT_PROGRAM &&
                        pParam->getSemantic() == Parameter::SPS_COLOR)
                {
                    if(mGLSLVersion > 100)
                        os << "out vec4 fragColour;" << std::endl;

                    // GLSL ES fragment program has to always write gl_FragColor
                    mInputToGLStatesMap[pParam->getName()] = "outputColor";
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
        void GLSLESProgramWriter::writeProgramDependencies(
            std::ostream& os, 
            Program* program)
        {
            for(unsigned int i = 0; i < program->getDependencyCount(); ++i)
            {
                const String& curDependency = program->getDependency(i);
                cacheDependencyFunctions(curDependency);
            }


            os << "//-----------------------------------------------------------------------------" << std::endl;
            os << "//                         PROGRAM DEPENDENCIES"                                 << std::endl;
            os << "//-----------------------------------------------------------------------------" << std::endl;

            FunctionVector forwardDecl; // Holds all function declarations
            const ShaderFunctionList& functionList = program->getFunctions();

            Function* curFunction = *(functionList.begin());
            FunctionAtomInstanceList& atomInstances = curFunction->getAtomInstances();
            FunctionAtomInstanceConstIterator itAtom = atomInstances.begin();
            FunctionAtomInstanceConstIterator itAtomEnd = atomInstances.end();
            // Now iterate over all function atoms
            for ( ; itAtom != itAtomEnd; ++itAtom)
            {   
                // Skip non function invocation atoms.
                if ((*itAtom)->getFunctionAtomType() != FunctionInvocation::Type)
                    continue;

                FunctionInvocation pFuncInvoc = *(static_cast<FunctionInvocation *>(*itAtom));
                forwardDecl.push_back(pFuncInvoc);
                
                // Now look into that function for other non-builtin functions and add them to the declaration list
                // Look for non-builtin functions
                // Do so by assuming that these functions do not have several variations.
                // Also, because GLSL is C based, functions must be defined before they are used
                // so we can make the assumption that we already have this function cached.
                //
                // If we find a function, look it up in the map and write it out
                discoverFunctionDependencies(pFuncInvoc, forwardDecl);
            }

            // Now remove duplicate declarations, first we have to sort the vector.
            std::sort(forwardDecl.begin(), forwardDecl.end(), FunctionInvocation::FunctionInvocationLessThan());
            forwardDecl.erase(std::unique(forwardDecl.begin(), forwardDecl.end(), FunctionInvocation::FunctionInvocationCompare()), forwardDecl.end());

            for(unsigned int i = 0; i < program->getDependencyCount(); ++i)
            {
                const String& curDependency = program->getDependency(i);

                // Write out #defines
                StringMap::const_iterator itDefines = mDefinesMap.begin();
                StringMap::const_iterator itDefinesEnd = mDefinesMap.end();
                for (; itDefines != itDefinesEnd; ++itDefines)
                {
                    if((*itDefines).second == curDependency)
                    {
                        os << (*itDefines).first;
                        os << "\n";
                    }
                }
            }

            // Parse the source shader and write out only the needed functions
            for (FunctionVector::const_iterator it = forwardDecl.begin(); it != forwardDecl.end(); ++it)
            {
                FunctionMap::const_iterator itCache = mFunctionCacheMap.begin();
                FunctionInvocation invoc = FunctionInvocation("", 0, 0);
                String body = BLANKSTRING;

                // Find the function in the cache
                for (; itCache != mFunctionCacheMap.end(); ++itCache)
                {
                    if(!((*it) == (*itCache).first))
                        continue;

                    invoc = (*itCache).first;
                    body = (*itCache).second;
                    break;
                }

                if(invoc.getFunctionName().length())
                {
                    // Write out the function from the cached FunctionInvocation;
                    os << invoc.getReturnType();
                    os << " ";
                    os << invoc.getFunctionName();
                    os << "(";

                    FunctionInvocation::OperandVector::iterator itOperand    = invoc.getOperandList().begin();
                    FunctionInvocation::OperandVector::iterator itOperandEnd = invoc.getOperandList().end();
                    for (; itOperand != itOperandEnd;)
                    {
                        Operand op = *itOperand;
                        Operand::OpSemantic opSemantic = op.getSemantic();
                        String paramName = op.getParameter()->getName();
                        int opMask = (*itOperand).getMask();
                        GpuConstantType gpuType = GCT_UNKNOWN;

                        switch(opSemantic)
                        {
                        case Operand::OPS_IN:
                            os << "in ";
                            break;

                        case Operand::OPS_OUT:
                            os << "out ";
                            break;

                        case Operand::OPS_INOUT:
                            os << "inout ";
                            break;

                        default:
                            break;
                        }

                        // Swizzle masks are only defined for types like vec2, vec3, vec4.
                        if (opMask == Operand::OPM_ALL)
                        {
                            gpuType = op.getParameter()->getType();
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

                        os << mGpuConstTypeMap[gpuType] << " " << paramName;

                        ++itOperand;

                        // Prepare for the next operand
                        if (itOperand != itOperandEnd)
                        {
                            os << ", ";
                        }
                    }
                    os << std::endl << "{" << std::endl << body << std::endl << "}" << std::endl;
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
            if (parameter->isArray() == true)
            {
                os << "[" << parameter->getSize() << "]";   
            }
        }
        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::cacheDependencyFunctions(const String & libName)
        {
            if(mCachedFunctionLibraries.find(libName) != mCachedFunctionLibraries.end())
            {
                // this mean that the lib is in the cache
                return;
            }

            mCachedFunctionLibraries[libName] = "";

            String libFileName =  libName + ".glsles";

            DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(libFileName);

            StringMap functionCache;
            functionCache.clear();

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
                        // Break up the line.
                        StringVector tokens = StringUtil::tokenise(line, " (\n\r");

                        // Cache #defines
                        if(tokens[0] == "#define")
                        {
                            // Add the line in
                            mDefinesMap[line] = libName;

                            // Move on to the next line in the shader
                            continue;
                        }

                        // Try to identify a function definition
                        // First, look for a return type
                        if(isBasicType(tokens[0]) && ((tokens.size() < 3) || (tokens[2] != "=")) )
                        {
                            String functionSig = "";
                            String functionBody = "";
                            FunctionInvocation *functionInvoc = NULL;

                            // Return type
                            functionSig = tokens[0];
                            functionSig += " ";

                            // Function name
                            functionSig += tokens[1];
                            functionSig += "(";

                            bool foundEndOfSignature = false;
                            // Now look for all the parameters, they may span multiple lines
                            while(!foundEndOfSignature)
                            {
                                // Trim whitespace from both sides of the line
                                StringUtil::trim(line);

                                // First we want to get everything right of the paren
                                StringVector paramTokens;
                                String::size_type lparen_pos = line.find('(', 0);
                                if(lparen_pos != String::npos)
                                {
                                    StringVector lineTokens = StringUtil::split(line, "(");
                                    paramTokens = StringUtil::split(lineTokens[1], ",");
                                }
                                else
                                {
                                    paramTokens = StringUtil::split(line, ",");
                                }

                                StringVector::const_iterator itParam;
                                for(itParam = paramTokens.begin(); itParam != paramTokens.end(); ++itParam)
                                {
                                    functionSig += *itParam;

                                    String::size_type rparen_pos = itParam->find(')', 0);
                                    if(rparen_pos == String::npos)
                                        functionSig += ",";
                                }

                                String::size_type space_pos = line.find(')', 0);
                                if(space_pos != String::npos)
                                {
                                    foundEndOfSignature = true;
                                }
                                line = stream->getLine();
                            }

                            functionInvoc = createInvocationFromString(functionSig);

                            // Ok, now if we have found the signature, iterate through the file until we find the end
                            // of the function.
                            bool foundEndOfBody = false;
                            size_t braceCount = 0;
                            while(!foundEndOfBody)
                            {
                                functionBody += line;

                                String::size_type brace_pos = line.find('{', 0);
                                if(brace_pos != String::npos)
                                {
                                    braceCount++;
                                }

                                brace_pos = line.find('}', 0);
                                if(brace_pos != String::npos)
                                    braceCount--;

                                if(braceCount == 0)
                                {
                                    foundEndOfBody = true;

                                    // Remove first and last braces
                                    size_t pos = functionBody.find("{");
                                    functionBody.erase(pos, 1);
                                    pos = functionBody.rfind("}");
                                    functionBody.erase(pos, 1);
                                    mFunctionCacheMap.insert(FunctionMap::value_type(*functionInvoc, functionBody));
                                }
                                functionBody += "\n";
                                line = stream->getLine();
                            }
                        }
                    }
                }
            }

            stream->close();
        }

    }
}
