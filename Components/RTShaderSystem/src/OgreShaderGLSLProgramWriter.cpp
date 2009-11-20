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

#include "OgreShaderGLSLProgramWriter.h"
#include "OgreStringConverter.h"
#include "OgreShaderGenerator.h"

namespace Ogre {
namespace RTShader {

String GLSLProgramWriter::TargetLanguage =  "glsl";

//-----------------------------------------------------------------------
GLSLProgramWriter::GLSLProgramWriter()
{
	mGLSLVersion = 120;
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
	mGpuConstTypeMap[GCT_SAMPLER3D] = "sampler3D";
	mGpuConstTypeMap[GCT_SAMPLERCUBE] = "samplerCube";
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

	if (mGLSLVersion == 130)
	{
		mContentToPerVertexAttributes[Parameter::SPC_COLOR_DIFFUSE] = "colour";
		mContentToPerVertexAttributes[Parameter::SPC_COLOR_SPECULAR] = "secondary_colour";
	}
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeSourceCode(std::ostream& os, Program* program)
{
	GpuProgramType gpuType = program->getType();
	if(gpuType == GPT_GEOMETRY_PROGRAM)
	{
		OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, 
			"Geomtry Program not supported in GLSL writer ", 
			"ProgramWriterGLSL::writeSourceCode" );	
	}

	// Clear out old input params
	mFragInputParams.clear();

	const ShaderFunctionList& functionList = program->getFunctions();
	ShaderFunctionConstIterator itFunction;

	const ShaderParameterList& parameterList = program->getParameters();
	ShaderParameterConstIterator itParam = parameterList.begin();
	
	// Write the current version (this force the driver to more fulfill the glsl standard)
	os << "#version "<< mGLSLVersion << std::endl;

	// Generate source code header.
	writeProgramTitle(os, program);
	os<< std::endl;

	// Write forward declarations
	writeForwardDeclartions(os, program);
	os<< std::endl;
	
	// Generate global variable code.
	writeUniformParametersTitle(os, program);
	os << std::endl;

	// Write the uniforms 
	for (itParam = parameterList.begin();  itParam != parameterList.end(); ++itParam)
	{
		ParameterPtr pUniformParam = *itParam;

		os << "uniform\t"; 
		os << mGpuConstTypeMap[pUniformParam->getType()];
		os << "\t";	
		os << pUniformParam->getName();
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
				if (gpuType == GPT_FRAGMENT_PROGRAM &&
					(opSemantic == Operand::OPS_OUT || opSemantic == Operand::OPS_INOUT))
				{	
					StringVector::iterator itFound = std::find(mFragInputParams.begin(), mFragInputParams.end(), paramName);	
					if(itFound != mFragInputParams.end())
					{						
						// Declare the copy variable
						String newVar = "local_" + paramName;
						String tempVar = paramName;

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
						// Now generate the swizzel mask according
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
void GLSLProgramWriter::writeForwardDeclartions(std::ostream& os, Program* program)
{
	os << "//-----------------------------------------------------------------------------" << std::endl;
	os << "//                         FORWARD DECLARTIONS" << std::endl;
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
				
				//  Swizzle masks are only defined for types like vec2, vec3, vec4.
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
						"GLSLProgramWriter::writeForwardDeclartions" );	
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

	// Finally write all function declarations to the shader file
	for (StringVector::iterator it = forwardDecl.begin(); it != endIt; it++)
	{
		os << *it;
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
			// push fragment inputs they all could be written (in glsl you can not write
			// input params in the fragment program)
			mFragInputParams.push_back(paramName);

			// In the vertex and fragment program the variable names must match.
			// Unfortunately now the input params are prefixed with an 'i' and output params with 'o'.
			// Thats why we are using a map for name mapping (we rename the params which are used in function atoms).
			paramName.replace(paramName.begin(), paramName.begin() + 1, "o");	
			mInputToGLStatesMap[pParam->getName()] = paramName;

			// After glsl 120 varying is deprecated
			if(mGLSLVersion <=  120)
			{
				os << "varying\t";
			}
			else
			{
				os << "out\t";
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
			os << "attribute\t"; 

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
		else if(paramContent == Parameter::SPC_COLOR_DIFFUSE)
		{
			mInputToGLStatesMap[paramName] = "gl_Color";
		}
		else if(paramContent == Parameter::SPC_COLOR_SPECULAR)
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
			else
			{
				// After glsl 120 varying is deprecated
				if(mGLSLVersion <=  120)
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
				os << ";" << std::endl;	
			}
		}
		else if(gpuType == GPT_FRAGMENT_PROGRAM &&
				pParam->getSemantic() == Parameter::SPS_COLOR)
		{					
			// GLSL fragment program has to write always gl_FragColor (but this is also deprecated after version 130)
			mInputToGLStatesMap[pParam->getName()] = "gl_FragColor";						
		}
	}
}

//-----------------------------------------------------------------------
void GLSLProgramWriter::writeLocalParameter(std::ostream& os, ParameterPtr parameter)
{
	os << mGpuConstTypeMap[parameter->getType()];
	os << "\t";	
	os << parameter->getName();		
}
//-----------------------------------------------------------------------
}
}