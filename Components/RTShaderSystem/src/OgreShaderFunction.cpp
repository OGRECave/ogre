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
#include <algorithm> // for std::sort
#include "OgreShaderFunction.h"
#include "OgreStringConverter.h"

namespace Ogre {
    namespace RTShader {
        //-----------------------------------------------------------------------------
        Function::Function(const String& name, const String& desc, const FunctionType functionType)
        {
            mName = name;
            mDescription = desc;
            mFunctionType = functionType;
        }

        //-----------------------------------------------------------------------------
        Function::~Function()
        {
            for (FunctionAtomInstanceIterator it = mAtomInstances.begin(); it != mAtomInstances.end(); ++it)
                OGRE_DELETE(*it);
            mAtomInstances.clear();

            for (ShaderParameterIterator it = mInputParameters.begin(); it != mInputParameters.end(); ++it)
                (*it).setNull();
            mInputParameters.clear();

            for (ShaderParameterIterator it = mOutputParameters.begin(); it != mOutputParameters.end(); ++it)
                (*it).setNull();
            mOutputParameters.clear();

            for (ShaderParameterIterator it = mLocalParameters.begin(); it != mLocalParameters.end(); ++it)
                (*it).setNull();
            mLocalParameters.clear();

        }

        //-----------------------------------------------------------------------------
        ParameterPtr Function::resolveInputParameter(Parameter::Semantic semantic,
            int index,
            const Parameter::Content content,
            GpuConstantType type)
        {
            return resolveParameter(semantic, index, content, type, Parameter::SPD_IN);
        }
        //-----------------------------------------------------------------------------
        ParameterPtr Function::resolveOutputParameter(Parameter::Semantic semantic,
            int index,
            Parameter::Content content,
            GpuConstantType type)
        {
            return resolveParameter(semantic, index, content, type, Parameter::SPD_OUT);
        }
        Ogre::RTShader::ParameterPtr Function::resolveParameter(Parameter::Semantic semantic, int index, Parameter::Content content, GpuConstantType type, Parameter::Direction direction)
        {
            ShaderParameterList &parametersList = getProgramParameters(direction);
            ParameterPtr param;

            // Check if desired parameter already defined.
            param = getParameterByContent(parametersList, content, type);
            if (param.get() != NULL)
                return param;

            // Case we have to create new parameter.
            if (index == -1)
            {
                index = 0;

                // Find the next available index of the target semantic.
                ShaderParameterIterator it;

                for (it = parametersList.begin(); it != parametersList.end(); ++it)
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
                param = getParameterBySemantic(parametersList, semantic, index);
                if (param.get() != NULL && param->getContent() == content)
                {
                    if (param->getType() == type)
                    {
                        return param;
                    }
                    else
                    {
                        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Can not resolve parameter - semantic: " + StringConverter::toString(semantic) + " - index: " + StringConverter::toString(index) + " due to type mismatch. Function <" + getName() + ">",
                            "Function::resolveOutputParameter");
                    }
                }
            }


            // No parameter found -> create new one.
            switch (semantic)
            {
            case Parameter::SPS_POSITION:
                assert(type == GCT_FLOAT4);
                param = ParameterFactory::createPosition(index, direction);
                break;

            case Parameter::SPS_BLEND_WEIGHTS:
                assert(type == GCT_FLOAT4);
                param = ParameterFactory::createWeights(index);
                break;
            case Parameter::SPS_BLEND_INDICES:
                assert(type == GCT_UINT4);
                param = ParameterFactory::createIndices(index);
                break;

            case Parameter::SPS_NORMAL:
                assert(type == GCT_FLOAT3);
                param = ParameterFactory::createNormal(index);
                break;

            case Parameter::SPS_COLOR:
                assert(type == GCT_FLOAT4);
                param = ParameterFactory::createColor(index);
                break;

            case Parameter::SPS_TEXTURE_COORDINATES:
                param = ParameterFactory::createTexcoord(type, index, content);
                break;

            case Parameter::SPS_BINORMAL:
                assert(type == GCT_FLOAT3);
                param = ParameterFactory::createBiNormal(index);
                break;

            case Parameter::SPS_TANGENT:
                assert(type == GCT_FLOAT3);
                param = ParameterFactory::createTangent(index);
                break;
            case Parameter::SPS_UNKNOWN:
                break;

            }

