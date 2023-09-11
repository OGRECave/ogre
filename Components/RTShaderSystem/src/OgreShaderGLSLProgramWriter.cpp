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
GLSLProgramWriter::GLSLProgramWriter() : mIsGLSLES(false), mIsVulkan(false)
{
    auto* rs = Root::getSingleton().getRenderSystem();
    mGLSLVersion = rs ? rs->getNativeShadingLanguageVersion() : 120;

    if(rs && rs->getCapabilities()->isShaderProfileSupported("spirv"))
    {
        mGLSLVersion = 460;
        mIsVulkan = true;
    }

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
    mParamSemanticToNameMap[Parameter::SPS_POSITION] = "vertex";
    mParamSemanticToNameMap[Parameter::SPS_NORMAL] = "normal";
    mParamSemanticToNameMap[Parameter::SPS_TANGENT] = "tangent";
    mParamSemanticToNameMap[Parameter::SPS_BINORMAL] = "binormal";
    mParamSemanticToNameMap[Parameter::SPS_BLEND_INDICES] = "blendIndices";
    mParamSemanticToNameMap[Parameter::SPS_BLEND_WEIGHTS] = "blendWeights";

    mParamSemanticToNameMap[Parameter::SPS_COLOR] = "colour";
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeSourceCode(std::ostream& os, Program* program)
{
    // Write the current version (this force the driver to more fulfill the glsl standard)
    os << "#version "<< mGLSLVersion << std::endl;

    // Generate dependencies.
    writeProgramDependencies(os, program);
    os << std::endl;

    writeMainSourceCode(os, program);
}

void GLSLProgramWriter::writeUniformBlock(std::ostream& os, const String& name, int binding,
                                          const UniformParameterList& uniforms)
{
    os << "layout(binding = " << binding << ", row_major) uniform " << name << " {\n";

    for (const auto& uparam : uniforms)
    {
        if(uparam->getType() == GCT_MATRIX_3X4 || uparam->getType() == GCT_MATRIX_2X4)
            os << "layout(column_major) ";
        writeParameter(os, uparam);
        os << ";\n";
    }

    os << "};\n";
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

    const UniformParameterList& parameterList = program->getParameters();

    // Generate global variable code.
    writeUniformParametersTitle(os, program);
    os << std::endl;

    auto* rs = Root::getSingleton().getRenderSystem();
    auto hasSSO = rs ? rs->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS) : false;

    // Write the uniforms
    UniformParameterList uniforms;
    for (const auto& param : parameterList)
    {
        if(!param->isSampler())
        {
            uniforms.push_back(param);
            continue;
        }
        writeSamplerParameter(os, param);
        os << ";" << std::endl;
    }
    if (mIsVulkan && !uniforms.empty())
    {
        writeUniformBlock(os, "OgreUniforms", gpuType, uniforms);
        uniforms.clear();
    }

    int uniformLoc = 0;
    for (const auto& uparam : uniforms)
    {
        if(mGLSLVersion >= 430 && hasSSO)
        {
            os << "layout(location = " << uniformLoc << ") ";
            auto esize = GpuConstantDefinition::getElementSize(uparam->getType(), true) / 4;
            uniformLoc += esize * std::max<int>(uparam->getSize(), 1);
        }

        os << "uniform\t";
        if(mIsGLSLES)
            os << "highp\t"; // force highp to avoid precision mismatch between VP/ FP
        writeParameter(os, uparam);
        os << ";\n";
    }
    os << std::endl;

    Function* curFunction = program->getMain();
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
        writeParameter(os, *itParam);
        os << ";" << std::endl;
    }
    os << std::endl;

    for (const auto& pFuncInvoc : curFunction->getAtomInstances())
    {
        redirectGlobalWrites(os, pFuncInvoc, inParams, parameterList);
        for (auto& operand : pFuncInvoc->getOperandList())
        {
            const ParameterPtr& param = operand.getParameter();
            if (gpuType != GPT_VERTEX_PROGRAM || param->getSemantic() != Parameter::SPS_TEXTURE_COORDINATES)
                continue;

            bool isInputParam = std::find(inParams.begin(), inParams.end(), param) != inParams.end();

            // Now that every texcoord is a vec4 (passed as vertex attributes) we
            // have to swizzle them according the desired type.
            if (isInputParam)
                operand.setMaskToParamType();
        }

        os << "\t";
        pFuncInvoc->writeSourceCode(os, getTargetLanguage());
        os << std::endl;
    }
    os << "}" << std::endl;
    os << std::endl;
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeProgramDependencies(std::ostream& os, Program* program)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "//                         PROGRAM DEPENDENCIES" << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "#include <OgreUnifiedShader.h>" << std::endl;

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

    int psInLocation = 0;

    for ( ; itParam != itParamEnd; ++itParam)
    {
        const ParameterPtr& pParam = *itParam;
        auto paramContent = pParam->getContent();
        auto paramSemantic = pParam->getSemantic();
        const String& paramName = pParam->getName();

        if (gpuType == GPT_FRAGMENT_PROGRAM)
        {
            if(paramContent == Parameter::SPC_POINTSPRITE_COORDINATE)
            {
                pParam->_rename("gl_PointCoord");
                continue;
            }
            else if(paramSemantic == Parameter::SPS_POSITION)
            {
                pParam->_rename("gl_FragCoord");
                continue;
            }
            else if(paramSemantic == Parameter::SPS_FRONT_FACING)
            {
                pParam->_rename("gl_FrontFacing");
                continue;
            }

            os << "IN(";
            if(pParam->isHighP())
                os << "f32"; // rely on unified shader vor f32vec4 etc.
            os << mGpuConstTypeMap[pParam->getType()];
            os << "\t";
            os << paramName;
            os << ", " << psInLocation++ << ")\n";
        }
        else if (gpuType == GPT_VERTEX_PROGRAM)
        {
            // Due the fact that glsl does not have register like cg we have to rename the params
            if(paramSemantic == Parameter::SPS_TEXTURE_COORDINATES)
                pParam->_rename(StringUtil::format("uv%d", pParam->getIndex()));
            else if(paramContent == Parameter::SPC_COLOR_SPECULAR)
                pParam->_rename("secondary_colour");
            else
                pParam->_rename(mParamSemanticToNameMap[paramSemantic]);

            os << "IN(";
            // all uv texcoords passed by ogre are at least vec4
            if ((paramSemantic == Parameter::SPS_TEXTURE_COORDINATES) && (pParam->getType() < GCT_FLOAT4))
            {
                os << "vec4";
            }
            else
            {
                // the gl rendersystems only pass float attributes
                GpuConstantType type = pParam->getType();
                if(!mIsVulkan && !GpuConstantDefinition::isFloat(type))
                    type = GpuConstantType(type & ~GpuConstantDefinition::getBaseType(type));
                os << mGpuConstTypeMap[type];
            }
            os << "\t";
            os << pParam->getName() << ", ";
            writeParameterSemantic(os, pParam);  // maps to location
            os << ")\n";
        }
    }
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeOutParameters(std::ostream& os, Function* function, GpuProgramType gpuType)
{
    const ShaderParameterList& outParams = function->getOutputParameters();
    int vsOutLocation = 0;

    for (const auto& p : outParams)
    {
        if(gpuType == GPT_VERTEX_PROGRAM)
        {
            // GLSL vertex program has to write always gl_Position (but this is also deprecated after version 130)
            if(p->getSemantic() == Parameter::SPS_POSITION)
            {
                p->_rename("gl_Position");
            }
            else if(p->getContent() == Parameter::SPC_POINTSPRITE_SIZE)
            {
                p->_rename("gl_PointSize");
            }
            else
            {
                os << "OUT(";

                // In the vertex and fragment program the variable names must match.
                // Unfortunately now the input params are prefixed with an 'i' and output params with 'o'.
                // Thats why we rename the params which are used in function atoms
                String paramName = p->getName();
                paramName[0] = 'i';
                p->_rename(paramName);

                writeParameter(os, p);
                os << ", " << vsOutLocation++ << ")\n";
            }
        }
        else if(gpuType == GPT_FRAGMENT_PROGRAM &&
                p->getSemantic() == Parameter::SPS_COLOR)
        {
            if(p->getIndex() == 0)
            {
                // handled by UnifiedShader
                p->_rename("gl_FragColor");
                continue;
            }

            os << "OUT(vec4\t" << p->getName() << ", " << p->getIndex() << ")\n";
        }
    }
}
//-----------------------------------------------------------------------
}
}
