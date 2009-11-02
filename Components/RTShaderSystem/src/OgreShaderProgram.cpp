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
Program::Program(GpuProgramType type)
{
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
			OGRE_DELETE *it;
		}	
	}
	mFunctions.clear();
}

//-----------------------------------------------------------------------------
GpuProgramType Program::getType() const
{
	return mType;
}

//-----------------------------------------------------------------------------
void Program::addParameter(ParameterPtr parameter)
{
	if (getParameterByName(parameter->getName()).get() != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Parameter <" + parameter->getName() + "> already declared in program.", 
			"Program::addParameter" );
	}

	mParameters.push_back(parameter);
}

//-----------------------------------------------------------------------------
void Program::removeParameter(ParameterPtr parameter)
{
	ShaderParameterIterator it;

	for (it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		if ((*it) == parameter)
		{
			(*it).setNull();
			mParameters.erase(it);
		}
	}
}

//-----------------------------------------------------------------------------
ParameterPtr Program::resolveAutoParameterReal(GpuProgramParameters::AutoConstantType autoType, 
												Real data)
{
	ParameterPtr param;

	// Check if parameter already exists.
	param = getParameterByAutoType(autoType);
	if (param.get() != NULL)
	{
		if (param->isAutoConstantRealParameter() &&
			param->getAutoConstantRealData() == data)
		{
			return param;
		}
	}
	
	// Create new parameter.
	param = ParameterPtr(OGRE_NEW Parameter(autoType, data));
	addParameter(param);

	return param;
}

//-----------------------------------------------------------------------------
ParameterPtr Program::resolveAutoParameterInt(GpuProgramParameters::AutoConstantType autoType, 
										   size_t data)
{
	ParameterPtr param;

	// Check if parameter already exists.
	param = getParameterByAutoType(autoType);
	if (param.get() != NULL)
	{
		if (param->isAutoConstantIntParameter() &&
			param->getAutoConstantIntData() == data)
		{
			return param;
		}
	}

	// Create new parameter.
	param = ParameterPtr(OGRE_NEW Parameter(autoType, data));
	addParameter(param);

	return param;
}

//-----------------------------------------------------------------------------
ParameterPtr Program::resolveParameter(GpuConstantType type, 
									int index, uint16 variability,
									const String& suggestedName)
{
	ParameterPtr param;

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
		if (param.get() != NULL)
		{		
			return param;		
		}
	}
	

	
	// Create new parameter.
	param = ParameterPtr(OGRE_NEW Parameter(type, suggestedName + StringConverter::toString(index), Parameter::SPS_UNKNOWN, index, Parameter::SPC_UNKNOWN, variability));
	addParameter(param);

	return param;
}



//-----------------------------------------------------------------------------
ParameterPtr Program::getParameterByName(const String& name)
{
	ShaderParameterIterator it;

	for (it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		if ((*it)->getName() == name)
		{
			return *it;
		}
	}

	return ParameterPtr();
}

//-----------------------------------------------------------------------------
ParameterPtr Program::getParameterByType(GpuConstantType type, int index)
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

	return ParameterPtr();
}

//-----------------------------------------------------------------------------
ParameterPtr Program::getParameterByAutoType(GpuProgramParameters::AutoConstantType autoType)
{
	ShaderParameterIterator it;

	for (it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		if ((*it)->isAutoConstantParameter() && (*it)->getAutoConstantType() == autoType)
		{
			return *it;
		}
	}

	return ParameterPtr();
}

//-----------------------------------------------------------------------------
Function* Program::createFunction(const String& name, const String& desc)
{
	Function* shaderFunction;

	shaderFunction = getFunctionByName(name);
	if (shaderFunction != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Function " + name + " already declared in program.", 
			"Program::createFunction" );
	}

	shaderFunction = OGRE_NEW Function(name, desc);
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