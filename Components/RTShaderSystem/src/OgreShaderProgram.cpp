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

#include "OgreShaderProgram.h"
#include "OgreStringConverter.h"

namespace Ogre {
namespace RTShader {

//-----------------------------------------------------------------------------
Program::Program(const String& name, const String& desc, GpuProgramType type)
{
	mName				= name;
	mDescription		= desc;
	mType				= type;
	mEntryPointFunction = NULL;
}

//-----------------------------------------------------------------------------
Program::~Program()
{
	destroyParameters();

	destroyFunctions();
}

//-----------------------------------------------------------------------------
void Program::destroyParameters()
{
	ShaderParameterIterator it;

	for (it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		if (*it != NULL)
		{
			delete *it;
		}	
	}
	mParameters.clear();
}

//-----------------------------------------------------------------------------
void Program::destroyFunctions()
{
	ShaderFunctionIterator it;

	for (it = mFunctions.begin(); it != mFunctions.end(); ++it)
	{
		if (*it != NULL)
		{
			delete *it;
		}	
	}
	mFunctions.clear();
}

//-----------------------------------------------------------------------------
const String& Program::getName() const
{
	return mName;
}

//-----------------------------------------------------------------------------
const String& Program::getDescription() const
{
	return mDescription;
}

//-----------------------------------------------------------------------------
GpuProgramType Program::getType() const
{
	return mType;
}

//-----------------------------------------------------------------------------
void Program::addParameter(Parameter* parameter)
{
	if (getParameterByName(parameter->getName()) != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Parameter <" + parameter->getName() + "> already declared in program <" + getName() +">", 
			"Program::addParameter" );
	}

	mParameters.push_back(parameter);
}

//-----------------------------------------------------------------------------
void Program::removeParameter(Parameter* parameter)
{
	ShaderParameterIterator it;

	for (it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		if ((*it) == parameter)
		{
			delete *it;
			mParameters.erase(it);
		}
	}
}

//-----------------------------------------------------------------------------
Parameter* Program::resolveAutoParameterReal(GpuProgramParameters::AutoConstantType autoType, 
										   Real data)
{
	Parameter* param = NULL;

	// Check if parameter already exists.
	param = getParameterByAutoType(autoType);
	if (param != NULL)
	{
		if (param->isAutoConstantRealParameter() &&
			param->getAutoConstantRealData() == data)
		{
			return param;
		}
	}
	
	// Create new parameter.
	param = new Parameter(autoType, data);
	addParameter(param);

	return param;
}

//-----------------------------------------------------------------------------
Parameter* Program::resolveAutoParameterInt(GpuProgramParameters::AutoConstantType autoType, 
										   size_t data)
{
	Parameter* param = NULL;

	// Check if parameter already exists.
	param = getParameterByAutoType(autoType);
	if (param != NULL)
	{
		if (param->isAutoConstantIntParameter() &&
			param->getAutoConstantIntData() == data)
		{
			return param;
		}
	}

	// Create new parameter.
	param = new Parameter(autoType, data);
	addParameter(param);

	return param;
}

//-----------------------------------------------------------------------------
Parameter* Program::resolveParameter(GpuConstantType type, 
									int index, uint16 variability,
									const String& suggestedName)
{
	Parameter* param = NULL;

	if (index == -1)
	{
		index = 0;

		// Find the next available index of the target type.
		ShaderParameterIterator it;

		for (it = mParameters.begin(); it != mParameters.end(); ++it)
		{
			if ((*it)->getType() == type &&
				(*it)->isAutoConstantParameter() == false)
			{
				index++;
			}
		}
	}
	else
	{
		// Check if parameter already exists.
		param = getParameterByType(type, index);
		if (param != NULL)
		{		
			return param;		
		}
	}
	

	
	// Create new parameter.
	param = new Parameter(type, suggestedName + StringConverter::toString(index), Parameter::SPS_UNKNOWN, index, variability);
	addParameter(param);

	return param;
}



//-----------------------------------------------------------------------------
Parameter* Program::getParameterByName(const String& name)
{
	ShaderParameterIterator it;

	for (it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		if ((*it)->getName() == name)
		{
			return *it;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
Parameter* Program::getParameterByType(GpuConstantType type, int index)
{
	ShaderParameterIterator it;

	for (it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		if ((*it)->getType() == type &&
			(*it)->getIndex() == index)
		{
			return *it;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
Parameter* Program::getParameterByAutoType(GpuProgramParameters::AutoConstantType autoType)
{
	ShaderParameterIterator it;

	for (it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		if ((*it)->isAutoConstantParameter() && (*it)->getAutoConstantType() == autoType)
		{
			return *it;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
Function* Program::createFunction(const String& name, const String& desc)
{
	Function* shaderFunction;

	shaderFunction = getFunctionByName(name);
	if (shaderFunction != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Function " + name + " already declared in program " + getName(), 
			"Program::createFunction" );
	}

	shaderFunction = new Function(name, desc);
	mFunctions.push_back(shaderFunction);

	return shaderFunction;
}

//-----------------------------------------------------------------------------
Function* Program::getFunctionByName(const String& name)
{
	ShaderFunctionIterator it;

	for (it = mFunctions.begin(); it != mFunctions.end(); ++it)
	{
		if ((*it)->getName() == name)
		{
			return *it;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void Program::addDependency(const String& libFileName)
{
	for (unsigned int i=0; i < mDependencies.size(); ++i)
	{
		if (mDependencies[i] == libFileName)
		{
			return;
		}
	}
	mDependencies.push_back(libFileName);
}

//-----------------------------------------------------------------------------
size_t Program::getDependencyCount() const
{
	return mDependencies.size();
}

//-----------------------------------------------------------------------------
const String& Program::getDependency(unsigned int index) const
{
	return mDependencies[index];
}

}
}