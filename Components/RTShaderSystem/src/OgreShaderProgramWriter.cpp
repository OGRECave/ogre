/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

//-----------------------------------------------------------------------
void ProgramWriter::writeProgramTitle(std::ostream& os, Program* program)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "// Program Type: " << to_string(program->getType()) << std::endl;
    os << "// Language: " <<  getTargetLanguage() << std::endl;
    os << "// Created by Ogre RT Shader Generator. All rights reserved." << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;
}

//-----------------------------------------------------------------------
void ProgramWriter::writeUniformParametersTitle(std::ostream& os, Program* program)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "//                         GLOBAL PARAMETERS" << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;
}
//-----------------------------------------------------------------------
void ProgramWriter::writeFunctionTitle(std::ostream& os, Function* function)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "//                         MAIN" << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;
}

ProgramWriter::ProgramWriter()
{
    mParamSemanticMap[Parameter::SPS_POSITION] = "POSITION";
    mParamSemanticMap[Parameter::SPS_BLEND_WEIGHTS] = "BLENDWEIGHT";
    mParamSemanticMap[Parameter::SPS_BLEND_INDICES] = "BLENDINDICES";
    mParamSemanticMap[Parameter::SPS_NORMAL] = "NORMAL";
    mParamSemanticMap[Parameter::SPS_COLOR] = "COLOR";
    mParamSemanticMap[Parameter::SPS_TEXTURE_COORDINATES] = "TEXCOORD";
    mParamSemanticMap[Parameter::SPS_BINORMAL] = "BINORMAL";
    mParamSemanticMap[Parameter::SPS_TANGENT] = "TANGENT";
}

ProgramWriter::~ProgramWriter() {}

void ProgramWriter::writeParameter(std::ostream& os, const ParameterPtr& parameter)
{
    if (!parameter->getStructType().empty())
    {
        os << parameter->getStructType() << '\t' << parameter->getName();
        return;
    }

    os << mGpuConstTypeMap[parameter->getType()] << '\t' << parameter->getName();
    if (parameter->isArray())
        os << '[' << parameter->getSize() << ']';
}

void ProgramWriter::writeSamplerParameter(std::ostream& os, const UniformParameterPtr& parameter)
{
    if (parameter->getType() == GCT_SAMPLER_EXTERNAL_OES)
    {
        os << "uniform\t";
        writeParameter(os, parameter);
        return;
    }

    switch(parameter->getType())
    {
    case GCT_SAMPLER1D:
        os << "SAMPLER1D(";
        break;
    case GCT_SAMPLER2D:
        os << "SAMPLER2D(";
        break;
    case GCT_SAMPLER3D:
        os << "SAMPLER3D(";
        break;
    case GCT_SAMPLERCUBE:
        os << "SAMPLERCUBE(";
        break;
    case GCT_SAMPLER2DSHADOW:
        os << "SAMPLER2DSHADOW(";
        break;
    case GCT_SAMPLER2DARRAY:
        os << "SAMPLER2DARRAY(";
        break;
    default:
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "unsupported sampler type");
    }
    os << parameter->getName() << ", " << parameter->getIndex() << ")";
}

void ProgramWriter::writeParameterSemantic(std::ostream& os, const ParameterPtr& parameter)
{
    OgreAssertDbg(parameter->getSemantic() != Parameter::SPS_UNKNOWN, "invalid semantic");
    os << mParamSemanticMap[parameter->getSemantic()];

    if (parameter->getSemantic() == Parameter::SPS_TEXTURE_COORDINATES ||
        (parameter->getSemantic() == Parameter::SPS_COLOR && parameter->getIndex() > 0))
    {
        os << parameter->getIndex();
    }
}

void ProgramWriter::redirectGlobalWrites(std::ostream& os, FunctionAtom* func, const ShaderParameterList& inputs,
                                         const UniformParameterList& uniforms)
{
    for (auto& operand : func->getOperandList())
    {
        auto opSemantic = operand.getSemantic();

        if (opSemantic != Operand::OPS_OUT && opSemantic != Operand::OPS_INOUT)
            continue;

        const ParameterPtr& param = operand.getParameter();

        // Check if we write to an input variable because they are only readable
        // Well, actually "attribute" were writable in GLSL < 120, but we dont care here
        bool doLocalRename = std::find(inputs.begin(), inputs.end(), param) != inputs.end();

        // If its not a varying param check if a uniform is written
        if (!doLocalRename)
        {
            doLocalRename = std::find(uniforms.begin(), uniforms.end(), param) != uniforms.end();
        }

        // now we check if we already declared a redirector var
        if (doLocalRename && mLocalRenames.find(param->getName()) == mLocalRenames.end())
        {
            // Declare the copy variable and assign the original
            String newVar = "local_" + param->getName();
            os << "\t" << mGpuConstTypeMap[param->getType()] << " " << newVar << " = " << param->getName() << ";"
                << std::endl;

            // From now on we replace it automatic
            param->_rename(newVar, true);
            mLocalRenames.insert(newVar);
        }
    }
}
}
}
