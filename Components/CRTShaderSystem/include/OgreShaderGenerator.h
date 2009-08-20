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
#ifndef _ShaderGenerator_
#define _ShaderGenerator_

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreRenderObjectListener.h"
#include "OgreSceneManager.h"
#include "OgreShaderRenderState.h"

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(OGRE_STATIC_LIB)
#   ifdef OGRE_CRTSHADERGENERATOR_EXPORTS
#       define OGRE_CRTSHADER_API __declspec(dllexport)
#   else
#       if defined(__MINGW32__)
#           define OGRE_CRTSHADER_API
#       else
#           define OGRE_CRTSHADER_API __declspec(dllimport)
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define OGRE_CRTSHADER_API  __attribute__ ((visibility("default")))
#else
#    define OGRE_CRTSHADER_API
#endif

namespace Ogre {

class RenderObjectListener;

namespace CRTShader {

class RenderState;
class SubRenderState;
class ProgramManager;
class FFPRenderStateBuilder;
class SubRenderStateFactory;

/************************************************************************/
/*                                                                      */
/************************************************************************/
class OGRE_CRTSHADER_API ShaderGenerator : public Singleton<ShaderGenerator>
{
	// Interface.
public:
	static bool		initialize	(SceneManager* sceneMgr);
	static void		finalize	();

	static ShaderGenerator&			getSingleton	();	
	static ShaderGenerator*			getSingletonPtr	();

	void			setSceneManager				(SceneManager* sceneMgr);
	SceneManager*	getSceneManager				();
	
	void			setShaderLanguage			(const String& shaderLanguage) { mShaderLanguage = shaderLanguage; }
	const String&	getShaderLanguage			() const { return mShaderLanguage; }

	void			setVertexShaderProfiles		(const String& vertexShaderProfile) { mVertexShaderProfiles = vertexShaderProfile; }
	const String&	getVertexShaderProfiles		() const { return mVertexShaderProfiles; }

	void			setFragmentShaderProfiles	(const String& FragmentShaderProfile) { mFragmentShaderProfiles = FragmentShaderProfile; }
	const String&	getFragmentShaderProfiles	() const { return mFragmentShaderProfiles; }

	void			setShaderCachePath			(const String& cachePath);
	const String&	getShaderCachePath			() const { return mShaderCachePath; }
	
	RenderState*	getRenderState				(const String& schemeName);

	void			addSubRenderStateFactory	(SubRenderStateFactory* factory);
	void			removeSubRenderStateFactory	(SubRenderStateFactory* factory);
	SubRenderState*	createSubRenderState		(const String& type);
	void			destroySubRenderState		(SubRenderState* subRenderState);


	bool			createShaderBasedTechnique	(const String& materialName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName);

	void			invalidateScheme			(const String& schemeName);
	bool			validateScheme				(const String& schemeName);
	bool			validateScheme				(const String& schemeName, const String& materialName);

	void			setMaxLightCount			(const int maxLightCount[3]);
	void			getMaxLightCount			(int maxLightCount[3]) const;


// Public types.
public:
	class SGPass;
	class SGTechnique;
	class SGMaterial;
	class SGScheme;

	//-----------------------------------------------------------------------------
	class SGPass
	{
	public:
		SGPass			(SGTechnique* parent, Pass* srcPass, Pass* dstPass);
		~SGPass			();
	
		void			buildRenderState			();
		void			acquireGpuPrograms			();
		void			notifyRenderSingleObject	(Renderable* rend, const AutoParamDataSource* source, const LightList* pLightList, bool suppressRenderStateChanges);

		Pass*			getSrcPass					() { return mSrcPass; }
		Pass*			getDstPass					() { return mDstPass; }

		SubRenderState*	getCustomFFPSubState		(int subStateOrder);

	protected:
		SubRenderState*	getCustomFFPSubState		(int subStateOrder, const RenderState* renderState);
	
	protected:
		SGTechnique*	mParent;
		Pass*			mSrcPass;
		Pass*			mDstPass;
			
		RenderState*	mCustomRenderState;	
		RenderStatePtr	mFinalRenderState;		
	};

	typedef std::vector<SGPass*>			SGPassList;
	typedef SGPassList::iterator			SGPassIterator;
	typedef SGPassList::const_iterator		SGPassConstIterator;

	//-----------------------------------------------------------------------------
	class SGTechnique
	{
	public:
		SGTechnique			(SGMaterial* parent, Technique* srcTechnique, const String& dstTechniqueSchemeName);
		~SGTechnique		();

		Technique*			getSourceTechnique			() { return mSrcTechnique; }
		Technique*			getDestinationTechnique		() { return mDstTechnique; }
		
		void				buildRenderState			();
		void				acquireGpuPrograms			();
		
	protected:
		SGMaterial*		mParent;
		Technique*		mSrcTechnique;
		Technique*		mDstTechnique;		
		SGPassList		mPassEntries;
	};

	typedef std::vector<SGTechnique*>			SGTechniqueList;
	typedef SGTechniqueList::iterator			SGTechniqueIterator;
	typedef SGTechniqueList::const_iterator		SGTechniqueConstIterator;

	//-----------------------------------------------------------------------------
	class SGMaterial
	{	
	public:
		SGTechniqueList		mTechniqueEntires;
	};

