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

#include "OgreShaderGLSLProgramWriter.h"
#include "OgreShaderProgram.h"
#include "OgreRoot.h"
#include "OgreString.h"

namespace Ogre {
namespace RTShader {

String GLSLProgramWriter::TargetLanguage = "glsl";

//-----------------------------------------------------------------------
GLSLProgramWriter::GLSLProgramWriter() : mIsGLSLES(false)
{
    mGLSLVersion = Ogre::Root::getSingleton().getRenderSystem()->getNativeShadingLanguageVersion();
    initializeStringMaps();
}

//-----------------------------------------------------------------------
GLSLProgramWriter::~GLSLProgramWriter()
{

}

//-----------------------------------------------------------------------
void GLSLProgramWriter::initializeStringMaps()
{
    // basic glsl types
    mGpuConstTypeMap[GCT_FLOAT1] = "float";
    mGpuConstTypeMap[GCT_FLOAT2] = "vec2";
    mGpuConstTypeMap[GCT_FLOAT3] = "vec3";
    mGpuConstTypeMap[GCT_FLOAT4] = "vec4";
    mGpuConstTypeMap[GCT_SAMPLER1D] = mIsGLSLES ? "sampler2D" : "sampler1D";
    mGpuConstTypeMap[GCT_SAMPLER2D] = "sampler2D";
    mGpuConstTypeMap[GCT_SAMPLER2DARRAY] = "sampler2DArray";
    mGpuConstTypeMap[GCT_SAMPLER3D] = "sampler3D";
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
    mGpuConstTypeMap[GCT_INT2] = "ivec2";
    mGpuConstTypeMap[GCT_INT3] = "ivec3";
    mGpuConstTypeMap[GCT_INT4] = "ivec4";
    mGpuConstTypeMap[GCT_UINT1] = "uint";
    mGpuConstTypeMap[GCT_UINT2] = "uvec2";
    mGpuConstTypeMap[GCT_UINT3] = "uvec3";
    mGpuConstTypeMap[GCT_UINT4] = "uvec4";

    // Custom vertex attributes defined http://www.ogre3d.org/docs/manual/manual_21.html
    mContentToPerVertexAttributes[Parameter::SPC_POSITION_OBJECT_SPACE] = "vertex";
    mContentToPerVertexAttributes[Parameter::SPC_NORMAL_OBJECT_SPACE] = "normal";
    mContentToPerVertexAttributes[Parameter::SPC_TANGENT_OBJECT_SPACE] = "tangent";
    mContentToPerVertexAttributes[Parameter::SPC_BINORMAL_OBJECT_SPACE] = "binormal";
    mContentToPerVertexAttributes[Parameter::SPC_BLEND_INDICES] = "blendIndices";
    mContentToPerVertexAttributes[Parameter::SPC_BLEND_WEIGHTS] = "blendWeights";

    mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE0] = "uv0";
    mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE1] = "uv1";
    mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE2] = "uv2";
    mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE3] = "uv3";
    mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE4] = "uv4";
    mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE5] = "uv5";
    mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE6] = "uv6";
    mContentToPerVertexAttributes[Parameter::SPC_TEXTURE_COORDINATE7] = "uv7";  

    if (mGLSLVersion >= 130 || mIsGLSLES)
    {
        mContentToPerVertexAttributes[Parameter::SPC_COLOR_DIFFUSE] = "colour";
        mContentToPerVertexAttributes[Parameter::SPC_COLOR_SPECULAR] = "secondary_colour";
    }
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeSourceCode(std::ostream& os, Program* program)
{
    // Write the current version (this force the driver to more fulfill the glsl standard)
    os << "#version "<< mGLSLVersion << std::endl;

    // Generate source code header.
    writeProgramTitle(os, program);
    os<< std::endl;

    // Write forward declarations
    writeForwardDeclarations(os, program);
    os<< std::endl;

    writeMainSourceCode(os, program);
}

