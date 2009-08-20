/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include <boost\functional\hash\hash.hpp>
#include "OgreShaderGenerator.h"
#include "OgreShaderProgram.h"
#include "OgreShaderProgramManager.h"
#include "OgreShaderFFPRenderStateBuilder.h"
#include "OgreShaderRenderState.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"


namespace Ogre {

//-----------------------------------------------------------------------
template<> 
CRTShader::ShaderGenerator* Singleton<CRTShader::ShaderGenerator>::ms_Singleton = 0;

namespace CRTShader {

//-----------------------------------------------------------------------
ShaderGenerator* ShaderGenerator::getSingletonPtr()
{
	assert( ms_Singleton );  
	return ms_Singleton;
}

//-----------------------------------------------------------------------
ShaderGenerator& ShaderGenerator::getSingleton()
{
	assert( ms_Singleton );  
	return ( *ms_Singleton );
}

//-----------------------------------------------------------------------------
ShaderGenerator::ShaderGenerator()
{
	mShaderLanguage			= "cg";
	mVertexShaderProfiles	= "vs_2_0";
	mFragmentShaderProfiles	= "ps_2_0";
	mProgramManager			= NULL;
	mFFPRenderStateBuilder	= NULL;
	mSceneMgr				= NULL;
	mRenderObjectListener	= NULL;
	mSceneManagerListener	= NULL;
	mActiveViewportValid	= false;
	mMaxLightCount[0]		= 0;
	mMaxLightCount[1]		= 0;
	mMaxLightCount[2]		= 0;
}

//-----------------------------------------------------------------------------
ShaderGenerator::~ShaderGenerator()
{
	
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::initialize(SceneManager* sceneMgr)
{
	if (ms_Singleton == NULL)
	{
		ms_Singleton = new ShaderGenerator;

		if (ms_Singleton->_initialize())
		{
			ms_Singleton->setSceneManager(sceneMgr);
		}
	}
		
	return false;
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::_initialize()
{
	OGRE_LOCK_AUTO_MUTEX

	mProgramManager			= new ProgramManager;
	mFFPRenderStateBuilder	= new FFPRenderStateBuilder;
	if (false == mFFPRenderStateBuilder->initialize())
		return false;

	return true;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::finalize()
{
	if (ms_Singleton != NULL)
	{
		ms_Singleton->_finalize();

		delete ms_Singleton;
		ms_Singleton = NULL;
	}
}

//-----------------------------------------------------------------------------
void ShaderGenerator::_finalize()
{
	OGRE_LOCK_AUTO_MUTEX

	mCachedRenderStates.clear();
	
	// Delete technique entries.
	for (SGTechniqueIterator itTech = mTechniqueEntriesList.begin(); itTech != mTechniqueEntriesList.end(); ++itTech)
	{			
		delete (*itTech);
	}
	mTechniqueEntriesList.clear();

	// Delete material entries.
	for (SGMaterialIterator itMat = mMaterialEntriesMap.begin(); itMat != mMaterialEntriesMap.end(); ++itMat)
	{		
		delete (itMat->second);
	}
	mMaterialEntriesMap.clear();

	// Delete scheme entries.
	for (SGSchemeIterator itScheme = mSchemeEntriesMap.begin(); itScheme != mSchemeEntriesMap.end(); ++itScheme)
	{		
		delete (itScheme->second);
	}
	mSchemeEntriesMap.clear();
	

	// Delete FFP Emulator.
	if (mFFPRenderStateBuilder != NULL)
	{
		mFFPRenderStateBuilder->finalize();
		delete mFFPRenderStateBuilder;
		mFFPRenderStateBuilder = NULL;
	}

	// Delete Program manager.
	if (mProgramManager != NULL)
	{
		delete mProgramManager;
		mProgramManager = NULL;
	}

	setSceneManager(NULL);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::addSubRenderStateFactory(SubRenderStateFactory* factory)
{
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactoryIterator itFind = mSubRenderStateFactoryMap.find(factory->getType());

	if (itFind != mSubRenderStateFactoryMap.end())
	{
		OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
			"A factory of type '" + factory->getType() + "' already exists.",
			"ShaderGenerator::addSubRenderStateFactory");
	}		
	
	mSubRenderStateFactoryMap[factory->getType()] = factory;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::removeSubRenderStateFactory(SubRenderStateFactory* factory)
{
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactoryIterator itFind = mSubRenderStateFactoryMap.find(factory->getType());

	if (itFind != mSubRenderStateFactoryMap.end())
		mSubRenderStateFactoryMap.erase(itFind);

}

//-----------------------------------------------------------------------------
SubRenderState*	ShaderGenerator::createSubRenderState(const String& type)
{
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactoryIterator itFind = mSubRenderStateFactoryMap.find(type);

	if (itFind != mSubRenderStateFactoryMap.end())
		return itFind->second->createInstance();


	OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
		"A factory of type '" + type + "' doesn't exists.",
		"ShaderGenerator::createSubRenderState");

	return NULL;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::destroySubRenderState(SubRenderState* subRenderState)
{
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactoryIterator itFind = mSubRenderStateFactoryMap.find(subRenderState->getType());

	if (itFind != mSubRenderStateFactoryMap.end())
	{
		 itFind->second->destroyInstance(subRenderState);
	}	
}

//-----------------------------------------------------------------------------
RenderState* ShaderGenerator::getRenderState(const String& schemeName)
{
	OGRE_LOCK_AUTO_MUTEX

	SGSchemeIterator itFind = mSchemeEntriesMap.find(schemeName);

	if (itFind == mSchemeEntriesMap.end())
	{
		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
			"A scheme named'" + schemeName + "' doesn't exists.",
			"ShaderGenerator::getRenderState");
	}

	return itFind->second->getRenderState();
}

//-----------------------------------------------------------------------------
void ShaderGenerator::setSceneManager(SceneManager* sceneMgr)
{
	if (mSceneMgr != sceneMgr)
	{
		if (mSceneMgr != NULL)
		{
			mSceneMgr->removeRenderObjectListener(mRenderObjectListener);
			delete mRenderObjectListener;
			mRenderObjectListener = NULL;

			mSceneMgr->removeListener(mSceneManagerListener);
			delete mSceneManagerListener;
			mSceneManagerListener = NULL;
		}

		mSceneMgr = sceneMgr;

		if (mSceneMgr != NULL)
		{
			mRenderObjectListener = new SGRenderObjectListener(this);
			mSceneMgr->addRenderObjectListener(mRenderObjectListener);

			mSceneManagerListener = new SGSceneManagerListener(this);
			mSceneMgr->addListener(mSceneManagerListener);
		}
	}
	
}

//-----------------------------------------------------------------------------
SceneManager* ShaderGenerator::getSceneManager()
{
	return mSceneMgr;
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::createShaderBasedTechnique(const String& materialName, 
												 const String& srcTechniqueSchemeName, 
												 const String& dstTechniqueSchemeName)
{
	OGRE_LOCK_AUTO_MUTEX

	// Make sure material exists;
	if (false == MaterialManager::getSingleton().resourceExists(materialName))
		return false;

	
	SGMaterialIterator itMatEntry = mMaterialEntriesMap.find(materialName);
	
	// Check if technique already created.
	if (itMatEntry != mMaterialEntriesMap.end())
	{
		SGTechniqueList& techniqueEntires = itMatEntry->second->mTechniqueEntires;
		SGTechniqueIterator itTechEntry = techniqueEntires.begin();

		for (; itTechEntry != techniqueEntires.end(); ++itTechEntry)
		{
			// Case the requested mapping already exists.
			if ((*itTechEntry)->getSourceTechnique()->getSchemeName() == srcTechniqueSchemeName &&
				(*itTechEntry)->getDestinationTechnique()->getSchemeName() == dstTechniqueSchemeName)
			{
				return true;
			}


			// Case a shader based technique with the same scheme name already defined based 
			// on different source technique. 
			// This state might lead to conflicts during shader generation - we prevent it by returning false here.
			else if ((*itTechEntry)->getDestinationTechnique()->getSchemeName() == dstTechniqueSchemeName)
			{
				return false;
			}			
		}
	}

	// No technique created -> check if one can be created from the given source technique scheme.	
	Technique* srcTechnique = NULL;

	srcTechnique = findSourceTechnique(materialName, srcTechniqueSchemeName);


	// No appropriate source technique found.
	if (srcTechnique == NULL)
		return false;

	// Create shader based technique from the given source technique.	
	SGMaterial* matEntry = NULL;

	if (itMatEntry == mMaterialEntriesMap.end())
	{
		matEntry = new SGMaterial;
		mMaterialEntriesMap[materialName] = matEntry;
	}
	else
	{
		matEntry = itMatEntry->second;
	}

	// Create the new technique entry.
	SGTechnique* techEntry = new SGTechnique(matEntry, srcTechnique, dstTechniqueSchemeName);
					

	// Add to material entry map.
	matEntry->mTechniqueEntires.push_back(techEntry);

	// Add to all technique list.
	mTechniqueEntriesList.push_back(techEntry);

	// Add to scheme.
	SGSchemeIterator itScheme = mSchemeEntriesMap.find(dstTechniqueSchemeName);
	SGScheme* schemeEntry = NULL;

	if (itScheme == mSchemeEntriesMap.end())
	{
		schemeEntry = new SGScheme;
		mSchemeEntriesMap[dstTechniqueSchemeName] = schemeEntry;
	}
	else
	{
		schemeEntry = itScheme->second;
	}

	schemeEntry->addTechniqueEntry(techEntry);
	

	// Invalidate the destination scheme.
	invalidateScheme(dstTechniqueSchemeName);	
	
	return true;
}
												 
//-----------------------------------------------------------------------------
 Technique* ShaderGenerator::findSourceTechnique(const String& materialName, const String& srcTechniqueSchemeName)
 {
	 MaterialPtr mat = MaterialManager::getSingleton().getByName(materialName);
	 Material::TechniqueIterator itMatTechniques = mat->getSupportedTechniqueIterator();
	 

	 // Find the source technique and make sure it is not programmable.
	 while (itMatTechniques.hasMoreElements())
	 {
		 Technique* curTechnique = itMatTechniques.getNext();

		 if (curTechnique->getSchemeName() == srcTechniqueSchemeName)
		 {
			 for (unsigned short i=0; i < curTechnique->getNumPasses(); ++i)
			 {
				 Pass* curPass = curTechnique->getPass(i);

				 if (curPass->isProgrammable() == true)
				 {
					 return NULL;
				 }				
			 }
			 return curTechnique;				
		 }		
	 }

	 return NULL;
 }

//-----------------------------------------------------------------------------
 void ShaderGenerator::notifyRenderSingleObject(Renderable* rend, 
	 const Pass* pass,  
	 const AutoParamDataSource* source, 
	 const LightList* pLightList, bool suppressRenderStateChanges)
{
	const Any& passUserData = pass->getUserAny();

	if (mActiveViewportValid == false || passUserData.isEmpty())	
		return;	
	
	OGRE_LOCK_AUTO_MUTEX

	SGPass* passEntry = any_cast<SGPass*>(passUserData);

	passEntry->notifyRenderSingleObject(rend, source, pLightList, suppressRenderStateChanges);
}


//-----------------------------------------------------------------------------
void ShaderGenerator::preFindVisibleObjects(SceneManager* source, 
											SceneManager::IlluminationRenderStage irs, 
											Viewport* v)
{
	OGRE_LOCK_AUTO_MUTEX

	const String& curMaterialScheme = v->getMaterialScheme();

	mActiveViewportValid = validateScheme(curMaterialScheme);
}

//-----------------------------------------------------------------------------
RenderStatePtr ShaderGenerator::getCachedRenderState(uint hashCode)
{
	OGRE_LOCK_AUTO_MUTEX

	RenderStateIterator itFind = mCachedRenderStates.find(hashCode);
	RenderStatePtr renderStatePtr;

	if (itFind != mCachedRenderStates.end())
	{	
		renderStatePtr = itFind->second;
	}

	return renderStatePtr;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::addRenderStateToCache(RenderStatePtr renderStatePtr)
{
	OGRE_LOCK_AUTO_MUTEX

	mCachedRenderStates[renderStatePtr->getHashCode()] = renderStatePtr;	
}

//-----------------------------------------------------------------------------
void ShaderGenerator::invalidateScheme(const String& schemeName)
{
	OGRE_LOCK_AUTO_MUTEX

	SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);

	if (itScheme != mSchemeEntriesMap.end())	
		itScheme->second->setValid(false);	
	
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::validateScheme(const String& schemeName)
{
	OGRE_LOCK_AUTO_MUTEX

	SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);

	// No such scheme exists.
	if (itScheme == mSchemeEntriesMap.end())	
		return false;

	itScheme->second->validate();

	return itScheme->second->validated();
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::validateScheme(const String& schemeName, const String& materialName)
{
	OGRE_LOCK_AUTO_MUTEX

	SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);

	// No such scheme exists.
	if (itScheme == mSchemeEntriesMap.end())	
		return false;

	return itScheme->second->validate(materialName);	
}


//-----------------------------------------------------------------------------
void ShaderGenerator::setShaderCachePath( const String& cachePath )
{
	if (mShaderCachePath.length() > 0)	
		ResourceGroupManager::getSingleton().removeResourceLocation(mShaderCachePath);	
	
	mShaderCachePath = cachePath;
	ResourceGroupManager::getSingleton().addResourceLocation(mShaderCachePath, "FileSystem");
}

//-----------------------------------------------------------------------
void ShaderGenerator::setMaxLightCount(const int maxLightCount[3])
{
	mMaxLightCount[0] = maxLightCount[0];
	mMaxLightCount[1] = maxLightCount[1];
	mMaxLightCount[2] = maxLightCount[2];
}

//-----------------------------------------------------------------------
void ShaderGenerator::getMaxLightCount(int maxLightCount[3]) const
{
	maxLightCount[0] = mMaxLightCount[0];
	maxLightCount[1] = mMaxLightCount[1];
	maxLightCount[2] = mMaxLightCount[2];
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGPass::SGPass(SGTechnique* parent, Pass* srcPass, Pass* dstPass)
{
	mParent				= parent;
	mSrcPass			= srcPass;
	mDstPass			= dstPass;	
	mCustomRenderState	= NULL;
	mDstPass->setUserAny(Any(this));
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGPass::~SGPass()
{
	if (mCustomRenderState != NULL)
	{
		delete mCustomRenderState;
		mCustomRenderState = NULL;
	}

	mFinalRenderState.setNull();
}


//-----------------------------------------------------------------------------
void ShaderGenerator::SGPass::buildRenderState()
{	
	const String& schemeName = mParent->getDestinationTechnique()->getSchemeName();
	const RenderState* renderStateGlobal = ShaderGenerator::getSingleton().getRenderState(schemeName);
	RenderState* localRenderState = new RenderState;
	

	// Set light properties.
	int maxLightCount[3] = {0};

	if (mCustomRenderState != NULL)
	{
		mCustomRenderState->getMaxLightCount(maxLightCount);
	}
	else if (renderStateGlobal != NULL)
	{
		renderStateGlobal->getMaxLightCount(maxLightCount);
	}
	
	localRenderState->setMaxLightCount(maxLightCount);
			
	// 1. Build the FFP state.	
	FFPRenderStateBuilder::getSingleton().buildRenderState(this, localRenderState);

	// 2. Append global render state of the shader generator.
	if (renderStateGlobal != NULL)
		localRenderState->append(*renderStateGlobal);

	// 3. Append specific render state of the target technique.
	if (mCustomRenderState != NULL)
		localRenderState->append(*mCustomRenderState);


	// Check if this render state already cached.
	mFinalRenderState = ShaderGenerator::getSingleton().getCachedRenderState(localRenderState->getHashCode());

	// The final pass render state was not cached -> add it.
	if (mFinalRenderState.isNull())
	{
		mFinalRenderState.bind(localRenderState);
		ShaderGenerator::getSingleton().addRenderStateToCache(mFinalRenderState);
	}

	// The final pass render state is already cached -> delete local one.
	else
	{
		delete localRenderState;
	}					
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGPass::acquireGpuPrograms()
{
	ProgramManager::getSingleton().acquireGpuPrograms(mDstPass, mFinalRenderState.get());
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGPass::notifyRenderSingleObject(Renderable* rend,  const AutoParamDataSource* source, 
											  const LightList* pLightList, bool suppressRenderStateChanges)
{
	if (mFinalRenderState.get() != NULL && suppressRenderStateChanges == false)
		mFinalRenderState->updateGpuProgramsParams(rend, mDstPass, source, pLightList);
}
//-----------------------------------------------------------------------------
SubRenderState*	ShaderGenerator::SGPass::getCustomFFPSubState(int subStateOrder)
{
	SubRenderState* customSubState = NULL;

	// Try to override with custom render state of this pass.
	customSubState = getCustomFFPSubState(subStateOrder, mCustomRenderState);
	
	// Case no custom sub state of this pass found, try to override with global scheme state.
	if (customSubState == NULL)
	{
		const String& schemeName = mParent->getDestinationTechnique()->getSchemeName();
		const RenderState* renderStateGlobal = ShaderGenerator::getSingleton().getRenderState(schemeName);

		customSubState = getCustomFFPSubState(subStateOrder, renderStateGlobal);
	}

	return customSubState;
}

//-----------------------------------------------------------------------------
SubRenderState*	ShaderGenerator::SGPass::getCustomFFPSubState(int subStateOrder, const RenderState* renderState)
{
	if (renderState != NULL)
	{
		const SubRenderStateList& subRenderStateList = renderState->getSubStateList();

		for (SubRenderStateConstIterator it=subRenderStateList.begin(); it != subRenderStateList.end(); ++it)
		{
			SubRenderState* curSubRenderState = *it;

			if (curSubRenderState->getExecutionOrder() == subStateOrder)
			{	
				SubRenderState* clone;

				clone = ShaderGenerator::getSingleton().createSubRenderState(curSubRenderState->getType());
				*clone = *curSubRenderState;

				return clone;			
			}
		}
	}
	
	return NULL;
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGTechnique::SGTechnique(SGMaterial* parent, Technique* srcTechnique, const String& dstTechniqueSchemeName)
{
	mParent				= parent;
	mSrcTechnique		= srcTechnique;
	mDstTechnique		= mSrcTechnique->getParent()->createTechnique();
	*mDstTechnique		= *mSrcTechnique;
	mDstTechnique->setSchemeName(dstTechniqueSchemeName);

	// Create pass entry for each pass.
	for (unsigned short i=0; i < mSrcTechnique->getNumPasses(); ++i)
	{
		Pass* srcPass = mSrcTechnique->getPass(i);
		Pass* dstPass = mDstTechnique->getPass(i);

		SGPass* passEntry = new SGPass(this, srcPass, dstPass);				

		mPassEntries.push_back(passEntry);
	}
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGTechnique::~SGTechnique()
{
	for (SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); ++itPass)
	{
		delete (*itPass);
	}
	mPassEntries.clear();
}


//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::buildRenderState()
{
	for (SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); ++itPass)
	{
		(*itPass)->buildRenderState();
	}
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::acquireGpuPrograms()
{
	for (SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); ++itPass)
	{
		(*itPass)->acquireGpuPrograms();
	}
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGScheme::SGScheme()
{
	mIsValid	 = false;
	mRenderState = NULL;
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGScheme::~SGScheme()
{
	if (mRenderState != NULL)
	{
		delete mRenderState;
		mRenderState = NULL;
	}
}

//-----------------------------------------------------------------------------
RenderState* ShaderGenerator::SGScheme::getRenderState()
{
	if (mRenderState == NULL)
		mRenderState = new RenderState;

	return mRenderState;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::addTechniqueEntry(SGTechnique* techEntry)
{
	mTechniqueEntires.push_back(techEntry);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::validate()
{

	// The target scheme is up to date.
	if (validated())
		return;
	
	SGTechniqueIterator itTech;

	// Build render state for each technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		curTechEntry->buildRenderState();		
	}

	// Build render state for each technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		curTechEntry->acquireGpuPrograms();		
	}
	
	// Mark this scheme as valid.
	setValid(true);
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::SGScheme::validate(const String& materialName)
{
	SGTechniqueIterator itTech;

	// Find the desired technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getSourceTechnique()->getParent()->getName() == materialName)
		{
			// Build render state for each technique.
			curTechEntry->buildRenderState();

			// Acquire the gpu programs.
			curTechEntry->acquireGpuPrograms();

			return true;
		}					
	}

	return false;
}

}
}
