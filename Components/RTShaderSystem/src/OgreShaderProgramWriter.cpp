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

#include "OgreShaderProgramWriter.h"
#include "OgreStringConverter.h"

namespace Ogre {
namespace RTShader {

//-----------------------------------------------------------------------
ProgramWriter::ProgramWriter(const String& language)
{
	mTargetLanguage = language;
	initializeStringMaps();
}

//-----------------------------------------------------------------------
ProgramWriter::~ProgramWriter()
{

}

void ProgramWriter::initializeStringMaps()
{
	if (getTargetLanguage() == "cg")
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


		mParamSemanticMap[Parameter::SPS_POSITION] = "POSITION";
		mParamSemanticMap[Parameter::SPS_BLEND_WEIGHTS] = "BLENDWEIGHT";
		mParamSemanticMap[Parameter::SPS_BLEND_INDICES] = "BLENDINDICES";
		mParamSemanticMap[Parameter::SPS_NORMAL] = "NORMAL";
		mParamSemanticMap[Parameter::SPS_COLOR] = "COLOR";
		mParamSemanticMap[Parameter::SPS_TEXTURE_COORDINATES] = "TEXCOORD";
		mParamSemanticMap[Parameter::SPS_BINORMAL] = "BINORMAL";
		mParamSemanticMap[Parameter::SPS_TANGENT] = "TANGENT";
		mParamSemanticMap[Parameter::SPS_UNKNOWN] = "";
	}
}

//-----------------------------------------------------------------------
void ProgramWriter::writeSourceCode(std::ostream& os, Program* program)
{
	const ShaderFunctionList& functionList = program->getFunctions();
	ShaderFunctionConstIterator itFunction;

	const ShaderParameterList& parameterList = program->getParameters();
	ShaderParameterConstIterator itParam = parameterList.begin();
	
	// Generate source code header.
	writeProgramTitle(os, program);
	os << std::endl;

	// Generate dependencies.
	writeProgramDependencies(os, program);
	os << std::endl;

	// Generate global variable code.
	writeUniformParametersTitle(os, program);
	os << std::endl;

	for (itParam=parameterList.begin();  itParam != parameterList.end(); ++itParam)
	{
		writeUniformParameter(os, *itParam);			
		os << ";" << std::endl;				
	}
	os << std::endl;

	// Write program function(s).
	for (itFunction=functionList.begin(); itFunction != functionList.end(); ++itFunction)
	{
		Function* curFunction = *itFunction;

		writeFunctionTitle(os, curFunction);
		writeFunctionDeclaration(os, curFunction);
		
		os << "{" << std::endl;

		// Write local parameters.
		const ShaderParameterList& localParams = curFunction->getLocalParameters();

		for (itParam=localParams.begin();  itParam != localParams.end(); ++itParam)
		{
			os << "\t";
			writeLocalParameter(os, *itParam);			
			os << ";" << std::endl;						
		}


		// Sort and write function atoms.
		curFunction->sortAtomInstances();

		const FunctionAtomInstanceList& atomInstances = curFunction->getAtomInstances();
		FunctionAtomInstanceConstIterator itAtom;
		
		for (itAtom=atomInstances.begin(); itAtom != atomInstances.end(); ++itAtom)
		{			
			writeAtomInstance(os, *itAtom);
		}
		

		os << "}" << std::endl;
	}

	os << std::endl;
}

//-----------------------------------------------------------------------
void ProgramWriter::writeProgramTitle(std::ostream& os, Program* program)
{
	os << "//-----------------------------------------------------------------------------" << std::endl;
	os << "// Program Type: ";
	switch(program->getType())
	{
	case GPT_VERTEX_PROGRAM:
		os << "Vertex shader";
		break;
	case GPT_FRAGMENT_PROGRAM:
		os << "Fragment shader";
		break;
	case GPT_GEOMETRY_PROGRAM:
		os << "Geometry shader";
		break;	
	default:
		break;
	}
	os << std::endl;
	os << "// Language: " <<  getTargetLanguage() << std::endl;
	os << "// Created by Ogre RT Shader System. All rights reserved." << std::endl;
	os << "//-----------------------------------------------------------------------------" << std::endl;
}

//-----------------------------------------------------------------------
void ProgramWriter::writeProgramDependencies(std::ostream& os, Program* program)
{
	os << "//-----------------------------------------------------------------------------" << std::endl;
	os << "//                         PROGRAM DEPENDENCIES" << std::endl;
	os << "//-----------------------------------------------------------------------------" << std::endl;


	for (unsigned int i=0; i < program->getDependencyCount(); ++i)
	{
		const String& curDependency = program->getDependency(i);

		os << "#include " << '\"' << curDependency << "." << getTargetLanguage() << '\"' << std::endl;
	}
}

//-----------------------------------------------------------------------
void ProgramWriter::writeUniformParametersTitle(std::ostream& os, Program* program)
{
	os << "//-----------------------------------------------------------------------------" << std::endl;
	os << "//                         GLOBAL PARAMETES" << std::endl;
	os << "//-----------------------------------------------------------------------------" << std::endl;
}

//-----------------------------------------------------------------------
void ProgramWriter::writeFunctionTitle(std::ostream& os, Function* function)
{
	os << "//-----------------------------------------------------------------------------" << std::endl;
	os << "// Function Name: " <<  function->getName() << std::endl;
	os << "// Function Desc: " <<  function->getDescription() << std::endl;
	os << "//-----------------------------------------------------------------------------" << std::endl;
}


//-----------------------------------------------------------------------
void ProgramWriter::writeUniformParameter(std::ostream& os, ParameterPtr parameter)
{
	os << mGpuConstTypeMap[parameter->getType()];
	os << "\t";	
	os << parameter->getName();	

	if (parameter->isSampler())
	{
		if (getTargetLanguage() == "cg")
		{
			os << " : register(s" << parameter->getIndex() << ")";
		}		
	}
	
}

//-----------------------------------------------------------------------
void ProgramWriter::writeFunctionParameter(std::ostream& os, ParameterPtr parameter)
{
	os << mGpuConstTypeMap[parameter->getType()];
	os << "\t";	
	os << parameter->getName();	

	if (parameter->getSemantic() != Parameter::SPS_UNKNOWN)
	{
		os << " : " << mParamSemanticMap[parameter->getSemantic()];				
		if (parameter->getSemantic() != Parameter::SPS_POSITION && parameter->getIndex() >= 0)
		{			
			os << StringConverter::toString(parameter->getIndex()).c_str();
		}
	}
}

//-----------------------------------------------------------------------
void ProgramWriter::writeLocalParameter(std::ostream& os, ParameterPtr parameter)
{
	os << mGpuConstTypeMap[parameter->getType()];
	os << "\t";	
	os << parameter->getName();		
}

//-----------------------------------------------------------------------
void ProgramWriter::writeFunctionDeclaration(std::ostream& os, Function* function)
{
	const ShaderParameterList& inParams  = function->getInputParameters();
	const ShaderParameterList& outParams = function->getOutputParameters();

	
	os << "void";
	os << " ";

	os << function->getName();
	os << std::endl << "\t(" << std::endl;

	ShaderParameterConstIterator it;
	size_t paramsCount = inParams.size() + outParams.size();
	size_t curParamIndex = 0;

	// Write input parameters.
	for (it=inParams.begin(); it != inParams.end(); ++it)
	{					
		os << "\t in ";
		writeFunctionParameter(os, *it);

		if (curParamIndex + 1 != paramsCount)		
			os << ", " << std::endl;
		
		curParamIndex++;
	}


	// Write output parameters.
	for (it=outParams.begin(); it != outParams.end(); ++it)
	{
		os << "\t out ";
		writeFunctionParameter(os, *it);

		if (curParamIndex + 1 != paramsCount)				
			os << ", " << std::endl;

		curParamIndex++;
	}	
	
	os << std::endl << "\t)" << std::endl;
}

//-----------------------------------------------------------------------
void ProgramWriter::writeAtomInstance(std::ostream& os, FunctionAtom* atom)
{
	os << std::endl << "\t";
	atom->writeSourceCode(os, getTargetLanguage());
	os << std::endl;
}


}
}