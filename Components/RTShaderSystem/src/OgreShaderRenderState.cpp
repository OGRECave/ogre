/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreShaderPrerequisites.h"
#include "OgreShaderRenderState.h"
#include "OgreShaderGenerator.h"
#include "OgreLogManager.h"
#include "OgreShaderProgram.h"
#include "OgreShaderProgramSet.h"
#include "OgreStringConverter.h"
#include "OgreShaderProgramManager.h"
#include "OgreShaderFFPRenderState.h"


namespace Ogre {
namespace RTShader {


//-----------------------------------------------------------------------
RenderState::RenderState()
{
	mLightCountAutoUpdate	 = true;	
	mLightCount[0]			 = 0;
	mLightCount[1]			 = 0;
	mLightCount[2]			 = 0;	
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
void RenderState::setLightCount(const int lightCount[3])
{
	mLightCount[0] = lightCount[0];
	mLightCount[1] = lightCount[1];
	mLightCount[2] = lightCount[2];
}

//-----------------------------------------------------------------------
void RenderState::getLightCount(int lightCount[3]) const
{
	lightCount[0] = mLightCount[0];
	lightCount[1] = mLightCount[1];
	lightCount[2] = mLightCount[2];
}

//-----------------------------------------------------------------------
void RenderState::addTemplateSubRenderState(SubRenderState* subRenderState)
{
	mSubRenderStateList.push_back(subRenderState);	
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
	mProgramSet				 = NULL;
	mSubRenderStateSortValid = false;	
}

//-----------------------------------------------------------------------
TargetRenderState::~TargetRenderState()
{
	destroyProgramSet();
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

//-----------------------------------------------------------------------
bool TargetRenderState::createCpuPrograms()
{
	sortSubRenderStates();

	ProgramSet* programSet = createProgramSet();
	Program* vsProgram = ProgramManager::getSingleton().createCpuProgram(GPT_VERTEX_PROGRAM);
	Program* psProgram = ProgramManager::getSingleton().createCpuProgram(GPT_FRAGMENT_PROGRAM);
	RTShader::Function* vsMainFunc = NULL;
	RTShader::Function* psMainFunc = NULL;

	programSet->setCpuVertexProgram(vsProgram);
	programSet->setCpuFragmentProgram(psProgram);

	// Create entry point functions.
	vsMainFunc = vsProgram->createFunction("main", "Vertex Program Entry point", Function::FFT_VS_MAIN);
	vsProgram->setEntryPointFunction(vsMainFunc);

	psMainFunc = psProgram->createFunction("main", "Pixel Program Entry point", Function::FFT_PS_MAIN);
	psProgram->setEntryPointFunction(psMainFunc);

	for (SubRenderStateListIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
	{
		SubRenderState* srcSubRenderState = *it;

		if (false == srcSubRenderState->createCpuSubPrograms(programSet))
		{
			LogManager::getSingleton().stream()	<< "RTShader::TargetRenderState : Could not generate sub render program of type: " << srcSubRenderState->getType();
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------
ProgramSet*	TargetRenderState::createProgramSet()
{
	destroyProgramSet();

	mProgramSet = OGRE_NEW ProgramSet;

	return mProgramSet;
}
//-----------------------------------------------------------------------
void TargetRenderState::destroyProgramSet()
{
	if (mProgramSet != NULL)
	{
		OGRE_DELETE mProgramSet;
		mProgramSet = NULL;
	}
}

//-----------------------------------------------------------------------
void TargetRenderState::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
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

//-----------------------------------------------------------------------
void TargetRenderState::sortSubRenderStates()
{
	if (mSubRenderStateSortValid == false)
	{
		if (mSubRenderStateList.size() > 1)
			qsort(&mSubRenderStateList[0], mSubRenderStateList.size(), sizeof(SubRenderState*), sSubRenderStateCompare);		

		mSubRenderStateSortValid = true;
	}
}

//-----------------------------------------------------------------------
int	TargetRenderState::sSubRenderStateCompare(const void * p0, const void *p1)
{
	SubRenderState* pInstance0 = *((SubRenderState**)p0);
	SubRenderState* pInstance1 = *((SubRenderState**)p1);

	return pInstance0->getExecutionOrder() - pInstance1->getExecutionOrder();	
}

}
}

