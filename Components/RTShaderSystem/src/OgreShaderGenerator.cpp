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
#include "OgreShaderExPerPixelLighting.h"
#include "OgreShaderExNormalMapLighting.h"
#include "OgreShaderExIntegratedPSSM3.h"
#include "OgreShaderMaterialSerializerListener.h"
#include "OgreShaderProgramWriterManager.h"
#include "OgreHighLevelGpuProgramManager.h"


namespace Ogre {

//-----------------------------------------------------------------------
template<> 
RTShader::ShaderGenerator* Singleton<RTShader::ShaderGenerator>::ms_Singleton = 0;

namespace RTShader {

String ShaderGenerator::DEFAULT_SCHEME_NAME		= "ShaderGeneratorDefaultScheme";
String ShaderGenerator::SGPass::UserKey			= "SGPass";
String ShaderGenerator::SGTechnique::UserKey	= "SGTechnique";

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
	mProgramWriterManager		= NULL;
	mProgramManager				= NULL;
	mFFPRenderStateBuilder		= NULL;
	mActiveSceneMgr				= NULL;
	mRenderObjectListener		= NULL;
	mSceneManagerListener		= NULL;
	mScriptTranslatorManager	= NULL;
	mMaterialSerializerListener	= NULL;
	mActiveViewportValid		= false;
	mLightCount[0]				= 0;
	mLightCount[1]				= 0;
	mLightCount[2]				= 0;
	mVSOutputCompactPolicy		= VSOCP_LOW;


	mShaderLanguage = "";
	
	HighLevelGpuProgramManager& hmgr = HighLevelGpuProgramManager::getSingleton();

	if (hmgr.isLanguageSupported("cg"))
	{
		mShaderLanguage	= "cg";
	}
	else if (hmgr.isLanguageSupported("glsl"))
	{
		mShaderLanguage	= "glsl";
	}
	else if (hmgr.isLanguageSupported("hlsl"))
	{
		mShaderLanguage	= "hlsl";
	}
	else
	{
		// ASSAF: This is disabled for now - to stop an exception on the iPhone
		// when running with the OpenGL ES 1.x that doesn't support shaders...
		/*
		OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
			"ShaderGenerator creation error: None of the profiles is supported.", 
			"ShaderGenerator::ShaderGenerator" );

		*/
		mShaderLanguage	= "cg"; // HACK for now
	}

	setVertexShaderProfiles("gpu_vp gp4vp vp40 vp30 arbvp1 vs_4_0 vs_3_0 vs_2_x vs_2_a vs_2_0 vs_1_1");
	setFragmentShaderProfiles("ps_4_0 ps_3_x ps_3_0 fp40 fp30 fp20 arbfp1 ps_2_x ps_2_a ps_2_b ps_2_0 ps_1_4 ps_1_3 ps_1_2 ps_1_1");
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
		ms_Singleton = OGRE_NEW ShaderGenerator;
		if (false == ms_Singleton->_initialize())
		{
			OGRE_DELETE ms_Singleton;
			ms_Singleton = NULL;
			return false;
		}
	}
		