            if (param.get() != NULL)
            {
                addParameter(parametersList, param);
                param->setDirection(direction);
            }

            return param;
        }

        //-----------------------------------------------------------------------------
        ParameterPtr Function::resolveLocalParameter(Parameter::Semantic semantic, int index,
            const String& name,
            GpuConstantType type)
        {
            ParameterPtr param;

            param = getParameterByName(mLocalParameters, name);
            if (param.get() != NULL)
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
                        "Can not resolve local parameter due to type mismatch. Function <" + getName() + ">",
                        "Function::resolveLocalParameter");
                }
            }

            param = ParameterPtr(OGRE_NEW Parameter(type, name, semantic, index, Parameter::SPC_UNKNOWN));
            addParameter(mLocalParameters, param);

            return param;
        }

        //-----------------------------------------------------------------------------
        ParameterPtr Function::resolveLocalParameter(Parameter::Semantic semantic, int index,
            const Parameter::Content content,
            GpuConstantType type)
        {
            ParameterPtr param;

            param = getParameterByContent(mLocalParameters, content, type);
            if (param.get() != NULL)
                return param;

            param = ParameterPtr(OGRE_NEW Parameter(type, "lLocalParam_" + StringConverter::toString(mLocalParameters.size()), semantic, index, content));
            addParameter(mLocalParameters, param);

            return param;
        }

        //-----------------------------------------------------------------------------
        void Function::addInputParameter(ParameterPtr parameter)
        {

            // Check that parameter with the same semantic and index in input parameters list.
            if (getParameterBySemantic(mInputParameters, parameter->getSemantic(), parameter->getIndex()).get() != NULL)
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                    "Parameter <" + parameter->getName() + "> has equal sematic parameter in function <" + getName() + ">",
                    "Function::addInputParameter");
            }

            addParameter(mInputParameters, parameter);
        }

        //-----------------------------------------------------------------------------
        void Function::addOutputParameter(ParameterPtr parameter)
        {
            // Check that parameter with the same semantic and index in output parameters list.
            if (getParameterBySemantic(mOutputParameters, parameter->getSemantic(), parameter->getIndex()).get() != NULL)
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                    "Parameter <" + parameter->getName() + "> has equal sematic parameter in function <" + getName() + ">",
                    "Function::addOutputParameter");
            }

            addParameter(mOutputParameters, parameter);
        }

        //-----------------------------------------------------------------------------
        void Function::deleteInputParameter(ParameterPtr parameter)
        {
            deleteParameter(mInputParameters, parameter);
        }

        //-----------------------------------------------------------------------------
        void Function::deleteOutputParameter(ParameterPtr parameter)
        {
            deleteParameter(mOutputParameters, parameter);
        }

        //-----------------------------------------------------------------------------
        void Function::deleteAllInputParameters()
        {
            mInputParameters.clear();
        }
        //-----------------------------------------------------------------------------
        void Function::addParameter(ShaderParameterList& parameterList, ParameterPtr parameter)
        {
            switch (parameter->getDirection())
            {
            case Parameter::SPD_IN:
                // Check that parameter with the same name doest exist in input parameters list.
                if (getParameterByName(mInputParameters, parameter->getName()).get() != NULL)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Parameter <" + parameter->getName() + "> already declared in function <" + getName() + ">",
                        "Function::addParameter");
                }
                break;
            case Parameter::SPD_OUT:
                // Check that parameter with the same name doest exist in output parameters list.
                if (getParameterByName(mOutputParameters, parameter->getName()).get() != NULL)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Parameter <" + parameter->getName() + "> already declared in function <" + getName() + ">",
                        "Function::addParameter");
                }

                break;
            }
            // Add to given parameters list.
            parameterList.push_back(parameter);
        }

        //-----------------------------------------------------------------------------
        void Function::deleteParameter(ShaderParameterList& parameterList, ParameterPtr parameter)
        {
            ShaderParameterIterator it;

            for (it = parameterList.begin(); it != parameterList.end(); ++it)
            {
                if (*it == parameter)
                {
                    (*it).setNull();
                    parameterList.erase(it);
                    break;
                }
            }
        }

        //-----------------------------------------------------------------------------
        ParameterPtr Function::getParameterByName(const ShaderParameterList& parameterList, const String& name)
        {
            ShaderParameterConstIterator it;

            for (it = parameterList.begin(); it != parameterList.end(); ++it)
            {
                if ((*it)->getName() == name)
                {
                    return *it;
                }
            }

            return ParameterPtr();
        }

        //-----------------------------------------------------------------------------
        ParameterPtr Function::getParameterBySemantic(const ShaderParameterList& parameterList,
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

            return ParameterPtr();
        }

        //-----------------------------------------------------------------------------
        ParameterPtr Function::getParameterByContent(const ShaderParameterList& parameterList, const Parameter::Content content, GpuConstantType type)
        {
            ShaderParameterConstIterator it;

            // Search only for known content.
            if (content != Parameter::SPC_UNKNOWN)
            {
                for (it = parameterList.begin(); it != parameterList.end(); ++it)
                {
                    if ((*it)->getContent() == content &&
                        (*it)->getType() == type)
                    {
                        return *it;
                    }
                }
            }

            return ParameterPtr();
        }


        //-----------------------------------------------------------------------------
        void Function::addAtomInstance(FunctionAtom* atomInstance)
        {
            mAtomInstances.push_back(atomInstance);
        }

        //-----------------------------------------------------------------------------
        bool Function::deleteAtomInstance(FunctionAtom* atomInstance)
        {
            FunctionAtomInstanceIterator it;

            for (it = mAtomInstances.begin(); it != mAtomInstances.end(); ++it)
            {
                if (*it == atomInstance)
                {
                    OGRE_DELETE atomInstance;
                    mAtomInstances.erase(it);
                    return true;
                }
            }

            return false;

        }

        namespace {
            struct CmpAtomInstance {
                bool operator()(const FunctionAtom* a, const FunctionAtom* b) const
                {
                    if (a->getGroupExecutionOrder() < b->getGroupExecutionOrder())
                        return true;
                    if (b->getGroupExecutionOrder() < a->getGroupExecutionOrder())
                        return false;
                    return a->getInternalExecutionOrder() < b->getInternalExecutionOrder();
                }
            };
        }

        //-----------------------------------------------------------------------------
        void Function::sortAtomInstances()
        {
            std::sort(mAtomInstances.begin(), mAtomInstances.end(), CmpAtomInstance());
        }

        //-----------------------------------------------------------------------------
        Ogre::RTShader::Function::FunctionType Function::getFunctionType() const
        {
            return mFunctionType;
        }
        //-----------------------------------------------------------------------------
        void Function::synchronizeInputParamsTo(Function* outputFunction)
        {
            const ShaderParameterList InputFunctionOriginalInParams = mInputParameters;
            ShaderParameterList parameters;
            deleteAllInputParameters();

            // Loop the output function output parameters and make sure that
            //   all of them exist in the current function input parameters.
            ShaderParameterConstIterator it;

            if (outputFunction != NULL)
            {
                const ShaderParameterList& outParams = outputFunction->getOutputParameters();
                for (it = outParams.begin(); it != outParams.end(); ++it)
                {
                    ParameterPtr curOutParemter = *it;

                    ParameterPtr paramToAdd = Function::getParameterBySemantic(
                        InputFunctionOriginalInParams,
                        curOutParemter->getSemantic(),
                        curOutParemter->getIndex());

                    if (paramToAdd.isNull())
                    {
                        // param not found - we will add the one from the vertex shader
                        paramToAdd = curOutParemter;
                        paramToAdd = paramToAdd->clone();
                    }

                    paramToAdd->setDirection(Parameter::SPD_IN);

                    addInputParameter(paramToAdd);
                }
            }
        }
        //-----------------------------------------------------------------------------
        ShaderParameterList& Function::getProgramParameters(Parameter::Direction direction)
        {
            switch (direction)
            {
            case Parameter::SPD_IN:
                return mInputParameters;
            case Parameter::SPD_OUT:
                return mOutputParameters;
                break;
            default:
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                    "Requested parmeters should be in or out",
                    "Function::getProgramParameters");
                break;
            }


        }
    }
}