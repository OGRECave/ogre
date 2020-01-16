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

#include "OgreShaderPrecompiledHeaders.h"

namespace Ogre {
namespace RTShader {

String GLSLProgramWriter::TargetLanguage = "glsl";

//-----------------------------------------------------------------------
GLSLProgramWriter::GLSLProgramWriter() : mIsGLSLES(false)
{
    auto* rs = Root::getSingleton().getRenderSystem();
    mGLSLVersion = rs ? rs->getNativeShadingLanguageVersion() : 120;
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
    mGpuConstTypeMap[GCT_SAMPLER1D] = "sampler1D";
    mGpuConstTypeMap[GCT_SAMPLER2D] = "sampler2D";
    mGpuConstTypeMap[GCT_SAMPLER2DARRAY] = "sampler2DArray";
    mGpuConstTypeMap[GCT_SAMPLER3D] = "sampler3D";
    mGpuConstTypeMap[GCT_SAMPLERCUBE] = "samplerCube";
    mGpuConstTypeMap[GCT_SAMPLER1DSHADOW] = "sampler1DShadow";
    mGpuConstTypeMap[GCT_SAMPLER2DSHADOW] = "sampler2DShadow";
    mGpuConstTypeMap[GCT_SAMPLER_EXTERNAL_OES] = "samplerExternalOES";
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
const char* GLSLProgramWriter::getGL3CompatDefines()
{
    // Redefine texture functions to maintain reusability
    return "#define texture1D texture\n"
           "#define texture2D texture\n"
           "#define shadow2DProj textureProj\n"
           "#define texture3D texture\n"
           "#define textureCube texture\n"
           "#define texture2DLod textureLod\n";
}

void GLSLProgramWriter::writeSourceCode(std::ostream& os, Program* program)
{
    // Write the current version (this force the driver to more fulfill the glsl standard)
    os << "#version "<< mGLSLVersion << std::endl;

    if(mGLSLVersion > 120)
    {
        os << getGL3CompatDefines();
    }

    // Generate dependencies.
    writeProgramDependencies(os, program);
    os << std::endl;

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
        const ShaderParameterList& inParams = curFunction->getInputParameters();

        writeFunctionTitle(os, curFunction);
        
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

        for (const auto& pFuncInvoc : curFunction->getAtomInstances())
        {
            for (auto& operand : pFuncInvoc->getOperandList())
            {
                const ParameterPtr& param = operand.getParameter();
                Operand::OpSemantic opSemantic = operand.getSemantic();

                bool isInputParam =
                    std::find(inParams.begin(), inParams.end(), param) != inParams.end();

                if (opSemantic == Operand::OPS_OUT || opSemantic == Operand::OPS_INOUT)
                {
                    // Check if we write to an input variable because they are only readable
                    // Well, actually "attribute" were writable in GLSL < 120, but we dont care here
                    bool doLocalRename = isInputParam;

                    // If its not a varying param check if a uniform is written
                    if (!doLocalRename)
                    {
                        doLocalRename = std::find(parameterList.begin(), parameterList.end(),
                                                  param) != parameterList.end();
                    }

                    // now we check if we already declared a redirector var
                    if(doLocalRename && mLocalRenames.find(param->getName()) == mLocalRenames.end())
                    {
                        // Declare the copy variable and assign the original
                        String newVar = "local_" + param->getName();
                        os << "\t" << mGpuConstTypeMap[param->getType()] << " " << newVar << " = " << param->getName() << ";" << std::endl;

                        // From now on we replace it automatic
                        param->_rename(newVar, true);
                        mLocalRenames.insert(newVar);
                    }
                }

                // Now that every texcoord is a vec4 (passed as vertex attributes) we
                // have to swizzle them according the desired type.
                if (gpuType == GPT_VERTEX_PROGRAM && isInputParam &&
                    param->getSemantic() == Parameter::SPS_TEXTURE_COORDINATES)
                {
                    operand.setMaskToParamType();
                }
            }

            os << "\t";
            pFuncInvoc->writeSourceCode(os, getTargetLanguage());
            os << std::endl;
        }
        os << "}" << std::endl;
    }
    os << std::endl;
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeProgramDependencies(std::ostream& os, Program* program)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "//                         PROGRAM DEPENDENCIES" << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;

    for (unsigned int i=0; i < program->getDependencyCount(); ++i)
    {
        os << "#include \"" << program->getDependency(i) << ".glsl\"" << std::endl;
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
        const String& paramName = pParam->getName();

        if (gpuType == GPT_FRAGMENT_PROGRAM)
        {
            if(paramContent == Parameter::SPC_POINTSPRITE_COORDINATE)
            {
                pParam->_rename("gl_PointCoord");
                continue;
            }
            else if(paramContent == Parameter::SPC_POSITION_PROJECTIVE_SPACE)
            {
                pParam->_rename("gl_FragCoord");
                continue;
            }

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
            pParam->_rename(mContentToPerVertexAttributes[paramContent]);

            // After GLSL 1.20 attribute is deprecated
            if (mGLSLVersion > 120 || (mGLSLVersion > 100 && mIsGLSLES))
            {
                os << "in\t";
            }
            else
            {
                os << "attribute\t";
            }

            // all uv texcoords passed by ogre are at least vec4
            if ((paramContent == Parameter::SPC_TEXTURE_COORDINATE0 ||
                 paramContent == Parameter::SPC_TEXTURE_COORDINATE1 ||
                 paramContent == Parameter::SPC_TEXTURE_COORDINATE2 ||
                 paramContent == Parameter::SPC_TEXTURE_COORDINATE3 ||
                 paramContent == Parameter::SPC_TEXTURE_COORDINATE4 ||
                 paramContent == Parameter::SPC_TEXTURE_COORDINATE5 ||
                 paramContent == Parameter::SPC_TEXTURE_COORDINATE6 ||
                 paramContent == Parameter::SPC_TEXTURE_COORDINATE7) &&
                (pParam->getType() < GCT_FLOAT4))
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
            pParam->_rename("gl_Color");
        }
        else if(paramContent == Parameter::SPC_COLOR_SPECULAR && !mIsGLSLES)
        {
            pParam->_rename("gl_SecondaryColor");
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
                pParam->_rename("gl_Position");
            }
            else if(pParam->getContent() == Parameter::SPC_POINTSPRITE_SIZE)
            {
                pParam->_rename("gl_PointSize");
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

                // In the vertex and fragment program the variable names must match.
                // Unfortunately now the input params are prefixed with an 'i' and output params with 'o'.
                // Thats why we rename the params which are used in function atoms
                String paramName = pParam->getName();
                paramName[0] = 'i';
                pParam->_rename(paramName);

                os << mGpuConstTypeMap[pParam->getType()];
                os << "\t";
                os << paramName;
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
            // GLSL fragment program has to write always gl_FragColor (but this is also deprecated after version 120)
            // Always add gl_FragColor as an output.  The name is for compatibility.
            if(mGLSLVersion <= 120 || (mIsGLSLES && mGLSLVersion == 100))
            {
                pParam->_rename("gl_FragColor");
            }
            else
            {
                os << "out vec4\t" << pParam->getName() << ";" << std::endl;
            }
        }
    }
    
    if(gpuType == GPT_VERTEX_PROGRAM && !mIsGLSLES) // TODO: also use for GLSLES?
    {
        // Special case where gl_Position needs to be redeclared
        if (mGLSLVersion >= 150 && Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(
                                       RSC_GLSL_SSO_REDECLARE))
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
