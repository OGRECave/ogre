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
#ifndef _ShaderGenerator_
#define _ShaderGenerator_

#include "OgreShaderPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreRenderObjectListener.h"
#include "OgreSceneManager.h"
#include "OgreShaderRenderState.h"
#include "OgreScriptTranslator.h"
#include "OgreShaderScriptTranslator.h"


namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Shader generator system main interface. This singleton based class
enables automatic generation of shader code based on existing material techniques.
*/
class _OgreRTSSExport ShaderGenerator : public Singleton<ShaderGenerator>, public RTShaderSystemAlloc
{
// Interface.
public:

	/** 
	Initialize the Shader Generator System.
	Return true upon success.
	@param sceneMgr The scene manager that the shader generator will be bound to. 
	*/
	static bool		initialize	();

	/** 
	Finalize the Shader Generator instance.
	*/
	static void		finalize	();


	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/
	static ShaderGenerator&			getSingleton	();	


	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/
	static ShaderGenerator*			getSingletonPtr	();

	/** 
	Add a scene manager to the shader generator scene managers list.
	@param sceneMgr The scene manager to add to the list.
	*/
	void			addSceneManager				(SceneManager* sceneMgr);

	/** 
	Remove a scene manager from the shader generator scene managers list.
	@param sceneMgr The scene manager to remove from the list.
	*/
	void			removeSceneManager			(SceneManager* sceneMgr);

	/** 
	Get the active scene manager that is doint the actual scene rendering.
	This attribute will be update on the call to preFindVisibleObjects. 
	*/
	SceneManager*	getActiveSceneManager		();
	
	/** 
	Set the target shader language.
	@param shaderLanguage The output shader language to use.	
	@remarks The default shader language is cg.
	*/
	void			setTargetLanguage			(const String& shaderLanguage);

	/** 
	Return the target shader language currently in use.		
	*/
	const String&	getTargetLanguage			() const { return mShaderLanguage; }

	/** 
	Set the output vertex shader target profiles.
	@param vertexShaderProfile The target profiles for the vertex shader.	
	*/
	void			setVertexShaderProfiles		(const String& vertexShaderProfiles);

	/** 
	Get the output vertex shader target profiles.	
	*/
	const String&	getVertexShaderProfiles		() const { return mVertexShaderProfiles; }

	/** 
	Get the output vertex shader target profiles as list of strings.	
	*/
	const StringVector&	getVertexShaderProfilesList		() const { return mVertexShaderProfilesList; }


	/** 
	Set the output fragment shader target profiles.
	@param fragmentShaderProfile The target profiles for the fragment shader.	
	*/
	void			setFragmentShaderProfiles	(const String& fragmentShaderProfiles);

	/** 
	Get the output fragment shader target profiles.	
	*/
	const String&	getFragmentShaderProfiles	() const { return mFragmentShaderProfiles; }

	/** 
	Get the output fragment shader target profiles as list of strings.
	*/
	const StringVector&	getFragmentShaderProfilesList	() const { return mFragmentShaderProfilesList; }

	/** 
	Set the output shader cache path. Generated shader code will be written to this path.
	In case of empty cache path shaders will be generated directly from system memory.
	@param cachePath The cache path of the shader.	
	The default is empty cache path.
	*/
	void			setShaderCachePath			(const String& cachePath);

	/** 
	Get the output shader cache path.
	*/
	const String&	getShaderCachePath			() const { return mShaderCachePath; }

	/** 
	Flush the shader cache. This operation will cause all active sachems to be invalidated and will
	destroy any CPU/GPU program that created by this shader generator.
	*/
	void			flushShaderCache			();

	/** 
	Return a global render state associated with the given scheme name.
	Modifying this render state will affect all techniques that belongs to that scheme.
	This is the best way to apply global changes to all techniques.
	After altering the render state one should call invalidateScheme method in order to 
	regenerate shaders.
	@param schemeName The destination scheme name.
	*/
	RenderState*	getRenderState				(const String& schemeName);

	/** 
	Get render state of specific pass.
	Using this method allows the user to customize the behavior of a specific pass.
	@param schemeName The destination scheme name.
	@param materialName The specific material name.
	@param passIndex The pass index.
	*/
	RenderState*	getRenderState				(const String& schemeName, const String& materialName, unsigned short passIndex);

