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

#include "OgreShaderProgram.h"

namespace Ogre {
namespace RTShader {

//-----------------------------------------------------------------------------
Program::Program(GpuProgramType type)
{
    mType               = type;
    mEntryPointFunction = NULL;
    mSkeletalAnimation  = false;
    mColumnMajorMatrices = true;
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
        OGRE_DELETE *it;
    }
    mFunctions.clear();
}

//-----------------------------------------------------------------------------
GpuProgramType Program::getType() const
{
    return mType;
}

//-----------------------------------------------------------------------------
void Program::addParameter(UniformParameterPtr parameter)
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
void Program::removeParameter(UniformParameterPtr parameter)
{
    UniformParameterIterator it;

    for (it = mParameters.begin(); it != mParameters.end(); ++it)
    {
        if ((*it) == parameter)
        {
            (*it).setNull();
            mParameters.erase(it);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
UniformParameterPtr Program::resolveAutoParameterReal(GpuProgramParameters::AutoConstantType autoType, 
                                                Real data, size_t size)
{
    UniformParameterPtr param;

    // Check if parameter already exists.
    param = getParameterByAutoType(autoType);
    if (param.get() != NULL)
    {
        if (param->isAutoConstantRealParameter() &&
            param->getAutoConstantRealData() == data)
        {
            param->setSize(std::max(size, param->getSize()));
            return param;
        }
    }
    
    // Create new parameter.
    param = UniformParameterPtr(OGRE_NEW UniformParameter(autoType, data, size));
    addParameter(param);

    return param;
}

//-----------------------------------------------------------------------------
UniformParameterPtr Program::resolveAutoParameterReal(GpuProgramParameters::AutoConstantType autoType, GpuConstantType type,
                                                Real data, size_t size)
{
    UniformParameterPtr param;

    // Check if parameter already exists.
    param = getParameterByAutoType(autoType);
    if (param.get() != NULL)
    {
        if (param->isAutoConstantRealParameter() &&
            param->getAutoConstantRealData() == data)
        {
            param->setSize(std::max(size, param->getSize()));
            return param;
        }
    }
    
    // Create new parameter.
    param = UniformParameterPtr(OGRE_NEW UniformParameter(autoType, data, size, type));
    addParameter(param);

    return param;
}

//-----------------------------------------------------------------------------
UniformParameterPtr Program::resolveAutoParameterInt(GpuProgramParameters::AutoConstantType autoType,
                                           size_t data, size_t size)
{
    UniformParameterPtr param;

    // Check if parameter already exists.
    param = getParameterByAutoType(autoType);
    if (param.get() != NULL)
    {
        if (param->isAutoConstantIntParameter() &&
            param->getAutoConstantIntData() == data)
        {
            param->setSize(std::max(size, param->getSize()));
            return param;
        }
    }

    // Create new parameter.
    param = UniformParameterPtr(OGRE_NEW UniformParameter(autoType, data, size));
    addParameter(param);

    return param;
}

//-----------------------------------------------------------------------------
UniformParameterPtr Program::resolveAutoParameterInt(GpuProgramParameters::AutoConstantType autoType, GpuConstantType type, 
                                           size_t data, size_t size)
{
    UniformParameterPtr param;

    // Check if parameter already exists.
    param = getParameterByAutoType(autoType);
    if (param.get() != NULL)
    {
        if (param->isAutoConstantIntParameter() &&
            param->getAutoConstantIntData() == data)
        {
            param->setSize(std::max(size, param->getSize()));
            return param;
        }
    }

    // Create new parameter.
    param = UniformParameterPtr(OGRE_NEW UniformParameter(autoType, data, size, type));
    addParameter(param);

    return param;
}

//-----------------------------------------------------------------------------
UniformParameterPtr Program::resolveParameter(GpuConstantType type, 
                                    int index, uint16 variability,
                                    const String& suggestedName,
                                    size_t size)
{
    UniformParameterPtr param;

    if (index == -1)
    {
        index = 0;

        // Find the next available index of the target type.
        UniformParameterIterator it;

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
    param = ParameterFactory::createUniform(type, index, variability, suggestedName, size);
    addParameter(param);

    return param;
}



//-----------------------------------------------------------------------------
UniformParameterPtr Program::getParameterByName(const String& name)
{
    UniformParameterIterator it;

    for (it = mParameters.begin(); it != mParameters.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            return *it;
        }
    }

    return UniformParameterPtr();
}

//-----------------------------------------------------------------------------
UniformParameterPtr Program::getParameterByType(GpuConstantType type, int index)
{
    UniformParameterIterator it;

    for (it = mParameters.begin(); it != mParameters.end(); ++it)
    {
        if ((*it)->getType() == type &&
            (*it)->getIndex() == index)
        {
            return *it;
        }
    }

    return UniformParameterPtr();
}

//-----------------------------------------------------------------------------
UniformParameterPtr Program::getParameterByAutoType(GpuProgramParameters::AutoConstantType autoType)
{
    UniformParameterIterator it;

    for (it = mParameters.begin(); it != mParameters.end(); ++it)
    {
        if ((*it)->isAutoConstantParameter() && (*it)->getAutoConstantType() == autoType)
        {
            return *it;
        }
    }

    return UniformParameterPtr();
}

//-----------------------------------------------------------------------------
Function* Program::createFunction(const String& name, const String& desc, const Function::FunctionType functionType)
{
    Function* shaderFunction;

    shaderFunction = getFunctionByName(name);
    if (shaderFunction != NULL)
    {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
            "Function " + name + " already declared in program.", 
            "Program::createFunction" );
    }

    shaderFunction = OGRE_NEW Function(name, desc, functionType);
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
