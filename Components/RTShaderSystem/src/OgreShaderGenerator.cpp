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
RTShader::ShaderGenerator* Singleton<RTShader::ShaderGenerator>::ms_Singleton = 0;

namespace RTShader {

String ShaderGenerator::DEFAULT_SCHEME_NAME = "ShaderGeneratorDefaultScheme";

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
	mLightCount[0]		= 0;
	mLightCount[1]		= 0;
	mLightCount[2]		= 0;
}

//-----------------------------------------------------------------------------
ShaderGenerator::~ShaderGenerator()
{
	
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::initialize()
{
	if (ms_Singleton == NULL)
	{
		ms_Singleton = new ShaderGenerator;
		if (ms_Singleton->_initialize())
			return true;
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

	// Remove current scene manager listeners.
	setSceneManager(NULL);

	// Delete render object listener.
	if (mRenderObjectListener != NULL)
	{
		delete mRenderObjectListener;
		mRenderObjectListener = NULL;
	}

	// Delete scene manager listener.
	if (mSceneManagerListener != NULL)
	{
		delete mSceneManagerListener;
		mSceneManagerListener = NULL;
	}		
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
void ShaderGenerator::getRenderStateList(const String& schemeName, 
									 const String& materialName, 
									 RenderStateList& renderStateList)
{
	OGRE_LOCK_AUTO_MUTEX

	SGSchemeIterator itFind = mSchemeEntriesMap.find(schemeName);

	if (itFind == mSchemeEntriesMap.end())
	{
		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
			"A scheme named'" + schemeName + "' doesn't exists.",
			"ShaderGenerator::getRenderStateList");
	}

	return itFind->second->getRenderStateList(materialName, renderStateList);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::setSceneManager(SceneManager* sceneMgr)
{
	if (mSceneMgr != sceneMgr)
	{
		if (mSceneMgr != NULL)
		{
			mSceneMgr->removeRenderObjectListener(mRenderObjectListener);		
			mSceneMgr->removeListener(mSceneManagerListener);			
		}

		mSceneMgr = sceneMgr;

		if (mSceneMgr != NULL)
		{
			if (mRenderObjectListener == NULL)
				mRenderObjectListener = new SGRenderObjectListener(this);
			
			mSceneMgr->addRenderObjectListener(mRenderObjectListener);

			if (mSceneManagerListener == NULL)
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
		const SGTechniqueList& techniqueEntires = itMatEntry->second->getTechniqueList();
		SGTechniqueConstIterator itTechEntry = techniqueEntires.begin();

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
	matEntry->getTechniqueList().push_back(techEntry);

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
		
	return true;
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::removeShaderBasedTechnique(const String& materialName, 
												 const String& srcTechniqueSchemeName, 
												 const String& dstTechniqueSchemeName)
{
	OGRE_LOCK_AUTO_MUTEX

	// Make sure material exists;
	if (false == MaterialManager::getSingleton().resourceExists(materialName))
		return false;

	// Make sure scheme exists.
	SGSchemeIterator itScheme = mSchemeEntriesMap.find(dstTechniqueSchemeName);
	SGScheme* schemeEntry = NULL;

	if (itScheme == mSchemeEntriesMap.end())	
		return false;
	
	schemeEntry = itScheme->second;
	


	// Find the material entry.
	SGMaterialIterator itMatEntry = mMaterialEntriesMap.find(materialName);

	// Case material not found.
	if (itMatEntry == mMaterialEntriesMap.end())
		return false;


	SGTechniqueList& matTechniqueEntires = itMatEntry->second->getTechniqueList();
	SGTechniqueIterator itTechEntry = matTechniqueEntires.begin();
	SGTechnique* dstTechnique = NULL;

	// Remove destination technique entry from material techniques list.
	for (; itTechEntry != matTechniqueEntires.end(); ++itTechEntry)
	{		
		if ((*itTechEntry)->getSourceTechnique()->getSchemeName() == srcTechniqueSchemeName &&
			(*itTechEntry)->getDestinationTechnique()->getSchemeName() == dstTechniqueSchemeName)
		{
			dstTechnique = *itTechEntry;
			matTechniqueEntires.erase(itTechEntry);
			break;			
		}		
	}

	// Technique not found.
	if (dstTechnique == NULL)	
		return false;	
	

	schemeEntry->removeTechniqueEntry(dstTechnique);

	// Case this scheme has no other techniques -> remove it.
	if (schemeEntry->empty())
	{
		delete itScheme->second;
		mSchemeEntriesMap.erase(itScheme);
	}

	// Case this material has no other techniques -> remove it.
	if (matTechniqueEntires.empty())
	{
		delete itMatEntry->second;
		mMaterialEntriesMap.erase(itMatEntry);
	}
	
	// Delete technique entries.
	for (SGTechniqueIterator itTech = mTechniqueEntriesList.begin(); itTech != mTechniqueEntriesList.end(); ++itTech)
	{			
		if (dstTechnique == (*itTech))
		{
			delete dstTechnique;
			mTechniqueEntriesList.erase(itTech);
			break;
		}		
	}
	
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
		itScheme->second->invalidate();	
	
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

	return true;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::invalidateMaterial(const String& schemeName, const String& materialName)
{
	OGRE_LOCK_AUTO_MUTEX

	SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);
	
	if (itScheme != mSchemeEntriesMap.end())			
		itScheme->second->invalidate(materialName);	
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::validateMaterial(const String& schemeName, const String& materialName)
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
void ShaderGenerator::setLightCount(const int lightCount[3])
{
	mLightCount[0] = lightCount[0];
	mLightCount[1] = lightCount[1];
	mLightCount[2] = lightCount[2];
}

//-----------------------------------------------------------------------
void ShaderGenerator::getLightCount(int lightCount[3]) const
{
	lightCount[0] = mLightCount[0];
	lightCount[1] = mLightCount[1];
	lightCount[2] = mLightCount[2];
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
	int lightCount[3] = {0};

	// Use light count definitions of the custom render state if exists.
	if (mCustomRenderState != NULL)
	{
		mCustomRenderState->getLightCount(lightCount);
	}

	// Use light count definitions of the global render state if exists.
	else if (renderStateGlobal != NULL)
	{
		renderStateGlobal->getLightCount(lightCount);
	}
	
	localRenderState->setLightCount(lightCount);
			
	// Build the FFP state.	
	FFPRenderStateBuilder::getSingleton().buildRenderState(this, localRenderState);


	// Append custom render state of this pass if exists.
	if (mCustomRenderState != NULL)
	{
		localRenderState->append(*mCustomRenderState);
	}

	// Append global render state of the shader generator.
	else if (renderStateGlobal != NULL)
	{
		localRenderState->append(*renderStateGlobal);
	}

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
RenderState* ShaderGenerator::SGPass::getCustomRenderState()
{
	if (mCustomRenderState == NULL) 
		mCustomRenderState = new RenderState; 
	
	return mCustomRenderState;
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

	mBuildDstTechnique = true;
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGTechnique::~SGTechnique()
{
	Material* parentMaterial = mSrcTechnique->getParent();

	for (unsigned int i=0; i < parentMaterial->getNumTechniques(); ++i)
	{
		if (mDstTechnique == parentMaterial->getTechnique(i))
		{
			parentMaterial->removeTechnique(i);			
			break;
		}		
	}
	
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
void ShaderGenerator::SGTechnique::getRenderStateList(RenderStateList& renderStateList)
{
	for (SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); ++itPass)
	{
		renderStateList.push_back((*itPass)->getCustomRenderState());
	}
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGScheme::SGScheme()
{
	mOutOfDate	 = true;
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
void ShaderGenerator::SGScheme::getRenderStateList(const String& materialName, RenderStateList& renderStateList)
{
	SGTechniqueIterator itTech;

	// Find the desired technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getSourceTechnique()->getParent()->getName() == materialName)
		{
			curTechEntry->getRenderStateList(renderStateList);
			break;
		}
	}
}


//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::addTechniqueEntry(SGTechnique* techEntry)
{
	mTechniqueEntires.push_back(techEntry);

	// Mark as out of data.
	mOutOfDate = true;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::removeTechniqueEntry(SGTechnique* techEntry)
{
	SGTechniqueIterator itTech;

	// Build render state for each technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry == techEntry)
		{
			mTechniqueEntires.erase(itTech);
			break;
		}		
	}
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::validate()
{

	// The target scheme is up to date.
	if (mOutOfDate == false)
		return;
	
	SGTechniqueIterator itTech;

	// Build render state for each technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getBuildDestinationTechnique())
			curTechEntry->buildRenderState();		
	}

	// Acquire GPU programs for each technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getBuildDestinationTechnique())
			curTechEntry->acquireGpuPrograms();		
	}

	// Turn off the build destination technique flag.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		curTechEntry->setBuildDestinationTechnique(false);
	}
	
	// Mark this scheme as up to date.
	mOutOfDate = false;
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::SGScheme::validate(const String& materialName)
{
	SGTechniqueIterator itTech;

	// Find the desired technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getSourceTechnique()->getParent()->getName() == materialName &&
			curTechEntry->getBuildDestinationTechnique())
		{
			// Build render state for each technique.
			curTechEntry->buildRenderState();

			// Acquire the GPU programs.
			curTechEntry->acquireGpuPrograms();

			// Turn off the build destination technique flag.
			curTechEntry->setBuildDestinationTechnique(false);

			return true;
		}					
	}

	return false;
}
//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::invalidate(const String& materialName)
{
	SGTechniqueIterator itTech;

	// Find the desired technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getSourceTechnique()->getParent()->getName() == materialName &&
			curTechEntry->getBuildDestinationTechnique())
		{			
			// Turn on the build destination technique flag.
			curTechEntry->setBuildDestinationTechnique(true);
			break;			
		}					
	}

	mOutOfDate = true;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::invalidate()
{	
	SGTechniqueIterator itTech;
	
	// Turn on the build destination technique flag of all techniques.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		curTechEntry->setBuildDestinationTechnique(true);
	}

	mOutOfDate = true;			
}

}
}