	return true;
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::_initialize()
{
	OGRE_LOCK_AUTO_MUTEX

	// Allocate program writer manager.
	mProgramWriterManager = OGRE_NEW ProgramWriterManager;

	// Allocate program manager.
	mProgramManager			= OGRE_NEW ProgramManager;

	// Allocate and initialize FFP render state builder.
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
	mFFPRenderStateBuilder	= OGRE_NEW FFPRenderStateBuilder;
	if (false == mFFPRenderStateBuilder->initialize())
		return false;
#endif

	// Create extensions factories.
	createSubRenderStateExFactories();

	// Allocate script translator manager.
	mScriptTranslatorManager = OGRE_NEW SGScriptTranslatorManager(this);
	ScriptCompilerManager::getSingleton().addTranslatorManager(mScriptTranslatorManager);

	addCustomScriptTranslator("rtshader_system", &mCoreScriptTranslaotr);

	// Create the default scheme.
	createScheme(DEFAULT_SCHEME_NAME);

	return true;
}



//-----------------------------------------------------------------------------
void ShaderGenerator::createSubRenderStateExFactories()
{
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactory* curFactory;

	curFactory = OGRE_NEW PerPixelLightingFactory;	
	addSubRenderStateFactory(curFactory);
	mSubRenderStateExFactories[curFactory->getType()] = (curFactory);

	curFactory = OGRE_NEW NormalMapLightingFactory;	
	addSubRenderStateFactory(curFactory);
	mSubRenderStateExFactories[curFactory->getType()] = (curFactory);

	curFactory = OGRE_NEW IntegratedPSSM3Factory;	
	addSubRenderStateFactory(curFactory);
	mSubRenderStateExFactories[curFactory->getType()] = (curFactory);

#endif
}

//-----------------------------------------------------------------------------
void ShaderGenerator::finalize()
{
	if (ms_Singleton != NULL)
	{
		ms_Singleton->_finalize();

		OGRE_DELETE ms_Singleton;
		ms_Singleton = NULL;
	}
}

//-----------------------------------------------------------------------------
void ShaderGenerator::_finalize()
{
	OGRE_LOCK_AUTO_MUTEX
	
	
	// Delete technique entries.
	for (SGTechniqueMapIterator itTech = mTechniqueEntriesMap.begin(); itTech != mTechniqueEntriesMap.end(); ++itTech)
	{			
		OGRE_DELETE (itTech->second);
	}
	mTechniqueEntriesMap.clear();

	// Delete material entries.
	for (SGMaterialIterator itMat = mMaterialEntriesMap.begin(); itMat != mMaterialEntriesMap.end(); ++itMat)
	{		
		OGRE_DELETE (itMat->second);
	}
	mMaterialEntriesMap.clear();

	// Delete scheme entries.
	for (SGSchemeIterator itScheme = mSchemeEntriesMap.begin(); itScheme != mSchemeEntriesMap.end(); ++itScheme)
	{		
		OGRE_DELETE (itScheme->second);
	}
	mSchemeEntriesMap.clear();

	// Destroy extensions factories.
	destroySubRenderStateExFactories();

#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
	// Delete FFP Emulator.
	if (mFFPRenderStateBuilder != NULL)
	{
		mFFPRenderStateBuilder->finalize();
		OGRE_DELETE mFFPRenderStateBuilder;
		mFFPRenderStateBuilder = NULL;
	}
#endif

	// Delete Program manager.
	if (mProgramManager != NULL)
	{
		OGRE_DELETE mProgramManager;
		mProgramManager = NULL;
	}

	// Delete Program writer manager.
	if(mProgramWriterManager != NULL)
	{
		OGRE_DELETE mProgramWriterManager;
		mProgramWriterManager = NULL;
	}

	removeCustomScriptTranslator("rtshader_system");

	// Delete script translator manager.
	if (mScriptTranslatorManager != NULL)
	{
		ScriptCompilerManager::getSingleton().removeTranslatorManager(mScriptTranslatorManager);
		OGRE_DELETE mScriptTranslatorManager;
		mScriptTranslatorManager = NULL;
	}

	// Delete material Serializer listener.
	if (mMaterialSerializerListener != NULL)
	{
		OGRE_DELETE mMaterialSerializerListener;
		mMaterialSerializerListener = NULL;
	}

	// Remove all scene managers.	
	while (mSceneManagerMap.empty() == false)
	{
		SceneManagerIterator itSceneMgr    = mSceneManagerMap.begin();

		removeSceneManager(itSceneMgr->second);
	}

	// Delete render object listener.
	if (mRenderObjectListener != NULL)
	{
		OGRE_DELETE mRenderObjectListener;
		mRenderObjectListener = NULL;
	}

	// Delete scene manager listener.
	if (mSceneManagerListener != NULL)
	{
		OGRE_DELETE mSceneManagerListener;
		mSceneManagerListener = NULL;
	}		
}

//-----------------------------------------------------------------------------
void ShaderGenerator::destroySubRenderStateExFactories()
{
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactoryIterator it;

	for (it = mSubRenderStateExFactories.begin(); it != mSubRenderStateExFactories.end(); ++it)
	{
		removeSubRenderStateFactory(it->second);		
		OGRE_DELETE it->second;		
	}
	mSubRenderStateExFactories.clear();
}

//-----------------------------------------------------------------------------
void ShaderGenerator::addSubRenderStateFactory(SubRenderStateFactory* factory)
{
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactoryIterator itFind = mSubRenderStateFactories.find(factory->getType());

	if (itFind != mSubRenderStateFactories.end())
	{
		OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
			"A factory of type '" + factory->getType() + "' already exists.",
			"ShaderGenerator::addSubRenderStateFactory");
	}		
	
