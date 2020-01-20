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

const char* TargetRenderState::UserKey = "TargetRenderState";

//-----------------------------------------------------------------------
RenderState::RenderState()
{
    mLightCountAutoUpdate    = true;    
    mLightCount[0]           = 0;
    mLightCount[1]           = 0;
    mLightCount[2]           = 0;   
}

//-----------------------------------------------------------------------
RenderState::~RenderState()
{
    reset();
}

//-----------------------------------------------------------------------
void RenderState::reset()
{
    for (SubRenderStateListIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
    {
        ShaderGenerator::getSingleton().destroySubRenderState(*it);
    }
    mSubRenderStateList.clear();
}

//-----------------------------------------------------------------------
void RenderState::setLightCount(const Vector3i& lightCount)
{
    mLightCount = lightCount;
}

//-----------------------------------------------------------------------
const Vector3i& RenderState::getLightCount() const
{
    return mLightCount;
}

//-----------------------------------------------------------------------
void RenderState::addTemplateSubRenderState(SubRenderState* subRenderState)
{
    bool addSubRenderState = true;

    // Go over the current sub render state.
    for (SubRenderStateListIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
    {
        // Case the same instance already exists -> do not add to list.
        if (*it == subRenderState)
        {
            addSubRenderState = false;
            break;
        }

        // Case it is different sub render state instance with the same type, use the new sub render state
        // instead of the previous sub render state. This scenario is usually caused by material inheritance, so we use the derived material sub render state
        // and destroy the base sub render state.
        else if ((*it)->getType() == subRenderState->getType())
        {
            removeTemplateSubRenderState(*it);
            break;
        }
    }

    // Add only if this sub render state instance is not on the list.
    if (addSubRenderState)
    {
        mSubRenderStateList.push_back(subRenderState);  
    }   
}

//-----------------------------------------------------------------------
void RenderState::removeTemplateSubRenderState(SubRenderState* subRenderState)
{
    for (SubRenderStateListIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
    {
        if ((*it) == subRenderState)
        {
            ShaderGenerator::getSingleton().destroySubRenderState(*it);
            mSubRenderStateList.erase(it);
            break;
        }       
    }
}

//-----------------------------------------------------------------------
TargetRenderState::TargetRenderState()
{
    mSubRenderStateSortValid = false;
}

//-----------------------------------------------------------------------
void TargetRenderState::addSubRenderStateInstance(SubRenderState* subRenderState)
{
    mSubRenderStateList.push_back(subRenderState);
    mSubRenderStateSortValid = false;
}

//-----------------------------------------------------------------------
void TargetRenderState::removeSubRenderStateInstance(SubRenderState* subRenderState)
{
    for (SubRenderStateListIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
    {
        if ((*it) == subRenderState)
        {
            ShaderGenerator::getSingleton().destroySubRenderState(*it);
            mSubRenderStateList.erase(it);
            break;
        }       
    }
}

//-----------------------------------------------------------------------------
void TargetRenderState::bindUniformParameters(Program* pCpuProgram, const GpuProgramParametersSharedPtr& passParams)
{
    // samplers are bound via registers in HLSL & Cg
    bool samplersBound = ShaderGenerator::getSingleton().getTargetLanguage()[0] != 'g';

    // Bind each uniform parameter to its GPU parameter.
    for (const auto& param : pCpuProgram->getParameters())
    {
        if((samplersBound && param->isSampler()) || !param->isUsed()) continue;

        param->bind(passParams);
        param->setUsed(false); // reset for shader regen
    }
}

void TargetRenderState::acquirePrograms(Pass* pass)
{
    createCpuPrograms();

    ProgramManager::getSingleton().createGpuPrograms(mProgramSet.get());

    for(auto type : {GPT_VERTEX_PROGRAM, GPT_FRAGMENT_PROGRAM})
    {
        // Bind the created GPU programs to the target pass.
        pass->setGpuProgram(type, mProgramSet->getGpuProgram(type));
        // Bind uniform parameters to pass parameters.
        bindUniformParameters(mProgramSet->getCpuProgram(type), pass->getGpuProgramParameters(type));
    }

    pass->getUserObjectBindings().setUserAny(UserKey, Any(this));
}


void TargetRenderState::releasePrograms(Pass* pass)
{
    if(!mProgramSet)
        return;

    pass->setGpuProgram(GPT_VERTEX_PROGRAM, GpuProgramPtr());
    pass->setGpuProgram(GPT_FRAGMENT_PROGRAM, GpuProgramPtr());

    ProgramManager::getSingleton().releasePrograms(mProgramSet.get());

    mProgramSet.reset();

    pass->getUserObjectBindings().eraseUserAny(UserKey);
}

//-----------------------------------------------------------------------
void TargetRenderState::createCpuPrograms()
{
    sortSubRenderStates();

    ProgramSet* programSet = createProgramSet();
    programSet->setCpuProgram(std::unique_ptr<Program>(new Program(GPT_VERTEX_PROGRAM)));
    programSet->setCpuProgram(std::unique_ptr<Program>(new Program(GPT_FRAGMENT_PROGRAM)));

    for (SubRenderStateListIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
    {
        SubRenderState* srcSubRenderState = *it;

        if (!srcSubRenderState->createCpuSubPrograms(programSet))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Could not generate sub render program of type: " + srcSubRenderState->getType());
        }
    }
}

//-----------------------------------------------------------------------
ProgramSet* TargetRenderState::createProgramSet()
{
    mProgramSet.reset(new ProgramSet);

    return mProgramSet.get();
}

//-----------------------------------------------------------------------
void TargetRenderState::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source,
                                                const LightList* pLightList)
{
    for (SubRenderStateListIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
    {
        SubRenderState* curSubRenderState = *it;

        curSubRenderState->updateGpuProgramsParams(rend, pass, source, pLightList);     
    }
}

//-----------------------------------------------------------------------
void TargetRenderState::link(const RenderState& rhs, Pass* srcPass, Pass* dstPass)
{   
    SubRenderStateList customSubRenderStates;

    // Sort current render states.
    sortSubRenderStates();

    // Insert all custom sub render states. (I.E Not FFP sub render states).
    const SubRenderStateList& subRenderStateList = rhs.getTemplateSubRenderStateList();

    for (SubRenderStateListConstIterator itSrc=subRenderStateList.begin(); itSrc != subRenderStateList.end(); ++itSrc)
    {
        const SubRenderState* srcSubRenderState = *itSrc;
        bool isCustomSubRenderState = true;

        if (srcSubRenderState->getExecutionOrder() == FFP_TRANSFORM ||
            srcSubRenderState->getExecutionOrder() == FFP_COLOUR ||
            srcSubRenderState->getExecutionOrder() == FFP_LIGHTING ||
            srcSubRenderState->getExecutionOrder() == FFP_TEXTURING ||
            srcSubRenderState->getExecutionOrder() == FFP_FOG)
        {
            isCustomSubRenderState = false;
        }       


        // Case it is a custom sub render state.
        if (isCustomSubRenderState)
        {
            bool subStateTypeExists = false;

            // Check if this type of sub render state already exists.
            for (SubRenderStateListConstIterator itDst=mSubRenderStateList.begin(); itDst != mSubRenderStateList.end(); ++itDst)
            {
                if ((*itDst)->getType() == srcSubRenderState->getType())
                {
                    subStateTypeExists = true;
                    break;
                }               
            }

            // Case custom sub render state not exits -> add it to custom list.
            if (subStateTypeExists == false)
            {
                SubRenderState* newSubRenderState = NULL;

                newSubRenderState = ShaderGenerator::getSingleton().createSubRenderState(srcSubRenderState->getType());
                *newSubRenderState = *srcSubRenderState;
                customSubRenderStates.push_back(newSubRenderState);
            }                       
        }                       
    }   

    // Merge the local custom sub render states.
    for (SubRenderStateListIterator itSrc=customSubRenderStates.begin(); itSrc != customSubRenderStates.end(); ++itSrc)
    {
        SubRenderState* customSubRenderState = *itSrc;

        if (customSubRenderState->preAddToRenderState(this, srcPass, dstPass))
        {
            addSubRenderStateInstance(customSubRenderState);
        }
        else
        {
            ShaderGenerator::getSingleton().destroySubRenderState(customSubRenderState);
        }       
    }   
}

namespace {
    struct CmpSubRenderStates {
        bool operator()(const SubRenderState* a, const SubRenderState* b) const
        {
            return a->getExecutionOrder() < b->getExecutionOrder();
        }
    };
}

//-----------------------------------------------------------------------
void TargetRenderState::sortSubRenderStates()
{
    if (mSubRenderStateSortValid == false)
    {
        std::sort(mSubRenderStateList.begin(), mSubRenderStateList.end(), CmpSubRenderStates());
        mSubRenderStateSortValid = true;
    }
}

}
}
