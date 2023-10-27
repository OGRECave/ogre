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

String CGProgramWriter::TargetLanguage = "hlsl";

//-----------------------------------------------------------------------
CGProgramWriter::CGProgramWriter()
{
    initializeStringMaps();

    if(GpuProgramManager::getSingleton().isSyntaxSupported("ps_4_0"))
    {
        mGpuConstTypeMap[GCT_SAMPLER2DARRAY] = "Sampler2DArray";
        mGpuConstTypeMap[GCT_SAMPLER2DSHADOW] = "Sampler2DShadow";
    }
}

//-----------------------------------------------------------------------
CGProgramWriter::~CGProgramWriter()
{

}

//-----------------------------------------------------------------------
void CGProgramWriter::initializeStringMaps()
{
    mGpuConstTypeMap[GCT_FLOAT1] = "float";
    mGpuConstTypeMap[GCT_FLOAT2] = "float2";
    mGpuConstTypeMap[GCT_FLOAT3] = "float3";
    mGpuConstTypeMap[GCT_FLOAT4] = "float4";
    mGpuConstTypeMap[GCT_SAMPLER1D] = "sampler1D";
    mGpuConstTypeMap[GCT_SAMPLER2D] = "sampler2D";
    mGpuConstTypeMap[GCT_SAMPLER3D] = "sampler3D";
    mGpuConstTypeMap[GCT_SAMPLERCUBE] = "samplerCUBE";
    mGpuConstTypeMap[GCT_MATRIX_2X2] = "float2x2";
    mGpuConstTypeMap[GCT_MATRIX_2X3] = "float2x3";
    mGpuConstTypeMap[GCT_MATRIX_2X4] = "float2x4";
    mGpuConstTypeMap[GCT_MATRIX_3X2] = "float3x2";
    mGpuConstTypeMap[GCT_MATRIX_3X3] = "float3x3";
    mGpuConstTypeMap[GCT_MATRIX_3X4] = "float3x4";
    mGpuConstTypeMap[GCT_MATRIX_4X2] = "float4x2";
    mGpuConstTypeMap[GCT_MATRIX_4X3] = "float4x3";
    mGpuConstTypeMap[GCT_MATRIX_4X4] = "float4x4";
    mGpuConstTypeMap[GCT_INT1] = "int";
    mGpuConstTypeMap[GCT_INT2] = "int2";
    mGpuConstTypeMap[GCT_INT3] = "int3";
    mGpuConstTypeMap[GCT_INT4] = "int4";
    mGpuConstTypeMap[GCT_UINT1] = "uint";
    mGpuConstTypeMap[GCT_UINT2] = "uint2";
    mGpuConstTypeMap[GCT_UINT3] = "uint3";
    mGpuConstTypeMap[GCT_UINT4] = "uint4";

    mParamSemanticMap[Parameter::SPS_FRONT_FACING] = "VFACE";
}

//-----------------------------------------------------------------------
void CGProgramWriter::writeSourceCode(std::ostream& os, Program* program)
{
    // Generate source code header.
    writeProgramTitle(os, program);
    os << std::endl;

    // Generate dependencies.
    writeProgramDependencies(os, program);
    os << std::endl;

    // Generate global variable code.
    writeUniformParametersTitle(os, program);
    os << std::endl;

    for (const auto& p : program->getParameters())
    {
        p->isSampler() ? writeSamplerParameter(os, p) : writeParameter(os, p);
        os << ";" << std::endl;
    }
    os << std::endl;


    Function* curFunction = program->getMain();

    writeFunctionTitle(os, curFunction);
    writeFunctionDeclaration(os, curFunction);

    os << "{" << std::endl;

    // Write local parameters.
    for (const auto& p : curFunction->getLocalParameters())
    {
        os << "\t";
        writeParameter(os, p);
        os << ";" << std::endl;
    }

    for (const auto& a : curFunction->getAtomInstances())
    {
        redirectGlobalWrites(os, a, curFunction->getInputParameters(), program->getParameters());
        writeAtomInstance(os, a);
    }


    os << "}" << std::endl;
    os << std::endl;
}

//-----------------------------------------------------------------------
void CGProgramWriter::writeFunctionParameter(std::ostream& os, ParameterPtr parameter)
{
    writeParameter(os, parameter);
    os << " : ";
    writeParameterSemantic(os, parameter);
}

//-----------------------------------------------------------------------
void CGProgramWriter::writeFunctionDeclaration(std::ostream& os, Function* function)
{
    os << "void main(\n";

    // Write input parameters.
    for (const auto& p : function->getInputParameters())
    {
        os << "\t in ";
        writeFunctionParameter(os, p);
        os << ",\n";
    }


    // Write output parameters.
    for (const auto& p : function->getOutputParameters())
    {
        os << "\t out ";
        writeFunctionParameter(os, p);
        os << ",\n";
    }

    os.seekp(-2, std::ios_base::end);

    os << "\n)\n";
}

//-----------------------------------------------------------------------
void CGProgramWriter::writeAtomInstance(std::ostream& os, FunctionAtom* atom)
{
    os << std::endl << "\t";
    atom->writeSourceCode(os, getTargetLanguage());
    os << std::endl;
}

}
}