	/** 
	Add sub render state factory. Plugins or 3d party applications may implement sub classes of
	SubRenderState interface. Add the matching factory will allow the application to create instances 
	of these sub classes.
	@param factory The factory to add.
	*/
	void			addSubRenderStateFactory	(SubRenderStateFactory* factory);

	/** 
	Remove sub render state factory. 
	@param factory The factory to remove.
	*/
	void			removeSubRenderStateFactory	(SubRenderStateFactory* factory);

	/** 
	Create an instance of sub render state from a given type. 
	@param type The type of sub render state to create.
	*/
	SubRenderState*	createSubRenderState		(const String& type);

	
	/** 
	Destroy an instance of sub render state. 
	@param subRenderState The instance to destroy.
	*/
	void			destroySubRenderState		(SubRenderState* subRenderState);


	/** 
	Checks if a shader based technique has been created for a given technique. 
	Return true if exist. False if not.
	@param materialName The source material name.
	@param srcTechniqueSchemeName The source technique scheme name.
	@param dstTechniqueSchemeName The destination shader based technique scheme name.
	*/
	bool			hasShaderBasedTechnique	(const String& materialName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName) const;

	/** 
	Create shader based technique from a given technique. 
	Return true upon success. Failure may occur if the source technique is not FFP pure, or different
	source technique is mapped to the requested destination scheme.
	@param materialName The source material name.
	@param srcTechniqueSchemeName The source technique scheme name.
	@param dstTechniqueSchemeName The destination shader based technique scheme name.
	*/
	bool			createShaderBasedTechnique	(const String& materialName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName);


	/** 
	Remove shader based technique from a given technique. 
	Return true upon success. Failure may occur if the given source technique was not previously
	registered successfully using the createShaderBasedTechnique method.
	@param materialName The source material name.
	@param srcTechniqueSchemeName The source technique scheme name.
	@param dstTechniqueSchemeName The destination shader based technique scheme name.
	*/
	bool			removeShaderBasedTechnique	(const String& materialName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName);


	/** 
	Remove all shader based techniques of the given material. 
	Return true upon success.
	@param materialName The source material name.	
	*/
	bool			removeAllShaderBasedTechniques	(const String& materialName);

	/** 
	Remove all shader based techniques that created by this shader generator.	
	*/
	void			removeAllShaderBasedTechniques	();

	/** 
	Create a scheme.
	@param schemeName The scheme name to create.
	*/
	void			createScheme				(const String& schemeName);

	/** 
	Invalidate a given scheme. This action will lead to shader regeneration of all techniques belongs to the
	given scheme name.
	@param schemeName The scheme to invalidate.
	*/
	void			invalidateScheme			(const String& schemeName);

	/** 
	Validate a given scheme. This action will generate shader programs for all techniques of the
	given scheme name.
	@param schemeName The scheme to validate.
	*/
	bool			validateScheme				(const String& schemeName);
	
	/** 
	Invalidate specific material scheme. This action will lead to shader regeneration of the technique belongs to the
	given scheme name.
	@param schemeName The scheme to invalidate.
	@param materialName The material to invalidate.
	*/
	void			invalidateMaterial			(const String& schemeName, const String& materialName);

	/** 
	Validate specific material scheme. This action will generate shader programs for the technique of the
	given scheme name.
	@param schemeName The scheme to validate.
	@param materialName The material to validate.
	*/
	bool			validateMaterial			(const String& schemeName, const String& materialName);	


	/** 
	Return custom material Serializer of the shader generator.
	This is useful when you'd like to export certain material that contains shader generator effects.
	I.E - when writing an exporter you may want mark your material as shader generated material 
	so in the next time you will load it by your application it will automatically generate shaders with custom
	attributes you wanted. To do it you'll have to do the following steps:
	1. Create shader based technique for you material via the createShaderBasedTechnique() method.
	2. Create MaterialSerializer instance.
	3. Add the return instance of serializer listener to the MaterialSerializer.
	4. Call one of the export methods of MaterialSerializer.
	*/
	SGMaterialSerializerListener*	getMaterialSerializerListener();


	/** Return the current number of generated vertex shaders. */
	size_t							getVertexShaderCount			() const;


	/** Return the current number of generated fragment shaders. */
	size_t							getFragmentShaderCount			() const;



