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

#include "OgreShaderFunction.h"
#include "OgreShaderProgramManager.h"
#include "OgreStringConverter.h"

namespace Ogre {
namespace RTShader {
//-----------------------------------------------------------------------------
Function::Function(const String& name, const String& desc)
{
	m_name			= name;
	m_description	= desc;
}

//-----------------------------------------------------------------------------
Function::~Function()
{
	for (FunctionAtomInstanceIterator it=mAtomInstances.begin(); it != mAtomInstances.end(); ++it)		
		delete (*it);
	mAtomInstances.clear();

	for (ShaderParameterIterator it = mInputParameters.begin(); it != mInputParameters.end(); ++it)
		delete *it;
	mInputParameters.clear();

	for (ShaderParameterIterator it = mOutputParameters.begin(); it != mOutputParameters.end(); ++it)
		delete *it;
	mOutputParameters.clear();

	for (ShaderParameterIterator it = mLocalParameters.begin(); it != mLocalParameters.end(); ++it)
		delete *it;
	mLocalParameters.clear();

}

//-----------------------------------------------------------------------------
Parameter* Function::resolveInputParameter(Parameter::Semantic semantic,
												 int index,
												 GpuConstantType type)
{
	Parameter* param = NULL;

	// Case we have to create new parameter.
	if (index == -1)
	{
		index = 0;

		// Find the next available index of the target semantic.
		ShaderParameterIterator it;

		for (it = mInputParameters.begin(); it != mInputParameters.end(); ++it)
		{
			if ((*it)->getSemantic() == semantic)
			{
				index++;
			}
		}
	}
	else
	{
		// Check if desired parameter already defined.
		param = getParameterBySemantic(mInputParameters, semantic, index);
		if (param != NULL)
		{
			if (param->getType() == type)
			{
				return param;
			}
			else 
			{
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
					"Can not resolve parameter - semantic: " + StringConverter::toString(semantic) + " - index: " + StringConverter::toString(index) + " due to type mismatch. Function <" + getName() + ">", 			
					"ShaderFunction::resolveInputParameter" );
			}
		}
	}

	

	// No parameter found -> create new one.
	switch (semantic)
	{
	case Parameter::SPS_POSITION:	
		assert(type == GCT_FLOAT4);
		param = ParameterFactory::createInPosition(index);
		break;
			
	case Parameter::SPS_BLEND_WEIGHTS:			
	case Parameter::SPS_BLEND_INDICES:
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
					"Can not resolve parameter - semantic: " + StringConverter::toString(semantic) + " - index: " + StringConverter::toString(index) + " since support in it is not implemented yet. Function <" + getName() + ">", 			
					"ShaderFunction::resolveInputParameter" );
		break;
			
	case Parameter::SPS_NORMAL:
		assert(type == GCT_FLOAT3);
		param = ParameterFactory::createInNormal(index);
		break;
			
	case Parameter::SPS_COLOR:
		assert(type == GCT_FLOAT4);
		param = ParameterFactory::createInColor(index);
		break;
						
	case Parameter::SPS_TEXTURE_COORDINATES:		
		param = ParameterFactory::createInTexcoord(type, index);				
		break;
			
	case Parameter::SPS_BINORMAL:
		assert(type == GCT_FLOAT3);
		param = ParameterFactory::createInBiNormal(index);
		break;
			
	case Parameter::SPS_TANGENT:
		assert(type == GCT_FLOAT3);
		param = ParameterFactory::createInTangent(index);
		break;
	}

	if (param != NULL)
		addInputParameter(param);

	return param;
}

//-----------------------------------------------------------------------------
Parameter* Function::resolveOutputParameter(Parameter::Semantic semantic,												  
											int index,
											GpuConstantType type)
{
	Parameter* param = NULL;

	// Case we have to create new parameter.
	if (index == -1)
	{
		index = 0;

		// Find the next available index of the target semantic.
		ShaderParameterIterator it;

		for (it = mOutputParameters.begin(); it != mOutputParameters.end(); ++it)
		{
			if ((*it)->getSemantic() == semantic)
			{
				index++;
			}
		}
	}
	else
	{
		// Check if desired parameter already defined.
		param = getParameterBySemantic(mOutputParameters, semantic, index);
		if (param != NULL)
		{
			if (param->getType() == type)
			{
				return param;
			}
			else 
			{
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
					"Can not resolve parameter - semantic: " + StringConverter::toString(semantic) + " - index: " + StringConverter::toString(index) + " due to type mismatch. Function <" + getName() + ">", 			
					"ShaderFunction::resolveOutputParameter" );
			}
		}
	}
	

	// No parameter found -> create new one.
	switch (semantic)
	{
	case Parameter::SPS_POSITION:	
		assert(type == GCT_FLOAT4);
		param = ParameterFactory::createOutPosition(index);
		break;

	case Parameter::SPS_BLEND_WEIGHTS:		
	case Parameter::SPS_BLEND_INDICES:
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Can not resolve parameter - semantic: " + StringConverter::toString(semantic) + " - index: " + StringConverter::toString(index) + " since support in it is not implemented yet. Function <" + getName() + ">", 			
			"ShaderFunction::resolveOutputParameter" );
		break;

	case Parameter::SPS_NORMAL:
		assert(type == GCT_FLOAT3);
		param = ParameterFactory::createOutNormal(index);
		break;

	case Parameter::SPS_COLOR:
		assert(type == GCT_FLOAT4);
		param = ParameterFactory::createOutColor(index);
		break;

	case Parameter::SPS_TEXTURE_COORDINATES:		
		param = ParameterFactory::createOutTexcoord(type, index);				
		break;

	case Parameter::SPS_BINORMAL:
		assert(type == GCT_FLOAT3);
		param = ParameterFactory::createOutBiNormal(index);
		break;

	case Parameter::SPS_TANGENT:
		assert(type == GCT_FLOAT3);
		param = ParameterFactory::createOutTangent(index);
		break;
	}

	if (param != NULL)
		addOutputParameter(param);

	return param;
}

