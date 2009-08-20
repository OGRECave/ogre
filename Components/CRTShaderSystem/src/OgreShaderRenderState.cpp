/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include <boost/functional/hash/hash.hpp>
#include "OgreShaderRenderState.h"
#include "OgreShaderGenerator.h"
#include "OgreLogManager.h"
#include "OgreShaderProgram.h"
#include "OgreShaderProgramset.h"
#include "OgreStringConverter.h"
#include "OgreShaderProgramManager.h"


namespace Ogre {
namespace CRTShader {


//-----------------------------------------------------------------------
RenderState::RenderState()
{
	mSubRenderStateSortValid = false;
	mHashCodeValid			 = false;
	mHashCode				 = 0;
	ShaderGenerator::getSingleton().getMaxLightCount(mMaxLightCount);
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
	subRenderState->setParent(this);
	mSubRenderStateList.push_back(subRenderState);
	mSubRenderStateSortValid = false;
	mHashCodeValid			 = false;
}

//-----------------------------------------------------------------------
void RenderState::append(const RenderState& rhs)
{	
	// Add the source sub render states to current list of sub render states.
	for (SubRenderStateConstIterator itSrc=rhs.mSubRenderStateList.begin(); itSrc != rhs.mSubRenderStateList.end(); ++itSrc)
	{
		const SubRenderState* srcSubRenderState = *itSrc;
		SubRenderState* dstSubRenderState = NULL;


		dstSubRenderState = ShaderGenerator::getSingleton().createSubRenderState(srcSubRenderState->getType());
		*dstSubRenderState = *srcSubRenderState;
		addSubRenderState(dstSubRenderState);			
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

	mMaxLightCount[0] = rhs.mMaxLightCount[0];
	mMaxLightCount[1] = rhs.mMaxLightCount[1];
	mMaxLightCount[2] = rhs.mMaxLightCount[2];
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

			boost::hash_combine(mHashCode, subRenderStateHashCode);			
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
	Program* vsProgram = ProgramManager::getSingleton().createProgram(baseName + "_VS", "Vertex Shader Program", GPT_VERTEX_PROGRAM);
	Program* psProgram = ProgramManager::getSingleton().createProgram(baseName + "_PS", "Fragment Shader Program", GPT_FRAGMENT_PROGRAM);
	CRTShader::Function* vsMainFunc = NULL;


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
			LogManager::getSingleton().stream()	<< "CRTShader::RenderState : Could not generate sub render program of type: " << srcSubRenderState->getType();
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
void RenderState::preFindVisibleObjects(SceneManager* source, 
								   SceneManager::IlluminationRenderStage irs, Viewport* v)
{
	for (SubRenderStateIterator it=mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
	{
		SubRenderState* curSubRenderState = *it;

		curSubRenderState->preFindVisibleObjects(source, irs, v);		
	}
}

//-----------------------------------------------------------------------
void RenderState::setMaxLightCount(const int maxLightCount[3])
{
	mMaxLightCount[0] = maxLightCount[0];
	mMaxLightCount[1] = maxLightCount[1];
	mMaxLightCount[2] = maxLightCount[2];
}

//-----------------------------------------------------------------------
void RenderState::getMaxLightCount(int maxLightCount[3]) const
{
	maxLightCount[0] = mMaxLightCount[0];
	maxLightCount[1] = mMaxLightCount[1];
	maxLightCount[2] = mMaxLightCount[2];
}


}
}