	/** Set the vertex shader outputs compaction policy. 
	@see VSOutputCompactPolicy.
	@param policy The policy to set.
	*/
	void							setVertexShaderOutputsCompactPolicy		(VSOutputCompactPolicy policy)  { mVSOutputCompactPolicy = policy; }
	
	/** Get the vertex shader outputs compaction policy. 
	@see VSOutputCompactPolicy.	
	*/
	VSOutputCompactPolicy			getVertexShaderOutputsCompactPolicy		() const { return mVSOutputCompactPolicy; }

	/// Default material scheme of the shader generator.
	static String DEFAULT_SCHEME_NAME;

// Protected types.
protected:
	class SGPass;
	class SGTechnique;
	class SGMaterial;
	class SGScheme;

	typedef vector<SGPass*>::type					SGPassList;
	typedef SGPassList::iterator					SGPassIterator;
	typedef SGPassList::const_iterator				SGPassConstIterator;

	typedef vector<SGTechnique*>::type				SGTechniqueList;
	typedef SGTechniqueList::iterator				SGTechniqueIterator;
	typedef SGTechniqueList::const_iterator			SGTechniqueConstIterator;

	typedef map<SGTechnique*, SGTechnique*>::type	SGTechniqueMap;
	typedef SGTechniqueMap::iterator				SGTechniqueMapIterator;
	
	typedef map<String, SGMaterial*>::type			SGMaterialMap;
	typedef SGMaterialMap::iterator					SGMaterialIterator;
	typedef SGMaterialMap::const_iterator			SGMaterialConstIterator;

	typedef map<String, SGScheme*>::type			SGSchemeMap;
	typedef SGSchemeMap::iterator					SGSchemeIterator;
	typedef SGMaterialMap::const_iterator			SGSchemeConstIterator;

	typedef map<String, ScriptTranslator*>::type	SGScriptTranslatorMap;
	typedef SGScriptTranslatorMap::iterator			SGScriptTranslatorIterator;
	typedef SGScriptTranslatorMap::const_iterator	SGScriptTranslatorConstIterator;


	
	/** Shader generator pass wrapper class. */
	class _OgreRTSSExport SGPass : public RTShaderSystemAlloc
	{
	public:
		SGPass			(SGTechnique* parent, Pass* srcPass, Pass* dstPass);
		~SGPass			();
	
		/** Build the render state. */
		void			buildTargetRenderState			();

		/** Acquire the CPU/GPU programs for this pass. */
		void			acquirePrograms			();

		/** Release the CPU/GPU programs of this pass. */
		void			releasePrograms			();


		/** Called when a single object is about to be rendered. */
		void			notifyRenderSingleObject	(Renderable* rend, const AutoParamDataSource* source, const LightList* pLightList, bool suppressRenderStateChanges);

		/** Get source pass. */
		Pass*			getSrcPass					() { return mSrcPass; }

		/** Get destination pass. */
		Pass*			getDstPass					() { return mDstPass; }

		/** Get custom FPP sub state of this pass. */
		SubRenderState*	getCustomFFPSubState		(int subStateOrder);

		/** Get custom render state of this pass. */
		RenderState*	getCustomRenderState		() { return mCustomRenderState; }

		/** Set the custom render state of this pass. */
		void			setCustomRenderState		(RenderState* customRenderState) { mCustomRenderState = customRenderState; }

		static String	UserKey;					// Key name for associating with a Pass instance.
	
	protected:
		SubRenderState*	getCustomFFPSubState		(int subStateOrder, const RenderState* renderState);

	protected:
		SGTechnique*			mParent;				// Parent technique.
		Pass*					mSrcPass;				// Source pass.
		Pass*					mDstPass;				// Destination pass.
		RenderState*			mCustomRenderState;		// Custom render state.
		TargetRenderState*		mTargetRenderState;	// The compiled render state.		
	};

	
	/** Shader generator technique wrapper class. */
	class _OgreRTSSExport SGTechnique : public RTShaderSystemAlloc
	{
	public:
		SGTechnique			(SGMaterial* parent, Technique* srcTechnique, const String& dstTechniqueSchemeName);		
		~SGTechnique		();
		
		/** Get the parent SGMaterial */
		const SGMaterial*	getParent						() const { return mParent; }
		