//-----------------------------------------------------------------------------
Parameter* Function::resolveLocalParameter(Parameter::Semantic semantic, int index, 
											GpuConstantType type, const String& name)
{
	Parameter* param = NULL;

	param = getParameterByName(mLocalParameters, name);

	if (param != NULL)
	{
		if (param->getType() == type &&
			param->getSemantic() == semantic &&
			param->getIndex() == index)
		{
			return param;
		}
		else 
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Can not resolve parameter " + name + " due to type mismatch. Function <" + getName() + ">", 			
				"ShaderFunction::resolveLocalParameter" );
		}		
	}
		
	param = new Parameter(type, name, semantic, index);
	addParameter(mLocalParameters, param);
			
	return param;
}

//-----------------------------------------------------------------------------
void Function::addInputParameter(Parameter* parameter)
{

	// Check that parameter with the same semantic and index in input parameters list.
	if (getParameterBySemantic(mInputParameters, parameter->getSemantic(), parameter->getIndex()) != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Parameter <" + parameter->getName() + "> has equal sematic parameter in function <" + getName() + ">", 			
			"ShaderFunction::addInputParameter" );
	}

	addParameter(mInputParameters, parameter);
}

//-----------------------------------------------------------------------------
void Function::addOutputParameter(Parameter* parameter)
{
	// Check that parameter with the same semantic and index in output parameters list.
	if (getParameterBySemantic(mOutputParameters, parameter->getSemantic(), parameter->getIndex()) != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Parameter <" + parameter->getName() + "> has equal sematic parameter in function <" + getName() + ">", 			
			"ShaderFunction::addOutputParameter" );
	}

	addParameter(mOutputParameters, parameter);
}

//-----------------------------------------------------------------------------
void Function::deleteInputParameter(Parameter* parameter)
{
	deleteParameter(mInputParameters, parameter);
}

//-----------------------------------------------------------------------------
void Function::deleteOutputParameter(Parameter* parameter)
{
	deleteParameter(mOutputParameters, parameter);
}

//-----------------------------------------------------------------------------
void Function::addParameter(ShaderParameterList& parameterList, Parameter* parameter)
										
{
	// Check that parameter with the same name doest exist in input parameters list.
	if (getParameterByName(mInputParameters, parameter->getName()) != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Parameter <" + parameter->getName() + "> already declared in function <" + getName() + ">", 			
			"ShaderFunction::addParameter" );
	}

	// Check that parameter with the same name doest exist in output parameters list.
	if (getParameterByName(mOutputParameters, parameter->getName()) != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Parameter <" + parameter->getName() + "> already declared in function <" + getName() + ">", 			
			"ShaderFunction::addParameter" );
	}


	// Add to given parameters list.
	parameterList.push_back(parameter);
}

//-----------------------------------------------------------------------------
void Function::deleteParameter(ShaderParameterList& parameterList, Parameter* parameter)
{
	ShaderParameterIterator it;

	for (it = parameterList.begin(); it != parameterList.end(); ++it)
	{
		if (*it == parameter)
		{
			delete *it;
			parameterList.erase(it);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
Parameter* Function::getParameterByName( const ShaderParameterList& parameterList, const String& name )
{
	ShaderParameterConstIterator it;

	for (it = parameterList.begin(); it != parameterList.end(); ++it)
	{
		if ((*it)->getName() == name)
		{
			return *it;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
Parameter* Function::getParameterBySemantic(const ShaderParameterList& parameterList, 
												const Parameter::Semantic semantic, 
												int index)
{
	ShaderParameterConstIterator it;

	for (it = parameterList.begin(); it != parameterList.end(); ++it)
	{
		if ((*it)->getSemantic() == semantic &&
			(*it)->getIndex() == index)
		{
			return *it;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void Function::addAtomInstace(FunctionAtom* atomInstance)
{
	mAtomInstances.push_back(atomInstance);
}

//-----------------------------------------------------------------------------
bool Function::deleteAtomInstance(FunctionAtom* atomInstance)
{
	FunctionAtomInstanceIterator it;

	for (it=mAtomInstances.begin(); it != mAtomInstances.end(); ++it)
	{
		if (*it == atomInstance)
		{
			delete atomInstance;
			mAtomInstances.erase(it);
			return true;
		}		
	}

	return false;
	
}

//-----------------------------------------------------------------------------
void Function::sortAtomInstances()
{
	if (mAtomInstances.size() > 1)	
		qsort(&mAtomInstances[0], mAtomInstances.size(), sizeof(FunctionAtom*), sAtomInstanceCompare);		
}

//-----------------------------------------------------------------------------
int Function::sAtomInstanceCompare(const void* p0, const void* p1)
{
	FunctionAtom* pInstance0 = *((FunctionAtom**)p0);
	FunctionAtom* pInstance1 = *((FunctionAtom**)p1);

	if (pInstance0->getGroupExecutionOrder() != pInstance1->getGroupExecutionOrder())
	{
		return pInstance0->getGroupExecutionOrder() - pInstance1->getGroupExecutionOrder();
	}
	 
	return pInstance0->getInternalExecutionOrder() - pInstance1->getInternalExecutionOrder();	
}


}
}