void GLSLProgramWriter::writeMainSourceCode(std::ostream& os, Program* program)
{
    GpuProgramType gpuType = program->getType();
    if(gpuType == GPT_GEOMETRY_PROGRAM)
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
            "Geometry Program not supported in GLSL writer ",
            "GLSLProgramWriter::writeSourceCode" );
    }

    // Clear out old input params
    mFragInputParams.clear();

    const ShaderFunctionList& functionList = program->getFunctions();
    ShaderFunctionConstIterator itFunction;

    const UniformParameterList& parameterList = program->getParameters();
    UniformParameterConstIterator itUniformParam = parameterList.begin();
    
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
    for (itFunction=functionList.begin(); itFunction != functionList.end(); ++itFunction)
    {
        Function* curFunction = *itFunction;

        writeFunctionTitle(os, curFunction);
        
        // Clear output mapping this map is used when we use
        // glsl built in types like gl_Color for example
        mInputToGLStatesMap.clear();

        // Write inout params and fill mInputToGLStatesMap
        writeInputParameters(os, curFunction, gpuType);
        writeOutParameters(os, curFunction, gpuType);
                    
        // The function name must always main.
        os << "void main(void) {" << std::endl;

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
        
        const FunctionAtomInstanceList& atomInstances = curFunction->getAtomInstances();
        FunctionAtomInstanceConstIterator itAtom = atomInstances.begin();
        FunctionAtomInstanceConstIterator itAtomEnd = atomInstances.end();

        for (; itAtom != itAtomEnd; ++itAtom)
        {       
            FunctionInvocation*  pFuncInvoc = (FunctionInvocation*)*itAtom;
            FunctionInvocation::OperandVector::iterator itOperand = pFuncInvoc->getOperandList().begin();
            FunctionInvocation::OperandVector::iterator itOperandEnd = pFuncInvoc->getOperandList().end();

            // Local string stream
            StringStream localOs;

            // Write function name          
            localOs << "\t" << pFuncInvoc->getFunctionName() << "(";

            ushort curIndLevel = 0;

            for (; itOperand != itOperandEnd; )
            {
                const ParameterPtr& param = itOperand->getParameter();
                Operand::OpSemantic opSemantic = itOperand->getSemantic();
                Parameter::Content content = param->getContent();

                if (opSemantic == Operand::OPS_OUT || opSemantic == Operand::OPS_INOUT)
                {
                    // Is the written parameter a varying 
                    bool isVarying = false;

                    // Check if we write to an varying because the are only readable in fragment programs 
                    if (gpuType == GPT_FRAGMENT_PROGRAM)
                    {
                        StringVector::iterator itFound = std::find(
                            mFragInputParams.begin(), mFragInputParams.end(), param->getName());
                        if (itFound != mFragInputParams.end())
                        {
                            // Declare the copy variable
                            String newVar = "local_" + param->getName();
                            String tempVar = param->getName();
                            isVarying = true;

                            // We stored the original values in the mFragInputParams thats why we have to replace the first var with o
                            // because all vertex output vars are prefixed with o in glsl the name has to match in the fragment program.
                            tempVar.replace(tempVar.begin(), tempVar.begin() + 1, "o");

                            // Declare the copy variable and assign the original
                            os << "\t" << mGpuConstTypeMap[param->getType()] << " " << newVar << " = " << tempVar << ";\n" << std::endl;

                            // From now on we replace it automatic 
                            mInputToGLStatesMap[param->getName()] = newVar;

                            // Remove the param because now it is replaced automatic with the local variable
                            // (which could be written).
                            mFragInputParams.erase(itFound++);
                        }
                    }
                    
                    // If its not a varying param check if a uniform is written
                    if(!isVarying)
                    {
                        UniformParameterList::const_iterator itFound =
                            std::find_if(parameterList.begin(), parameterList.end(),
                                         std::bind2nd(CompareUniformByName(), param->getName()));
                        if (itFound != parameterList.end())
                        {   
                            // Declare the copy variable
                            String newVar = "local_" + param->getName();

                            // now we check if we already declared a uniform redirector var
                            if(mInputToGLStatesMap.find(newVar) == mInputToGLStatesMap.end())
                            {
                                // Declare the copy variable and assign the original
                                os << "\t" << mGpuConstTypeMap[itFound->get()->getType()] << " " << newVar << " = " << param->getName() << ";\n" << std::endl;

                                // From now on we replace it automatic 
                                mInputToGLStatesMap[param->getName()] = newVar;
                            }
                        }
                    }
                }

                if(mInputToGLStatesMap.find(param->getName()) != mInputToGLStatesMap.end())
                {
                    int mask = itOperand->getMask(); // our swizzle mask

                    // Here we insert the renamed param name
                    localOs << mInputToGLStatesMap[param->getName()];

                    if(mask != Operand::OPM_ALL)
                    {
                        localOs << "." << Operand::getMaskAsString(mask);
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
                        // Now generate the swizzel mask according
                        // the type.
                        switch(param->getType())
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
                    localOs << itOperand->toString();
                }
                
                ++itOperand;

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
        os << "}" << std::endl;
    }
    os << std::endl;
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeFunctionDeclaration(std::ostream& os, FunctionInvocation& func)
{
    os << func.getReturnType() << " " << func.getFunctionName() << "(";

    FunctionInvocation::OperandVector::iterator itOperand    = func.getOperandList().begin();
    FunctionInvocation::OperandVector::iterator itOperandEnd = func.getOperandList().end();
    for (; itOperand != itOperandEnd;)
    {
      const ParameterPtr& param = itOperand->getParameter();
      Operand::OpSemantic opSemantic = itOperand->getSemantic();
      int opMask = itOperand->getMask();
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
          gpuType = param->getType();
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
              "GLSLProgramWriter::writeFunctionDeclaration" );
      }

      // Write the operand type.
      os << mGpuConstTypeMap[gpuType] << " " << param->getName();

      ++itOperand;
      //move over all operators with indirection
      while ((itOperand != itOperandEnd) && (itOperand->getIndirectionLevel() != 0))
      {
          ++itOperand;
      }

      // Prepare for the next operand
      if (itOperand != itOperandEnd)
      {
          os << ", ";
      }
    }
    os << ")";
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeForwardDeclarations(std::ostream& os, Program* program)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "//                         FORWARD DECLARATIONS" << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;

    StringVector forwardDecl; // holds all generated function declarations 
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

            StringStream funcDecl;
            writeFunctionDeclaration(funcDecl, *pFuncInvoc);

            // Push the generated declaration into the vector
            // duplicate declarations will be removed later.
            forwardDecl.push_back(funcDecl.str());
        }
    }

    // Now remove duplicate declaration, first we have to sort the vector.
    std::sort(forwardDecl.begin(), forwardDecl.end());
    StringVector::iterator endIt = std::unique(forwardDecl.begin(), forwardDecl.end()); 

    // Finally write all function declarations to the shader file
    for (StringVector::iterator it = forwardDecl.begin(); it != endIt; ++it)
    {
        os << *it << ";\n";
    }
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeInputParameters(std::ostream& os, Function* function, GpuProgramType gpuType)
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
            if(paramContent == Parameter::SPC_POINTSPRITE_COORDINATE)
            {
                mInputToGLStatesMap[pParam->getName()] = "gl_PointCoord";
                continue;
            }

            // push fragment inputs they all could be written (in glsl you can not write
            // input params in the fragment program)
            mFragInputParams.push_back(paramName);

            // In the vertex and fragment program the variable names must match.
            // Unfortunately now the input params are prefixed with an 'i' and output params with 'o'.
            // Thats why we are using a map for name mapping (we rename the params which are used in function atoms).
            paramName.replace(paramName.begin(), paramName.begin() + 1, "o");   
            mInputToGLStatesMap[pParam->getName()] = paramName;

            // After GLSL 1.20 varying is deprecated
            if(mGLSLVersion <= 120 || (mGLSLVersion == 100 && mIsGLSLES))
            {
                os << "varying\t";
            }
            else
            {
                os << "in\t";
            }

            os << mGpuConstTypeMap[pParam->getType()];
            os << "\t"; 
            os << paramName;
            os << ";" << std::endl; 
        }
        else if (gpuType == GPT_VERTEX_PROGRAM && 
                 mContentToPerVertexAttributes.find(paramContent) != mContentToPerVertexAttributes.end())
        {
            // Due the fact that glsl does not have register like cg we have to rename the params
            // according there content.
            mInputToGLStatesMap[paramName] = mContentToPerVertexAttributes[paramContent];

            // After GLSL 1.40 attribute is deprecated
            if (mGLSLVersion >= 140 || (mGLSLVersion > 100 && mIsGLSLES))
            {
                os << "in\t";
            }
            else
            {
                os << "attribute\t";
            }

            // all uv texcoords passed by ogre are vec4
            if (paramContent == Parameter::SPC_TEXTURE_COORDINATE0 ||
                paramContent == Parameter::SPC_TEXTURE_COORDINATE1 ||
                paramContent == Parameter::SPC_TEXTURE_COORDINATE2 ||
                paramContent == Parameter::SPC_TEXTURE_COORDINATE3 ||
                paramContent == Parameter::SPC_TEXTURE_COORDINATE4 ||
                paramContent == Parameter::SPC_TEXTURE_COORDINATE5 ||
                paramContent == Parameter::SPC_TEXTURE_COORDINATE6 ||
                paramContent == Parameter::SPC_TEXTURE_COORDINATE7 )
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
        else if(paramContent == Parameter::SPC_COLOR_DIFFUSE && !mIsGLSLES)
        {
            mInputToGLStatesMap[paramName] = "gl_Color";
        }
        else if(paramContent == Parameter::SPC_COLOR_SPECULAR && !mIsGLSLES)
        {
            mInputToGLStatesMap[paramName] = "gl_SecondaryColor";
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
void GLSLProgramWriter::writeOutParameters(std::ostream& os, Function* function, GpuProgramType gpuType)
{
    const ShaderParameterList& outParams = function->getOutputParameters();

    ShaderParameterConstIterator itParam = outParams.begin();
    ShaderParameterConstIterator itParamEnd = outParams.end();

    for ( ; itParam != itParamEnd; ++itParam)
    {
        ParameterPtr pParam = *itParam;

        if(gpuType == GPT_VERTEX_PROGRAM)
        {
            // GLSL vertex program has to write always gl_Position (but this is also deprecated after version 130)
            if(pParam->getContent() == Parameter::SPC_POSITION_PROJECTIVE_SPACE)
            {
                mInputToGLStatesMap[pParam->getName()] = "gl_Position";
            }
            else if(pParam->getContent() == Parameter::SPC_POINTSPRITE_SIZE)
            {
                mInputToGLStatesMap[pParam->getName()] = "gl_PointSize";
            }
            else
            {
                // After GLSL 1.20 varying is deprecated
                if(mGLSLVersion <= 120 || (mGLSLVersion == 100 && mIsGLSLES))
                {
                    os << "varying\t";
                }
                else
                {
                    os << "out\t";
                }

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
            // GLSL fragment program has to write always gl_FragColor (but this is also deprecated after version 130)
            // Always add gl_FragColor as an output.  The name is for compatibility.
            if(mGLSLVersion <= 130 || (mIsGLSLES && mGLSLVersion == 100))
            {
                mInputToGLStatesMap[pParam->getName()] = "gl_FragColor";
            }
            else
            {
                os << "out vec4 fragColour;" << std::endl;
                mInputToGLStatesMap[pParam->getName()] = "fragColour";
            }
        }
    }
    
    if(gpuType == GPT_VERTEX_PROGRAM && !mIsGLSLES) // TODO: also use for GLSLES?
    {
        // Special case where gl_Position needs to be redeclared
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_GLSL_SSO_REDECLARE) &&
           mGLSLVersion >= 150)
        {
            os << "out gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\nfloat gl_ClipDistance[];\n};\n" << std::endl;
        }
    }
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeLocalParameter(std::ostream& os, ParameterPtr parameter)
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
}
}