		/** Get the source technique. */
		Technique*			getSourceTechnique				() { return mSrcTechnique; }

		/** Get the destination technique. */
		Technique*			getDestinationTechnique			() { return mDstTechnique; }

		/** Get the destination technique scheme name. */
		const String&		getDestinationTechniqueSchemeName() const { return mDstTechniqueSchemeName; }
		
		/** Build the render state. */
		void				buildTargetRenderState				();

		/** Acquire the CPU/GPU programs for this technique. */
		void				acquirePrograms				();

		/** Release the CPU/GPU programs of this technique. */
		void				releasePrograms				();

		/** Tells the technique that it needs to generate shader code. */
		void				setBuildDestinationTechnique	(bool buildTechnique)	{ mBuildDstTechnique = buildTechnique; }		

		/** Tells if the destination technique should be build. */
		bool				getBuildDestinationTechnique	() const				{ return mBuildDstTechnique; }

		/** Get render state of specific pass.
		@param passIndex The pass index.
		*/
		RenderState*		getRenderState					(unsigned short passIndex);

		static String	UserKey;					// Key name for associating with a Technique instance.

	protected:
		
		/** Create the passes entries. */
		void				createSGPasses			();

		/** Destroy the passes entries. */
		void				destroySGPasses			();

		
	protected:
		SGMaterial*				mParent;					// Parent material.		
		Technique*				mSrcTechnique;				// Source technique.
		Technique*				mDstTechnique;				// Destination technique.
		SGPassList				mPassEntries;				// All passes entries.
		RenderStateList			mCustomRenderStates;		// The custom render states of all passes.
		bool					mBuildDstTechnique;			// Flag that tells if destination technique should be build.		
		String					mDstTechniqueSchemeName;	// Scheme name of destination technique.
	};

	
	/** Shader generator material wrapper class. */
	class _OgreRTSSExport SGMaterial : public RTShaderSystemAlloc
	{	
	
	public:
		/** Class constructor. */
		SGMaterial(const String& materialName)					{ mName = materialName; }

		/** Get the material name. */
		const String& getMaterialName				() const	{ return mName; }
		
		/** Get the const techniques list of this material. */
		const SGTechniqueList&	getTechniqueList	() const	 { return mTechniqueEntires; }

		/** Get the techniques list of this material. */
		SGTechniqueList&	getTechniqueList		() 			 { return mTechniqueEntires; }
	
	protected:
		String				mName;					// The material name.
		SGTechniqueList		mTechniqueEntires;		// All passes entries.
	};

	
	/** Shader generator scheme class. */
	class _OgreRTSSExport SGScheme : public RTShaderSystemAlloc
	{	
	public:
		SGScheme		(const String& schemeName);
		~SGScheme		();	


		/** Return true if this scheme dose not contains any techniques.
		*/
		bool					empty				() const  { return mTechniqueEntires.empty(); }
		
		/** Invalidate the whole scheme.
		@see ShaderGenerator::invalidateScheme.
		*/
		void					invalidate				();

		/** Validate the whole scheme.
		@see ShaderGenerator::validateScheme.
		*/
		void					validate				();

		/** Invalidate specific material.
		@see ShaderGenerator::invalidateMaterial.
		*/
		void					invalidate				(const String& materialName);

		/** Validate specific material.
		@see ShaderGenerator::validateMaterial.
		*/
		bool					validate				(const String& materialName);
				
		/** Add a technique to current techniques list. */
		void					addTechniqueEntry		(SGTechnique* techEntry);

		/** Remove a technique from the current techniques list. */
		void					removeTechniqueEntry	(SGTechnique* techEntry);


		/** Get global render state of this scheme. 
		@see ShaderGenerator::getRenderState.
		*/
		RenderState*			getRenderState			();

		/** Get specific pass render state. 
		@see ShaderGenerator::getRenderState.
		*/
		RenderState*			getRenderState			(const String& materialName, unsigned short passIndex);

	protected:
		/** Synchronize the current light settings of this scheme with the current settings of the scene. */
		void					synchronizeWithLightSettings	();

		/** Synchronize the fog settings of this scheme with the current settings of the scene. */
		void					synchronizeWithFogSettings		();


