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
	mSubRenderStateSortValid = false;
	mHashCodeValid			 = false;
	mHashCode				 = 0;
	mLightCountAutoUpdate	 = true;
}

//-----------------------------------------------------------------------
RenderState::~RenderState()
{
	reset();
}

//-----------------------------------------------------------------------
void RenderState::reset()
{
	for (SubRenderStateIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
	{
		ShaderGenerator::getSingleton().destroySubRenderState(*it);
	}
	mSubRenderStateList.clear();
}

//-----------------------------------------------------------------------
void RenderState::addSubRenderState(SubRenderState* subRenderState)
{
	mSubRenderStateList.push_back(subRenderState);
	mSubRenderStateSortValid = false;
	mHashCodeValid			 = false;
}

//-----------------------------------------------------------------------
void RenderState::removeSubRenderState(SubRenderState* subRenderState)
{
	for (SubRenderStateIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
	{
		if ((*it) == subRenderState)
		{
			ShaderGenerator::getSingleton().destroySubRenderState(*it);
			break;
		}		
	}
}

//-----------------------------------------------------------------------
void RenderState::merge(const RenderState& rhs, Pass* srcPass, Pass* dstPass)
{	
	SubRenderStateList customSubRenderStates;

	// Sort current render states.
	sortSubRenderStates();

	// Insert all custom sub render states. (I.E Not FFP sub render states).
	for (SubRenderStateConstIterator itSrc=rhs.mSubRenderStateList.begin(); itSrc != rhs.mSubRenderStateList.end(); ++itSrc)
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
			for (SubRenderStateConstIterator itDst=mSubRenderStateList.begin(); itDst != mSubRenderStateList.end(); ++itDst)
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
	for (SubRenderStateIterator itSrc=customSubRenderStates.begin(); itSrc != customSubRenderStates.end(); ++itSrc)
	{
		SubRenderState* customSubRenderState = *itSrc;

		if (customSubRenderState->preAddToRenderState(this, srcPass, dstPass))
		{
			addSubRenderState(customSubRenderState);
		}
		else
		{
			ShaderGenerator::getSingleton().destroySubRenderState(customSubRenderState);
		}		
	}	
}

//-----------------------------------------------------------------------
void RenderState::copyFrom(const RenderState& rhs)
{
	// Avoid copying on self.
	if (this == &rhs)
		return;

	// Reset state.
	reset();

	// Copy sub render states.
	for (SubRenderStateConstIterator it=rhs.mSubRenderStateList.begin(); it != rhs.mSubRenderStateList.end(); ++it)
	{
		const SubRenderState* srcSubRenderState = *it;
		SubRenderState* dstSubRenderState = ShaderGenerator::getSingleton().createSubRenderState(srcSubRenderState->getType());

		*dstSubRenderState = *srcSubRenderState;
		addSubRenderState(dstSubRenderState);
	}

	mSubRenderStateSortValid = rhs.mSubRenderStateSortValid;

	mLightCount[0] = rhs.mLightCount[0];
	mLightCount[1] = rhs.mLightCount[1];
	mLightCount[2] = rhs.mLightCount[2];
	mLightCountAutoUpdate = rhs.mLightCountAutoUpdate;

	mHashCode		= rhs.mHashCode;
	mHashCodeValid	= rhs.mHashCodeValid;
}

//-----------------------------------------------------------------------
RenderState& RenderState::operator=(const RenderState& rhs)
{
	copyFrom(rhs);

	return *this;
}

//-----------------------------------------------------------------------
uint32 RenderState::getHashCode()
{	
	if (mHashCodeValid == false)
	{		
		sortSubRenderStates();

		mHashCode = 0;

		for (SubRenderStateIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
		{
			SubRenderState* srcSubRenderState = *it;
			uint32 subRenderStateHashCode = srcSubRenderState->getHashCode();

			sh_hash_combine(mHashCode, subRenderStateHashCode);			
		}

		mHashCodeValid = true;
	}
	
	return mHashCode;
}
	
//-----------------------------------------------------------------------
void RenderState::sortSubRenderStates()
{
	if (mSubRenderStateSortValid == false)
	{
		if (mSubRenderStateList.size() > 1)
			qsort(&mSubRenderStateList[0], mSubRenderStateList.size(), sizeof(SubRenderState*), sSubRenderStateCompare);		

		mSubRenderStateSortValid = true;
	}
}

//-----------------------------------------------------------------------
int	RenderState::sSubRenderStateCompare(const void * p0, const void *p1)
{
	SubRenderState* pInstance0 = *((SubRenderState**)p0);
	SubRenderState* pInstance1 = *((SubRenderState**)p1);

	return pInstance0->getExecutionOrder() - pInstance1->getExecutionOrder();	
}

//-----------------------------------------------------------------------
bool RenderState::createCpuPrograms(ProgramSet* programSet)
{
	sortSubRenderStates();

	const String baseName = StringConverter::toString(getHashCode());
	Program* vsProgram = ProgramManager::getSingleton().createCpuProgram(baseName + "_VS", "Vertex Shader Program", GPT_VERTEX_PROGRAM);
	Program* psProgram = ProgramManager::getSingleton().createCpuProgram(baseName + "_PS", "Fragment Shader Program", GPT_FRAGMENT_PROGRAM);
	RTShader::Function* vsMainFunc = NULL;


	programSet->setCpuVertexProgram(vsProgram);
	programSet->setCpuFragmentProgram(psProgram);

	// Create entry point functions.
	vsMainFunc = vsProgram->createFunction("main", "Program Entry point");
	vsProgram->setEntryPointFunction(vsMainFunc);

	vsMainFunc = psProgram->createFunction("main", "Program Entry point");
	psProgram->setEntryPointFunction(vsMainFunc);

	for (SubRenderStateIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
	{
		SubRenderState* srcSubRenderState = *it;

		if (false == srcSubRenderState->createCpuSubPrograms(programSet))
		{
			LogManager::getSingleton().stream()	<< "RTShader::RenderState : Could not generate sub render program of type: " << srcSubRenderState->getType();
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------
void RenderState::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
										  const LightList* pLightList)
{
	for (SubRenderStateIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
	{
		SubRenderState* curSubRenderState = *it;
		
		curSubRenderState->updateGpuProgramsParams(rend, pass, source, pLightList);		
	}
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


}
}