	typedef std::map<const String, SGMaterial*>		SGMaterialMap;
	typedef SGMaterialMap::iterator					SGMaterialIterator;
	typedef SGMaterialMap::const_iterator			SGMaterialConstIterator;

	//-----------------------------------------------------------------------------
	class SGScheme
	{	
	public:
		SGScheme		();
		~SGScheme		();

		bool					validated				() const		{ return mIsValid; }
		void					setValid				(bool valid) 	{ mIsValid = valid; }
		
		void					validate				();
		bool					validate				(const String& materialName);

		

		void					addTechniqueEntry		(SGTechnique* techEntry);

		RenderState*			getRenderState			();

	protected:
		SGTechniqueList			mTechniqueEntires;
		bool					mIsValid;
		RenderState*			mRenderState;
	};

	typedef std::map<const String, SGScheme*>			SGSchemeMap;
	typedef SGSchemeMap::iterator						SGSchemeIterator;
	typedef SGMaterialMap::const_iterator				SGSchemeConstIterator;
	

// Protected types.
protected:
	//-----------------------------------------------------------------------------
	class SGRenderObjectListener : public RenderObjectListener
	{
	public:
		SGRenderObjectListener(ShaderGenerator* owner)
		{
			mOwner = owner;
		}

		virtual void notifyRenderSingleObject(Renderable* rend, const Pass* pass,  
			const AutoParamDataSource* source, 
			const LightList* pLightList, bool suppressRenderStateChanges)
		{
			mOwner->notifyRenderSingleObject(rend, pass, source, pLightList, suppressRenderStateChanges);
		}

	protected:
		ShaderGenerator* mOwner;
	};

	//-----------------------------------------------------------------------------
	class SGSceneManagerListener : public SceneManager::Listener
	{
	public:
		SGSceneManagerListener(ShaderGenerator* owner)
		{
			mOwner = owner;
		}

		virtual void preFindVisibleObjects(SceneManager* source, 
			SceneManager::IlluminationRenderStage irs, Viewport* v)
		{
			mOwner->preFindVisibleObjects(source, irs, v);
		}

		virtual void postFindVisibleObjects(SceneManager* source, 
			SceneManager::IlluminationRenderStage irs, Viewport* v)
		{

		}

		virtual void shadowTexturesUpdated(size_t numberOfShadowTextures) 
		{

		}

		virtual void shadowTextureCasterPreViewProj(Light* light, 
			Camera* camera, size_t iteration) 
		{

		}

		virtual void shadowTextureReceiverPreViewProj(Light* light, 
			Frustum* frustum)
		{

		}

	protected:
		ShaderGenerator* mOwner;
	};

	//-----------------------------------------------------------------------------
	typedef std::map<String, SubRenderStateFactory*> 		SubRenderStateFactoryMap;
	typedef SubRenderStateFactoryMap::iterator 				SubRenderStateFactoryIterator;
	typedef SubRenderStateFactoryMap::const_iterator		SubRenderStateFactoryConstIterator;

	//-----------------------------------------------------------------------------
	typedef std::map<uint32, RenderStatePtr> 			RenderStateMap;
	typedef RenderStateMap::iterator 					RenderStateIterator;
	typedef RenderStateMap::const_iterator				RenderStateConstIterator;

protected:
	bool				_initialize			();
	void				_finalize			();

	Technique*			findSourceTechnique				(const String& materialName, const String& srcTechniqueSchemeName);

	void				notifyRenderSingleObject		(Renderable* rend, const Pass* pass,  const AutoParamDataSource* source, const LightList* pLightList, bool suppressRenderStateChanges);
	void				preFindVisibleObjects			(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v);
	RenderStatePtr		getCachedRenderState			(uint hashCode);
	void				addRenderStateToCache			(RenderStatePtr renderStatePtr);			
	
protected:
	ShaderGenerator		();
	~ShaderGenerator	();

protected:	
	OGRE_AUTO_MUTEX												// Auto mutex.
	SceneManager*				mSceneMgr;						// The current scene manager.
	SGRenderObjectListener*		mRenderObjectListener;			// Render object listener.
	SGSceneManagerListener*		mSceneManagerListener;			// Scene manager listener.
	String						mShaderLanguage;				// The target shader language (currently only cg supported).
	String						mVertexShaderProfiles;			// The target vertex shader profile. Will be used as argument for program compilation.
	String						mFragmentShaderProfiles;		// The target Fragment shader profile. Will be used as argument for program compilation.
	String						mShaderCachePath;				// Path for caching the generated shaders.
	ProgramManager*				mProgramManager;				// Shader program manager.
	FFPRenderStateBuilder*		mFFPRenderStateBuilder;			// Fixed Function Render state builder.
	SGMaterialMap				mMaterialEntriesMap;			// Material entries map.
	SGSchemeMap					mSchemeEntriesMap;				// Scheme entries map.
	SGTechniqueList				mTechniqueEntriesList;			// All technique entries.
	RenderStateMap				mCachedRenderStates;			// All cached render states.
	SubRenderStateFactoryMap	mSubRenderStateFactoryMap;		// Sub render state registered factories.
	bool						mActiveViewportValid;			// True if active view port use a valid SGScheme.
	int							mMaxLightCount[3];				// Max light count per light type.

private:
	friend class SGPass;
	
};

}
}

#endif