	mSubRenderStateFactories[factory->getType()] = factory;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::removeSubRenderStateFactory(SubRenderStateFactory* factory)
{
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactoryIterator itFind = mSubRenderStateFactories.find(factory->getType());

	if (itFind != mSubRenderStateFactories.end())
		mSubRenderStateFactories.erase(itFind);

}

//-----------------------------------------------------------------------------
SubRenderState*	ShaderGenerator::createSubRenderState(const String& type)
{
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactoryIterator itFind = mSubRenderStateFactories.find(type);

	if (itFind != mSubRenderStateFactories.end())
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

	SubRenderStateFactoryIterator itFind = mSubRenderStateFactories.find(subRenderState->getType());

	if (itFind != mSubRenderStateFactories.end())
	{
		 itFind->second->destroyInstance(subRenderState);
	}	
}

//-----------------------------------------------------------------------------
SubRenderState*	ShaderGenerator::createSubRenderState(ScriptCompiler* compiler, 
													  PropertyAbstractNode* prop, Pass* pass)
{
	OGRE_LOCK_AUTO_MUTEX

	SubRenderStateFactoryIterator it = mSubRenderStateFactories.begin();
	SubRenderStateFactoryIterator itEnd = mSubRenderStateFactories.end();
	SubRenderState* subRenderState = NULL;

	while (it != itEnd)
	{
		subRenderState = it->second->createInstance(compiler, prop, pass);
		if (subRenderState != NULL)		
			break;				
		++it;
	}	

	return subRenderState;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::createScheme(const String& schemeName)
{
	OGRE_LOCK_AUTO_MUTEX

	SGSchemeIterator itFind = mSchemeEntriesMap.find(schemeName);
	SGScheme* schemeEntry   = NULL;

	if (itFind == mSchemeEntriesMap.end())
	{
		schemeEntry = OGRE_NEW SGScheme(schemeName);
		mSchemeEntriesMap[schemeName] = schemeEntry;
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
RenderState* ShaderGenerator::getRenderState(const String& schemeName, 
									 const String& materialName, 
									 unsigned short passIndex)
{
	OGRE_LOCK_AUTO_MUTEX

	SGSchemeIterator itFind = mSchemeEntriesMap.find(schemeName);

	if (itFind == mSchemeEntriesMap.end())
	{
		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
			"A scheme named'" + schemeName + "' doesn't exists.",
			"ShaderGenerator::getRenderState");
	}

	return itFind->second->getRenderState(materialName, passIndex);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::addSceneManager(SceneManager* sceneMgr)
{
	// Make sure this scene manager not exists in the map.
	SceneManagerIterator itFind = mSceneManagerMap.find(sceneMgr->getName());
	
	if (itFind != mSceneManagerMap.end())
		return;

	if (mRenderObjectListener == NULL)
		mRenderObjectListener = OGRE_NEW SGRenderObjectListener(this);
	
	sceneMgr->addRenderObjectListener(mRenderObjectListener);

	if (mSceneManagerListener == NULL)
		mSceneManagerListener = OGRE_NEW SGSceneManagerListener(this);
	
	sceneMgr->addListener(mSceneManagerListener);

	mSceneManagerMap[sceneMgr->getName()] = sceneMgr;

	// Update the active scene manager.
	if (mActiveSceneMgr == NULL)
		mActiveSceneMgr = sceneMgr;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::removeSceneManager(SceneManager* sceneMgr)
{
	// Make sure this scene manager exists in the map.
	SceneManagerIterator itFind = mSceneManagerMap.find(sceneMgr->getName());
	
	if (itFind != mSceneManagerMap.end())
	{
		itFind->second->removeRenderObjectListener(mRenderObjectListener);		
		itFind->second->removeListener(mSceneManagerListener);	

		mSceneManagerMap.erase(itFind);

		// Update the active scene manager.
		if (mActiveSceneMgr == sceneMgr)
			mActiveSceneMgr = NULL;
	}
}

//-----------------------------------------------------------------------------
SceneManager* ShaderGenerator::getActiveSceneManager()
{
	return mActiveSceneMgr;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::setVertexShaderProfiles(const String& vertexShaderProfiles)
{
	mVertexShaderProfiles = vertexShaderProfiles;
	mVertexShaderProfilesList = StringUtil::split(vertexShaderProfiles);
}
//-----------------------------------------------------------------------------
void ShaderGenerator::setFragmentShaderProfiles(const String& fragmentShaderProfiles)
{
	mFragmentShaderProfiles = fragmentShaderProfiles;
	mFragmentShaderProfilesList = StringUtil::split(fragmentShaderProfiles);
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
				(*itTechEntry)->getDestinationTechniqueSchemeName() == dstTechniqueSchemeName)
			{
				return true;
			}


			// Case a shader based technique with the same scheme name already defined based 
			// on different source technique. 
			// This state might lead to conflicts during shader generation - we prevent it by returning false here.
			else if ((*itTechEntry)->getDestinationTechniqueSchemeName() == dstTechniqueSchemeName)
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
		matEntry = OGRE_NEW SGMaterial(materialName);
		mMaterialEntriesMap[materialName] = matEntry;
	}
	else
	{
		matEntry = itMatEntry->second;
	}

	// Create the new technique entry.
	SGTechnique* techEntry = OGRE_NEW SGTechnique(matEntry, srcTechnique, dstTechniqueSchemeName);
					

	// Add to material entry map.
	matEntry->getTechniqueList().push_back(techEntry);

	// Add to all technique map.
	mTechniqueEntriesMap[techEntry] = techEntry;

	// Add to scheme.
	SGSchemeIterator itScheme = mSchemeEntriesMap.find(dstTechniqueSchemeName);
	SGScheme* schemeEntry = NULL;

	if (itScheme == mSchemeEntriesMap.end())
	{
		schemeEntry = OGRE_NEW SGScheme(dstTechniqueSchemeName);
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
			(*itTechEntry)->getDestinationTechniqueSchemeName() == dstTechniqueSchemeName)
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

	SGTechniqueMapIterator itTechMap = mTechniqueEntriesMap.find(dstTechnique);

	if (itTechMap != mTechniqueEntriesMap.end())	
		mTechniqueEntriesMap.erase(itTechMap);				
	
	OGRE_DELETE dstTechnique;
		
	return true;
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::removeAllShaderBasedTechniques(const String& materialName)
{
	OGRE_LOCK_AUTO_MUTEX

	// Find the material entry.
	SGMaterialIterator itMatEntry = mMaterialEntriesMap.find(materialName);

	// Case material not found.
	if (itMatEntry == mMaterialEntriesMap.end())
		return false;


	SGTechniqueList& matTechniqueEntires = itMatEntry->second->getTechniqueList();
		
	// Remove all technique entries from material techniques list.
	while (matTechniqueEntires.empty() == false)
	{	
		SGTechniqueIterator itTechEntry = matTechniqueEntires.begin();

		removeShaderBasedTechnique(materialName, (*itTechEntry)->getSourceTechnique()->getSchemeName(), 
			(*itTechEntry)->getDestinationTechniqueSchemeName());		
	}

	OGRE_DELETE itMatEntry->second;
	mMaterialEntriesMap.erase(itMatEntry);
	
	return true;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::removeAllShaderBasedTechniques()
{
	OGRE_LOCK_AUTO_MUTEX

	while (mMaterialEntriesMap.size() > 0)
	{
		SGMaterialIterator itMatEntry = mMaterialEntriesMap.begin();

		removeAllShaderBasedTechniques(itMatEntry->first);
	}
}
												 
//-----------------------------------------------------------------------------
 Technique* ShaderGenerator::findSourceTechnique(const String& materialName, const String& srcTechniqueSchemeName)
 {
	 MaterialPtr mat = MaterialManager::getSingleton().getByName(materialName);
	 Material::TechniqueIterator itMatTechniques = mat->getTechniqueIterator();
	 

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
	if (mActiveViewportValid)
	{		
		const Any& passUserData = pass->getUserObjectBindings().getUserAny(SGPass::UserKey);

		if (passUserData.isEmpty())	
			return;	

		OGRE_LOCK_AUTO_MUTEX

		SGPass* passEntry = any_cast<SGPass*>(passUserData);

		passEntry->notifyRenderSingleObject(rend, source, pLightList, suppressRenderStateChanges);
						
	}	
}


//-----------------------------------------------------------------------------
void ShaderGenerator::preFindVisibleObjects(SceneManager* source, 
											SceneManager::IlluminationRenderStage irs, 
											Viewport* v)
{
	OGRE_LOCK_AUTO_MUTEX

	const String& curMaterialScheme = v->getMaterialScheme();
		
	mActiveSceneMgr      = source;
	mActiveViewportValid = validateScheme(curMaterialScheme);
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
SGMaterialSerializerListener* ShaderGenerator::getMaterialSerializerListener()
{
	if (mMaterialSerializerListener == NULL)
		mMaterialSerializerListener = OGRE_NEW SGMaterialSerializerListener;

	return mMaterialSerializerListener;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::flushShaderCache()
{
	SGTechniqueMapIterator itTech = mTechniqueEntriesMap.begin();
	SGTechniqueMapIterator itTechEnd = mTechniqueEntriesMap.end();

	// Release all programs.
	for (; itTech != itTechEnd; ++itTech)
	{			
		itTech->second->releasePrograms();
	}

	ProgramManager::getSingleton().flushGpuProgramsCache();
	
	SGSchemeIterator itScheme = mSchemeEntriesMap.begin();
	SGSchemeIterator itSchemeEnd = mSchemeEntriesMap.end();

	// Invalidate all schemes.
	for (; itScheme != itSchemeEnd; ++itScheme)
	{
		itScheme->second->invalidate();
	}
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::addCustomScriptTranslator(const String& key, ScriptTranslator* translator)
{
	OGRE_LOCK_AUTO_MUTEX

	SGScriptTranslatorIterator itFind = mScriptTranslatorsMap.find(key);

	if (itFind != mScriptTranslatorsMap.end())	
		return false;
	
	mScriptTranslatorsMap[key] = translator;

	return true;
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::removeCustomScriptTranslator(const String& key)
{
	OGRE_LOCK_AUTO_MUTEX

	SGScriptTranslatorIterator itFind = mScriptTranslatorsMap.find(key);

	if (itFind == mScriptTranslatorsMap.end())	
		return false;

	mScriptTranslatorsMap.erase(itFind);

	return true;
}

//-----------------------------------------------------------------------------
size_t ShaderGenerator::getNumTranslators() const
{
	OGRE_LOCK_AUTO_MUTEX

	return mScriptTranslatorsMap.size();
}

//-----------------------------------------------------------------------------
ScriptTranslator* ShaderGenerator::getTranslator(const AbstractNodePtr& node)
{
	OGRE_LOCK_AUTO_MUTEX

	ScriptTranslator *translator = 0;
	
	if(node->type == Ogre::ANT_OBJECT)
	{
		ObjectAbstractNode *obj			  = reinterpret_cast<ObjectAbstractNode*>(node.get());
		SGScriptTranslatorIterator itFind = mScriptTranslatorsMap.find(obj->cls);
		
		if(itFind != mScriptTranslatorsMap.end())
			translator = itFind->second;
	}
	
	return translator;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::serializePassAttributes(MaterialSerializer* ser, SGPass* passEntry)
{
	
	// Write section header and begin it.
	ser->writeAttribute(3, "rtshader_system");
	ser->beginSection(3);

	// Grab the custom render state this pass uses.
	RenderState* customenderState = passEntry->getCustomRenderState();

	if (customenderState != NULL)
	{
		// Write each of the sub-render states that composing the final render state.
		const SubRenderStateList& subRenderStates = customenderState->getTemplateSubRenderStateList();
		SubRenderStateListConstIterator it		= subRenderStates.begin();
		SubRenderStateListConstIterator itEnd	= subRenderStates.end();


		for (; it != itEnd; ++it)
		{
			SubRenderState* curSubRenderState = *it;
			SubRenderStateFactoryIterator itFactory = mSubRenderStateFactories.find(curSubRenderState->getType());

			if (itFactory != mSubRenderStateFactories.end())
			{
				SubRenderStateFactory* curFactory = itFactory->second;

				curFactory->writeInstance(ser, curSubRenderState, passEntry->getSrcPass(), passEntry->getDstPass());
			}
		}
	}
	
	// Write section end.
	ser->endSection(3);		
}

//-----------------------------------------------------------------------------
size_t ShaderGenerator::getVertexShaderCount() const
{
	return mProgramManager->getVertexShaderCount();
}

//-----------------------------------------------------------------------------
size_t ShaderGenerator::getFragmentShaderCount() const
{
	return mProgramManager->getFragmentShaderCount();
}

//-----------------------------------------------------------------------------
void ShaderGenerator::setTargetLanguage(const String& shaderLanguage)
{
	// Make sure that the shader language is supported.
	if (mProgramWriterManager->isLanguageSupported(shaderLanguage) == false)
	{
		OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
			"The language " + shaderLanguage + " is not supported !!",
			"ShaderGenerator::setShaderLanguage");
	}

	// Case target language changed -> flush the shaders cache.
	if (mShaderLanguage != shaderLanguage)
	{
		mShaderLanguage = shaderLanguage;
		flushShaderCache();
	}
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGPass::SGPass(SGTechnique* parent, Pass* srcPass, Pass* dstPass)
{
	mParent					= parent;
	mSrcPass				= srcPass;
	mDstPass				= dstPass;	
	mCustomRenderState		= NULL;
	mTargetRenderState	= NULL;
	mDstPass->getUserObjectBindings().setUserAny(SGPass::UserKey, Any(this));
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGPass::~SGPass()
{
	if (mTargetRenderState != NULL)
	{
		OGRE_DELETE mTargetRenderState;
		mTargetRenderState = NULL;
	}
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGPass::buildTargetRenderState()
{	
	const String& schemeName = mParent->getDestinationTechniqueSchemeName();
	const RenderState* renderStateGlobal = ShaderGenerator::getSingleton().getRenderState(schemeName);
	

	mTargetRenderState = OGRE_NEW TargetRenderState;

	// Set light properties.
	int lightCount[3] = {0};	

	// Use light count definitions of the custom render state if exists.
	if (mCustomRenderState != NULL && mCustomRenderState->getLightCountAutoUpdate() == false)
	{
		mCustomRenderState->getLightCount(lightCount);
	}

	// Use light count definitions of the global render state if exists.
	else if (renderStateGlobal != NULL)
	{
		renderStateGlobal->getLightCount(lightCount);
	}
	
	
	mTargetRenderState->setLightCount(lightCount);
			
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
	// Build the FFP state.	
	FFPRenderStateBuilder::getSingleton().buildRenderState(this, mTargetRenderState);
#endif


	// Link the target render state with the custom render state of this pass if exists.
	if (mCustomRenderState != NULL)
	{
		mTargetRenderState->link(*mCustomRenderState, mSrcPass, mDstPass);
	}

	// Link the target render state with the scheme render state of the shader generator.
	if (renderStateGlobal != NULL)
	{
		mTargetRenderState->link(*renderStateGlobal, mSrcPass, mDstPass);
	}				
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGPass::acquirePrograms()
{
	ProgramManager::getSingleton().acquirePrograms(mDstPass, mTargetRenderState);
	mTargetRenderState->notifyGpuProgramsAcquired(mDstPass);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGPass::releasePrograms()
{
	ProgramManager::getSingleton().releasePrograms(mDstPass, mTargetRenderState);	
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGPass::notifyRenderSingleObject(Renderable* rend,  const AutoParamDataSource* source, 
											  const LightList* pLightList, bool suppressRenderStateChanges)
{
	if (mTargetRenderState != NULL && suppressRenderStateChanges == false)
		mTargetRenderState->updateGpuProgramsParams(rend, mDstPass, source, pLightList);
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
		const String& schemeName = mParent->getDestinationTechniqueSchemeName();
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
		const SubRenderStateList& subRenderStateList = renderState->getTemplateSubRenderStateList();

		for (SubRenderStateListConstIterator it=subRenderStateList.begin(); it != subRenderStateList.end(); ++it)
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
	mParent					= parent;
	mSrcTechnique			= srcTechnique;
	mDstTechniqueSchemeName = dstTechniqueSchemeName;
	mDstTechnique			= NULL;
	mBuildDstTechnique		= true;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::createSGPasses()
{
	// Create pass entry for each pass.
	for (unsigned short i=0; i < mSrcTechnique->getNumPasses(); ++i)
	{
		Pass* srcPass = mSrcTechnique->getPass(i);
		Pass* dstPass = mDstTechnique->getPass(i);

		SGPass* passEntry = OGRE_NEW SGPass(this, srcPass, dstPass);				

		if (i < mCustomRenderStates.size())
			passEntry->setCustomRenderState(mCustomRenderStates[i]);
		mPassEntries.push_back(passEntry);
	}
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGTechnique::~SGTechnique()
{
	const String& materialName = mParent->getMaterialName();
		
	if (MaterialManager::getSingleton().resourceExists(materialName))
	{
		MaterialPtr mat = MaterialManager::getSingleton().getByName(materialName);
	
		// Remove the destination technique from parent material.
		for (unsigned int i=0; i < mat->getNumTechniques(); ++i)
		{
			if (mDstTechnique == mat->getTechnique(i))
			{
				// Unload the generated technique in order tor free referenced resources.
				mDstTechnique->_unload();

				// Remove the generated technique in order to restore the material to its original state.
				mat->removeTechnique(i);

				// Make sure the material goes back to its original state.
				mat->touch();
				break;
			}		
		}
	}

	// Release CPU/GPU programs that associated with this technique passes.
	for (SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); ++itPass)
	{
		(*itPass)->releasePrograms();
	}
	
	// Destroy the passes.
	destroySGPasses();

	// Delete the custom render states of each pass if exist.
	for (unsigned int i=0; i < mCustomRenderStates.size(); ++i)
	{
		if (mCustomRenderStates[i] != NULL)
		{
			OGRE_DELETE mCustomRenderStates[i];
			mCustomRenderStates[i] = NULL;
		}		
	}
	mCustomRenderStates.clear();
	
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::destroySGPasses()
{
	for (SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); ++itPass)
	{
		OGRE_DELETE (*itPass);
	}
	mPassEntries.clear();
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::buildTargetRenderState()
{
	// Remove existing destination technique and passes
	// in order to build it again from scratch.
	if (mDstTechnique != NULL)
	{
		Material* mat = mSrcTechnique->getParent();

		for (unsigned short i=0; i < mat->getNumTechniques(); ++i)
		{
			if (mat->getTechnique(i) == mDstTechnique)
			{
				mat->removeTechnique(i);
				break;
			}
		}
		destroySGPasses();		
	}
		
	// Create the destination technique and passes.
	mDstTechnique	= mSrcTechnique->getParent()->createTechnique();
	mDstTechnique->getUserObjectBindings().setUserAny(SGTechnique::UserKey, Any(this));
	*mDstTechnique	= *mSrcTechnique;
	mDstTechnique->setSchemeName(mDstTechniqueSchemeName);
	createSGPasses();


	// Build render state for each pass.
	for (SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); ++itPass)
	{
		(*itPass)->buildTargetRenderState();
	}
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::acquirePrograms()
{
	for (SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); ++itPass)
	{
		(*itPass)->acquirePrograms();
	}
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::releasePrograms()
{
	// Remove destination technique.
	if (mDstTechnique != NULL)
	{
		Material* mat = mSrcTechnique->getParent();

		for (unsigned short i=0; i < mat->getNumTechniques(); ++i)
		{
			if (mat->getTechnique(i) == mDstTechnique)
			{
				mat->removeTechnique(i);
				break;
			}
		}
		mDstTechnique = NULL;
	}

	// Release CPU/GPU programs that associated with this technique passes.
	for (SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); ++itPass)
	{
		(*itPass)->releasePrograms();
	}

	// Destroy the passes.
	destroySGPasses();
}

//-----------------------------------------------------------------------------
RenderState* ShaderGenerator::SGTechnique::getRenderState(unsigned short passIndex)
{
	RenderState* renderState = NULL;

	if (passIndex >= mCustomRenderStates.size())	
		mCustomRenderStates.resize(passIndex + 1, NULL);		

	renderState = mCustomRenderStates[passIndex];
	if (renderState == NULL)
	{
		renderState = OGRE_NEW RenderState;
		mCustomRenderStates[passIndex] = renderState;
	}
	
	return renderState;
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGScheme::SGScheme(const String& schemeName)
{
	mOutOfDate	 = true;
	mRenderState = NULL;
	mName		 = schemeName;
	mFogMode	 = FOG_NONE;
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGScheme::~SGScheme()
{
	if (mRenderState != NULL)
	{
		OGRE_DELETE mRenderState;
		mRenderState = NULL;
	}
}

//-----------------------------------------------------------------------------
RenderState* ShaderGenerator::SGScheme::getRenderState()
{
	if (mRenderState == NULL)
		mRenderState = OGRE_NEW RenderState;

	return mRenderState;
}

//-----------------------------------------------------------------------------
RenderState* ShaderGenerator::SGScheme::getRenderState(const String& materialName, unsigned short passIndex)
{
	SGTechniqueIterator itTech;

	// Find the desired technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getSourceTechnique()->getParent()->getName() == materialName)
		{
			return curTechEntry->getRenderState(passIndex);			
		}
	}

	return NULL;
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

	// Synchronize with light settings.
	synchronizeWithLightSettings();

	// Synchronize with fog settings.
	synchronizeWithFogSettings();

	// The target scheme is up to date.
	if (mOutOfDate == false)
		return;
	
	SGTechniqueIterator itTech;

	// Build render state for each technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getBuildDestinationTechnique())
			curTechEntry->buildTargetRenderState();		
	}

	// Acquire GPU programs for each technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getBuildDestinationTechnique())
			curTechEntry->acquirePrograms();		
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
void ShaderGenerator::SGScheme::synchronizeWithLightSettings()
{
	SceneManager* sceneManager = ShaderGenerator::getSingleton().getActiveSceneManager();

	if (sceneManager != NULL)
	{
		const LightList& lightList =  sceneManager->_getLightsAffectingFrustum();
		RenderState* curRenderState = getRenderState();
		int sceneLightCount[3] = {0};
		int currLightCount[3] = {0};

		for (unsigned int i=0; i < lightList.size(); ++i)
		{
			sceneLightCount[lightList[i]->getType()]++;
		}

		if (curRenderState->getLightCountAutoUpdate())
		{
			mRenderState->getLightCount(currLightCount);
		}

		// Case light state has been changed -> invalidate this scheme.
		if (currLightCount[0] != sceneLightCount[0] ||
			currLightCount[1] != sceneLightCount[1] ||
			currLightCount[2] != sceneLightCount[2])
		{		
			curRenderState->setLightCount(sceneLightCount);
			invalidate();
		}
	}
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::synchronizeWithFogSettings()
{
	SceneManager* sceneManager = ShaderGenerator::getSingleton().getActiveSceneManager();

	if (sceneManager != NULL && sceneManager->getFogMode() != mFogMode)
	{
		mFogMode = sceneManager->getFogMode();
		invalidate();
	}
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::SGScheme::validate(const String& materialName)
{
	// Synchronize with light settings.
	synchronizeWithLightSettings();

	// Synchronize with fog settings.
	synchronizeWithFogSettings();


	SGTechniqueIterator itTech;
	
	// Find the desired technique.
	for (itTech = mTechniqueEntires.begin(); itTech != mTechniqueEntires.end(); ++itTech)
	{
		SGTechnique* curTechEntry = *itTech;

		if (curTechEntry->getParent()->getMaterialName() == materialName &&
			curTechEntry->getBuildDestinationTechnique())
		{		
			// Build render state for each technique.
			curTechEntry->buildTargetRenderState();

			// Acquire the CPU/GPU programs.
			curTechEntry->acquirePrograms();

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

		if (curTechEntry->getParent()->getMaterialName() == materialName)
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