	protected:
		String					mName;					// Scheme name.
		SGTechniqueList			mTechniqueEntires;		// Technique entries.
		bool					mOutOfDate;				// Tells if this scheme is out of date.
		RenderState*			mRenderState;			// The global render state of this scheme.
		FogMode					mFogMode;				// Current fog mode.
	};


// Protected types.
protected:
	
	/** Shader generator RenderObjectListener sub class. */
	class _OgreRTSSExport SGRenderObjectListener : public RenderObjectListener, public RTShaderSystemAlloc
	{
	public:
		SGRenderObjectListener(ShaderGenerator* owner)
		{
			mOwner = owner;
		}

		/** 
		Listener overridden function notify the shader generator when rendering single object.
		*/
		virtual void notifyRenderSingleObject(Renderable* rend, const Pass* pass,  
			const AutoParamDataSource* source, 
			const LightList* pLightList, bool suppressRenderStateChanges)
		{
			mOwner->notifyRenderSingleObject(rend, pass, source, pLightList, suppressRenderStateChanges);
		}

	protected:
		ShaderGenerator* mOwner;
	};

	/** Shader generator scene manager sub class. */
	class _OgreRTSSExport SGSceneManagerListener : public SceneManager::Listener, public RTShaderSystemAlloc
	{
	public:
		SGSceneManagerListener(ShaderGenerator* owner)
		{
			mOwner = owner;
		}

		/** 
		Listener overridden function notify the shader generator when finding visible objects process started.
		*/
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
		ShaderGenerator* mOwner;			// The shader generator instance.
	};

	/** Shader generator ScriptTranslatorManager sub class. */
	class _OgreRTSSExport SGScriptTranslatorManager : public ScriptTranslatorManager
	{
	public:
		SGScriptTranslatorManager(ShaderGenerator* owner)
		{
			mOwner = owner;
		}

		/// Returns the number of translators being managed
		virtual size_t getNumTranslators() const
		{
			return mOwner->getNumTranslators();
		}
		
		/// Returns a manager for the given object abstract node, or null if it is not supported
		virtual ScriptTranslator *getTranslator(const AbstractNodePtr& node)
		{
			return mOwner->getTranslator(node);
		}

	protected:
		ShaderGenerator* mOwner;		// The shader generator instance.
	};

	//-----------------------------------------------------------------------------
	typedef map<String, SubRenderStateFactory*>::type 		SubRenderStateFactoryMap;
	typedef SubRenderStateFactoryMap::iterator 				SubRenderStateFactoryIterator;
	typedef SubRenderStateFactoryMap::const_iterator		SubRenderStateFactoryConstIterator;

	//-----------------------------------------------------------------------------
	typedef map<String, SceneManager*>::type 				SceneManagerMap;
	typedef SceneManagerMap::iterator 						SceneManagerIterator;
	typedef SceneManagerMap::const_iterator					SceneManagerConstIterator;

protected:
	/** Class default constructor */
	ShaderGenerator		();

	/** Class destructor */
	~ShaderGenerator	();

	/** Initialize the shader generator instance. */
	bool				_initialize			();
	
	/** Finalize the shader generator instance. */
	void				_finalize			();

	/** Find source technique to generate shader based technique based on it. */
	Technique*			findSourceTechnique				(const String& materialName, const String& srcTechniqueSchemeName);

	/** Called from the sub class of the RenderObjectLister when single object is rendered. */
	void				notifyRenderSingleObject		(Renderable* rend, const Pass* pass,  const AutoParamDataSource* source, const LightList* pLightList, bool suppressRenderStateChanges);

	/** Called from the sub class of the SceneManager::Listener when finding visible object process starts. */
	void				preFindVisibleObjects			(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v);

	/** Create sub render state core extensions factories */
	void				createSubRenderStateExFactories			();

	/** Destroy sub render state core extensions factories */
	void				destroySubRenderStateExFactories		();

	/** Create an instance of the SubRenderState based on script properties using the
	current sub render state factories.
	@see SubRenderStateFactory::createInstance	
	@param compiler The compiler instance.
	@param prop The abstract property node.
	@param pass The pass that is the parent context of this node.
	@param the translator for the specific SubRenderState
	*/
	SubRenderState*		createSubRenderState				(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator);
	
	/** Create an instance of the SubRenderState based on script properties using the
	current sub render state factories.
	@see SubRenderStateFactory::createInstance	
	@param compiler The compiler instance.
	@param prop The abstract property node.
	@param texState The texture unit state that is the parent context of this node.
	@param the translator for the specific SubRenderState
	*/
	SubRenderState*		createSubRenderState				(ScriptCompiler* compiler, PropertyAbstractNode* prop, TextureUnitState* texState, SGScriptTranslator* translator);

	/** 
	Add custom script translator. 
	Return true upon success.
	@param key The key name of the given translator.
	@param translator The translator to associate with the given key.
	*/
	bool				addCustomScriptTranslator			(const String& key, ScriptTranslator* translator);

	/** 
	Remove custom script translator. 
	Return true upon success.
	@param key The key name of the translator to remove.	
	*/
	bool				removeCustomScriptTranslator		(const String& key);

	/** Return number of script translators. */
	size_t				getNumTranslators					() const;

	/** Return a matching script translator. */
	ScriptTranslator*	getTranslator						(const AbstractNodePtr& node);


	/** This method called by instance of SGMaterialSerializerListener and 
	serialize a given pass entry attributes.
	@param ser The material serializer.
	@param passEntry The SGPass instance.
	*/
	void				serializePassAttributes				(MaterialSerializer* ser, SGPass* passEntry);

	/** This method called by instance of SGMaterialSerializerListener and 
	serialize a given textureUnitState entry attributes.
	@param ser The material serializer.
	@param passEntry The SGPass instance.
	@param srcTextureUnit The TextureUnitState being serialized.
	*/
	void serializeTextureUnitStateAttributes(MaterialSerializer* ser, SGPass* passEntry, const TextureUnitState* srcTextureUnit);


protected:	
	OGRE_AUTO_MUTEX													// Auto mutex.
	SceneManager*					mActiveSceneMgr;				// The active scene manager.
	SceneManagerMap					mSceneManagerMap;				// A map of all scene managers this generator is bound to.
	SGRenderObjectListener*			mRenderObjectListener;			// Render object listener.
	SGSceneManagerListener*			mSceneManagerListener;			// Scene manager listener.
	SGScriptTranslatorManager*		mScriptTranslatorManager;		// Script translator manager.
	SGMaterialSerializerListener*	mMaterialSerializerListener;	// Custom material Serializer listener - allows exporting material that contains shader generated techniques.
	SGScriptTranslatorMap			mScriptTranslatorsMap;			// A map of the registered custom script translators.
	SGScriptTranslator				mCoreScriptTranslator;			// The core translator of the RT Shader System.
	String							mShaderLanguage;				// The target shader language (currently only cg supported).
	String							mVertexShaderProfiles;			// The target vertex shader profile. Will be used as argument for program compilation.
	StringVector					mVertexShaderProfilesList;		// List of target vertex shader profiles.
	String							mFragmentShaderProfiles;		// The target fragment shader profile. Will be used as argument for program compilation.
	StringVector					mFragmentShaderProfilesList;	// List of target fragment shader profiles..
	String							mShaderCachePath;				// Path for caching the generated shaders.
	ProgramManager*					mProgramManager;				// Shader program manager.
	ProgramWriterManager*			mProgramWriterManager;			// Shader program writer manager.
	FFPRenderStateBuilder*			mFFPRenderStateBuilder;			// Fixed Function Render state builder.
	SGMaterialMap					mMaterialEntriesMap;			// Material entries map.
	SGSchemeMap						mSchemeEntriesMap;				// Scheme entries map.
	SGTechniqueMap					mTechniqueEntriesMap;			// All technique entries map.
	SubRenderStateFactoryMap		mSubRenderStateFactories;		// Sub render state registered factories.
	SubRenderStateFactoryMap		mSubRenderStateExFactories;		// Sub render state core extension factories.
	bool							mActiveViewportValid;			// True if active view port use a valid SGScheme.
	int								mLightCount[3];					// Light count per light type.
	VSOutputCompactPolicy			mVSOutputCompactPolicy;			// Vertex shader outputs compact policy.
	
private:
	friend class SGPass;
	friend class FFPRenderStateBuilder;
	friend class SGScriptTranslatorManager;
	friend class SGScriptTranslator;
	friend class SGMaterialSerializerListener;
	
};

/** @} */
/** @} */

}
}

#endif

