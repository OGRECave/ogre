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
#ifndef _ShaderGenerator_
#define _ShaderGenerator_

#include "OgreShaderPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreFileSystemLayer.h"
#include "OgreRenderObjectListener.h"
#include "OgreSceneManager.h"
#include "OgreShaderRenderState.h"
#include "OgreScriptTranslator.h"
#include "OgreShaderScriptTranslator.h"
#include "OgreMaterialSerializer.h"


namespace Ogre {


namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

class SGRenderObjectListener;
class SGSceneManagerListener;
class SGScriptTranslatorManager;
class SGResourceGroupListener;
class SGMaterialSerializerListener;
class ProgramManager;

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
    */
    static bool initialize();

    /** 
    Destroy the Shader Generator instance.
    */
    static void destroy();


    /** Override standard Singleton retrieval.

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
    static ShaderGenerator& getSingleton(); 


    /// @copydoc Singleton::getSingleton()
    static ShaderGenerator* getSingletonPtr();

    /** 
    Add a scene manager to the shader generator scene managers list.
    @param sceneMgr The scene manager to add to the list.
    */
    void addSceneManager(SceneManager* sceneMgr);

    /** 
    Remove a scene manager from the shader generator scene managers list.
    @param sceneMgr The scene manager to remove from the list.
    */
    void removeSceneManager(SceneManager* sceneMgr);

    /** 
    Get the active scene manager that is doint the actual scene rendering.
    This attribute will be update on the call to preFindVisibleObjects. 
    */
    SceneManager* getActiveSceneManager();

    /** 
    Set the active scene manager against which new render states are compiled.
    Note that normally the setting of the active scene manager is updated through the
    preFindVisibleObjects method.
    */

    void _setActiveSceneManager(SceneManager* sceneManager);

    /** 
    Set the target shader language.
    @param shaderLanguage The output shader language to use.
    @remarks The default shader language is cg.
    */
    void setTargetLanguage(const String& shaderLanguage);

    /** 
    Return the target shader language currently in use.     
    */
    const String& getTargetLanguage() const { return mShaderLanguage; }

    /** 
    Set the output shader target profiles.
    @param type shader type
    @param shaderProfiles The target profiles for the shader.
    */
    void setShaderProfiles(GpuProgramType type, const String& shaderProfiles);

    /** 
    Get the output shader target profiles.
    */
    const String& getShaderProfiles(GpuProgramType type) const;

    /** 
    Set the output shader cache path. Generated shader code will be written to this path.
    In case of empty cache path shaders will be generated directly from system memory.
    @param cachePath The cache path of the shader.  
    The default is empty cache path.
    */
    void setShaderCachePath(const String& cachePath);

    /** 
    Get the output shader cache path.
    */
    const String& getShaderCachePath() const { return mShaderCachePath; }

    /** 
    Flush the shader cache. This operation will cause all active schemes to be invalidated and will
    destroy any CPU/GPU program that created by this shader generator.
    */
    void flushShaderCache();

    /** 
    Return a global render state associated with the given scheme name.
    Modifying this render state will affect all techniques that belongs to that scheme.
    This is the best way to apply global changes to all techniques.
    After altering the render state one should call invalidateScheme method in order to 
    regenerate shaders.
    @param schemeName The destination scheme name.
    */
    RenderState* getRenderState(const String& schemeName);


    typedef std::pair<RenderState*, bool> RenderStateCreateOrRetrieveResult;
    /** 
    Returns a requested render state. If the render state does not exist this function creates it.
    @param schemeName The scheme name to retrieve.
    */
    RenderStateCreateOrRetrieveResult createOrRetrieveRenderState(const String& schemeName);


    /** 
    Tells if a given render state exists
    @param schemeName The scheme name to check.
    */
    bool hasRenderState(const String& schemeName) const;
    
    /**
     Get render state of specific pass.
     Using this method allows the user to customize the behavior of a specific pass.
     @param schemeName The destination scheme name.
     @param materialName The specific material name.
     @param groupName The specific material name.
     @param passIndex The pass index.
     */
    RenderState* getRenderState(const String& schemeName, const String& materialName, const String& groupName, unsigned short passIndex);

    /// @overload
    RenderState* getRenderState(const String& schemeName, const Material& mat, uint16 passIndex = 0)
    {
        return getRenderState(schemeName, mat.getName(), mat.getGroup(), passIndex);
    }

    /** 
    Add sub render state factory. Plugins or 3d party applications may implement sub classes of
    SubRenderState interface. Add the matching factory will allow the application to create instances 
    of these sub classes.
    @param factory The factory to add.
    */
    void addSubRenderStateFactory(SubRenderStateFactory* factory);

    /** 
    Returns the number of existing factories
    */
    size_t getNumSubRenderStateFactories() const;

    /** 
    Returns a sub render state factory by index
    @note index must be lower than the value returned by getNumSubRenderStateFactories()
    */
    SubRenderStateFactory* getSubRenderStateFactory(size_t index);

    /** 
    Returns a sub render state factory by name
    */
    SubRenderStateFactory* getSubRenderStateFactory(const String& type);

    /** 
    Remove sub render state factory. 
    @param factory The factory to remove.
    */
    void removeSubRenderStateFactory(SubRenderStateFactory* factory);

    /** 
    Create an instance of sub render state from a given type. 
    @param type The type of sub render state to create.
    */
    SubRenderState* createSubRenderState(const String& type);

    /// @overload
    template<typename T>
    T* createSubRenderState()
    {
        return static_cast<T*>(createSubRenderState(T::Type));
    }

    /** 
    Destroy an instance of sub render state. 
    @param subRenderState The instance to destroy.
    */
    void destroySubRenderState(SubRenderState* subRenderState);

    /**
     Checks if a shader based technique has been created for a given technique.
     Return true if exist. False if not.
     @param materialName The source material name.
     @param groupName The source group name.
     @param srcTechniqueSchemeName The source technique scheme name.
     @param dstTechniqueSchemeName The destination shader based technique scheme name.
     */
    bool hasShaderBasedTechnique(const String& materialName, const String& groupName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName) const;

    /// @overload
    bool hasShaderBasedTechnique(const Material& mat, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName) const
    {
        return hasShaderBasedTechnique(mat.getName(), mat.getGroup(), srcTechniqueSchemeName, dstTechniqueSchemeName);
    }

    /**
    Create shader based technique from a given technique.
    Return true upon success. Failure may occur if the source technique is not FFP pure, or different
    source technique is mapped to the requested destination scheme.
    @param srcMat The source material.
    @param srcTechniqueSchemeName The source technique scheme name.
    @param dstTechniqueSchemeName The destination shader based technique scheme name.
    @param overProgrammable If true a shader will be created even if the pass already has shaders
    */
    bool createShaderBasedTechnique(const Material& srcMat, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName, bool overProgrammable = false);

    /// @overload
    bool createShaderBasedTechnique(const Technique* srcTech, const String& dstTechniqueSchemeName, bool overProgrammable = false);

    /**
     Remove shader based technique from a given technique.
     Return true upon success. Failure may occur if the given source technique was not previously
     registered successfully using the createShaderBasedTechnique method.
     @param srcTech The source technique.
     @param dstTechniqueSchemeName The destination shader based technique scheme name.
     */
    bool removeShaderBasedTechnique(const Technique* srcTech, const String& dstTechniqueSchemeName);

    /** 
    Remove all shader based techniques of the given material. 
    Return true upon success.
    @param materialName The source material name.   
    @param groupName The source group name. 
    */
    bool removeAllShaderBasedTechniques(const String& materialName, const String& groupName OGRE_RESOURCE_GROUP_INIT);

    /// @overload
    bool removeAllShaderBasedTechniques(const Material& mat)
    {
        return removeAllShaderBasedTechniques(mat.getName(), mat.getGroup());
    }

    /** 
    Clone all shader based techniques from one material to another.
    This function can be used in conjunction with the Material::clone() function to copy 
    both material properties and RTSS state from one material to another.
    @param srcMat The source material
    @param dstMat The destination material
    @return True if successful
    */
    bool cloneShaderBasedTechniques(Material& srcMat, Material& dstMat);

    /** 
    Remove all shader based techniques that created by this shader generator.   
    */
    void removeAllShaderBasedTechniques();

    /** 
    Create a scheme.
    @param schemeName The scheme name to create.
    */
    void createScheme(const String& schemeName);

    /** 
    Invalidate a given scheme. This action will lead to shader regeneration of all techniques belongs to the
    given scheme name.
    @param schemeName The scheme to invalidate.
    */
    void invalidateScheme(const String& schemeName);

    /** 
    Validate a given scheme. This action will generate shader programs for all techniques of the
    given scheme name.
    @param schemeName The scheme to validate.
    */
    bool validateScheme(const String& schemeName);
    
    /** 
    Invalidate specific material scheme. This action will lead to shader regeneration of the technique belongs to the
    given scheme name.
    @param schemeName The scheme to invalidate.
    @param materialName The material to invalidate.
    @param groupName The source group name. 
    */
    void invalidateMaterial(const String& schemeName, const String& materialName, const String& groupName OGRE_RESOURCE_GROUP_INIT);

    /// @overload
    void invalidateMaterial(const String& schemeName, const Material& mat)
    {
        invalidateMaterial(schemeName, mat.getName(), mat.getGroup());
    }

    /** 
    Validate specific material scheme. This action will generate shader programs for the technique of the
    given scheme name.
    @param schemeName The scheme to validate.
    @param materialName The material to validate.
    @param groupName The source group name. 
    */
    bool validateMaterial(const String& schemeName, const String& materialName, const String& groupName OGRE_RESOURCE_GROUP_INIT);

    /// @overload
    void validateMaterial(const String& schemeName, const Material& mat)
    {
        validateMaterial(schemeName, mat.getName(), mat.getGroup());
    }

	/**
	Invalidate specific material scheme. This action will lead to shader regeneration of the technique belongs to the
	given scheme name.
	@param schemeName The scheme to invalidate.
	@param materialName The material to invalidate.
	@param groupName The source group name.
	*/
    void invalidateMaterialIlluminationPasses(const String& schemeName, const String& materialName, const String& groupName OGRE_RESOURCE_GROUP_INIT);

	/**
	Validate specific material scheme. This action will generate shader programs illumination passes of the technique of the
	given scheme name.
	@param schemeName The scheme to validate.
	@param materialName The material to validate.
	@param groupName The source group name.
	*/
	bool validateMaterialIlluminationPasses(const String& schemeName, const String& materialName, const String& groupName OGRE_RESOURCE_GROUP_INIT);

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
    MaterialSerializer::Listener* getMaterialSerializerListener();

    /** Return the current number of generated shaders. */
    size_t getShaderCount(GpuProgramType type) const;

    /** Set the vertex shader outputs compaction policy. 
    @see VSOutputCompactPolicy.
    @param policy The policy to set.
    */
    void setVertexShaderOutputsCompactPolicy(VSOutputCompactPolicy policy)  { mVSOutputCompactPolicy = policy; }
    
    /** Get the vertex shader outputs compaction policy. 
    @see VSOutputCompactPolicy. 
    */
    VSOutputCompactPolicy getVertexShaderOutputsCompactPolicy() const { return mVSOutputCompactPolicy; }


    /** Sets whether shaders are created for passes with shaders.
    Note that this only refers to when the system parses the materials itself.
    Not for when calling the createShaderBasedTechnique() function directly
    @param value The value to set this attribute pass.  
    */
    void setCreateShaderOverProgrammablePass(bool value) { mCreateShaderOverProgrammablePass = value; }

    /** Returns whether shaders are created for passes with shaders.
    @see setCreateShaderOverProgrammablePass(). 
    */
    bool getCreateShaderOverProgrammablePass() const { return mCreateShaderOverProgrammablePass; }


    /** Returns the amount of schemes used in the for RT shader generation
    */
    size_t getRTShaderSchemeCount() const;

    /** Returns the scheme name used in the for RT shader generation by index
    */
    const String& getRTShaderScheme(size_t index) const;

    /// mark the given texture unit as non-FFP
    static void _markNonFFP(const TextureUnitState* tu);

    /// same as @ref MSN_SHADERGEN
    static String DEFAULT_SCHEME_NAME;
private:
    class SGPass;
    class SGTechnique;
    class SGMaterial;
    class SGScheme;

    typedef std::pair<String,String>                MatGroupPair;
    struct MatGroupPair_less
    {
        // ensure we arrange the list first by material name then by group name
        bool operator()(const MatGroupPair& p1, const MatGroupPair& p2) const
        {
            int cmpVal = strcmp(p1.first.c_str(),p2.first.c_str());
            return (cmpVal < 0) || ((cmpVal == 0) && (strcmp(p1.second.c_str(),p2.second.c_str()) < 0));
        }
    };

    typedef std::vector<SGPass*>                   SGPassList;
    typedef SGPassList::iterator                        SGPassIterator;
    typedef SGPassList::const_iterator              SGPassConstIterator;

    typedef std::vector<SGTechnique*>              SGTechniqueList;
    typedef SGTechniqueList::iterator               SGTechniqueIterator;
    typedef SGTechniqueList::const_iterator         SGTechniqueConstIterator;

    typedef std::map<SGTechnique*, SGTechnique*>   SGTechniqueMap;
    typedef SGTechniqueMap::iterator                    SGTechniqueMapIterator;
    
    typedef std::map<MatGroupPair, SGMaterial*, MatGroupPair_less> SGMaterialMap;
    typedef SGMaterialMap::iterator                 SGMaterialIterator;
    typedef SGMaterialMap::const_iterator           SGMaterialConstIterator;

    typedef std::map<String, SGScheme*>                SGSchemeMap;
    typedef SGSchemeMap::iterator                   SGSchemeIterator;
    typedef SGSchemeMap::const_iterator             SGSchemeConstIterator;

    typedef std::map<uint32, ScriptTranslator*>        SGScriptTranslatorMap;
    typedef SGScriptTranslatorMap::iterator         SGScriptTranslatorIterator;
    typedef SGScriptTranslatorMap::const_iterator   SGScriptTranslatorConstIterator;


    
    /** Shader generator pass wrapper class. */
    class _OgreRTSSExport SGPass : public RTShaderSystemAlloc
    {
    public:
		SGPass(SGTechnique* parent, Pass* srcPass, Pass* dstPass, IlluminationStage stage);
        ~SGPass();
    
        /** Build the render state and acquire the CPU/GPU programs */
        void buildTargetRenderState();

        /** Get source pass. */
        Pass* getSrcPass() { return mSrcPass; }

        /** Get destination pass. */
        Pass* getDstPass() { return mDstPass; }

		/** Get illumination stage. */
		IlluminationStage getIlluminationStage() { return mStage; }

		/** Get illumination state. */
		bool isIlluminationPass() { return mStage != IS_UNKNOWN; }

        /** Get custom render state of this pass. */
        RenderState* getCustomRenderState() { return mCustomRenderState; }

        /** Set the custom render state of this pass. */
        void setCustomRenderState(RenderState* customRenderState) { mCustomRenderState = customRenderState; }

        const SGTechnique* getParent() const { return mParent; }
    protected:
        // Parent technique.
        SGTechnique* mParent;
        // Source pass.
        Pass* mSrcPass;
        // Destination pass.
        Pass* mDstPass;
		// Illumination stage
		IlluminationStage mStage;
        // Custom render state.
        RenderState* mCustomRenderState;
    };

    
    /** Shader generator technique wrapper class. */
    class _OgreRTSSExport SGTechnique : public RTShaderSystemAlloc
    {
    public:
        SGTechnique(SGMaterial* parent, const Technique* srcTechnique,
                    const String& dstTechniqueSchemeName, bool overProgrammable);
        ~SGTechnique();
        
        /** Get the parent SGMaterial */
        const SGMaterial* getParent() const { return mParent; }
        
        /** Get the source technique. */
        const Technique* getSourceTechnique() { return mSrcTechnique; }

        /** Get the destination technique. */
        Technique* getDestinationTechnique() { return mDstTechnique; }

        /** Get the destination technique scheme name. */
        const String& getDestinationTechniqueSchemeName() const { return mDstTechniqueSchemeName; }
        
        /** Build the render state. */
        void buildTargetRenderState();

		/** Build the render state for illumination passes. */
		void buildIlluminationTargetRenderState();

		/** Destroy the illumination passes entries. */
		void destroyIlluminationSGPasses();

        /** Release the CPU/GPU programs of this technique. */
        void releasePrograms();

        /** Tells the technique that it needs to generate shader code. */
        void setBuildDestinationTechnique(bool buildTechnique)  { mBuildDstTechnique = buildTechnique; }        

        /** Tells if the destination technique should be build. */
        bool getBuildDestinationTechnique() const               { return mBuildDstTechnique; }

        /** Get render state of specific pass.
        @param passIndex The pass index.
        */
        RenderState* getRenderState(unsigned short passIndex);
        /** Tells if a custom render state exists for the given pass. */
        bool hasRenderState(unsigned short passIndex);

        /// whether shaders are created for passes with shaders
        bool overProgrammablePass() { return mOverProgrammable; }

        const SGPassList& getPassList() const  { return mPassEntries; }

        // Key name for associating with a Technique instance.
        static String UserKey;

    protected:
        
        /** Create the passes entries. */
        void createSGPasses();

		/** Create the illumination passes entries. */
		void createIlluminationSGPasses();

        /** Destroy the passes entries. */
        void destroySGPasses();
        
    protected:
        // Auto mutex.
        OGRE_AUTO_MUTEX;
        // Parent material.     
        SGMaterial* mParent;
        // Source technique.
        const Technique* mSrcTechnique;
        // Destination technique.
        Technique* mDstTechnique;
		// All passes entries, both normal and illumination.
        SGPassList mPassEntries;
        // The custom render states of all passes.
        typedef std::vector<RenderState*> RenderStateList;
        RenderStateList mCustomRenderStates;
        // Flag that tells if destination technique should be build.        
        bool mBuildDstTechnique;
        // Scheme name of destination technique.
        String mDstTechniqueSchemeName;
        bool mOverProgrammable;
    };

    
    /** Shader generator material wrapper class. */
    class _OgreRTSSExport SGMaterial : public RTShaderSystemAlloc
    {   
    
    public:
        /** Class constructor. */
        SGMaterial(const String& materialName, const String& groupName) : mName(materialName), mGroup(groupName) 
        {}

        /** Get the material name. */
        const String& getMaterialName() const   { return mName; }
        
        /** Get the group name. */
        const String& getGroupName() const  { return mGroup; }

        /** Get the const techniques list of this material. */
        const SGTechniqueList& getTechniqueList() const  { return mTechniqueEntries; }

        /** Get the techniques list of this material. */
        SGTechniqueList& getTechniqueList()              { return mTechniqueEntries; }
    
    protected:
        // The material name.
        String mName;
        // The group name.
        String mGroup;
        // All passes entries.
        SGTechniqueList mTechniqueEntries;
    };

    
    /** Shader generator scheme class. */
    class _OgreRTSSExport SGScheme : public RTShaderSystemAlloc
    {   
    public:
        SGScheme(const String& schemeName);
        ~SGScheme();    


        /** Return true if this scheme dose not contains any techniques.
        */
        bool empty() const  { return mTechniqueEntries.empty(); }
        
        /** Invalidate the whole scheme.
        @see ShaderGenerator::invalidateScheme.
        */
        void invalidate();

        /** Validate the whole scheme.
        @see ShaderGenerator::validateScheme.
        */
        void validate();

        /** Invalidate specific material.
        @see ShaderGenerator::invalidateMaterial.
        */
        void invalidate(const String& materialName, const String& groupName);

        /** Validate specific material.
        @see ShaderGenerator::validateMaterial.
        */
        bool validate(const String& materialName, const String& groupName);

		/** Validate illumination passes of the specific material.
		@see ShaderGenerator::invalidateMaterialIlluminationPasses.
		*/
        void invalidateIlluminationPasses(const String& materialName, const String& groupName);

		/** Validate illumination passes of the specific material.
		@see ShaderGenerator::validateMaterialIlluminationPasses.
		*/
        bool validateIlluminationPasses(const String& materialName, const String& groupName);

        /** Add a technique to current techniques list. */
        void addTechniqueEntry(SGTechnique* techEntry);

        /** Remove a technique from the current techniques list. */
        void removeTechniqueEntry(SGTechnique* techEntry);


        /** Get global render state of this scheme. 
        @see ShaderGenerator::getRenderState.
        */
        RenderState* getRenderState();

        /** Get specific pass render state. 
        @see ShaderGenerator::getRenderState.
        */
        RenderState* getRenderState(const String& materialName, const String& groupName, unsigned short passIndex);

    protected:
        /** Synchronize the current light settings of this scheme with the current settings of the scene. */
        void synchronizeWithLightSettings();

        /** Synchronize the fog settings of this scheme with the current settings of the scene. */
        void synchronizeWithFogSettings();


    protected:
        // Scheme name.
        String mName;
        // Technique entries.
        SGTechniqueList mTechniqueEntries;
        // Tells if this scheme is out of date.
        bool mOutOfDate;
        // The global render state of this scheme.
        std::unique_ptr<RenderState> mRenderState;
        // Current fog mode.
        FogMode mFogMode;
    };

    //-----------------------------------------------------------------------------
    typedef std::map<String, SubRenderStateFactory*>       SubRenderStateFactoryMap;
    typedef SubRenderStateFactoryMap::iterator              SubRenderStateFactoryIterator;
    typedef SubRenderStateFactoryMap::const_iterator        SubRenderStateFactoryConstIterator;

    //-----------------------------------------------------------------------------
    typedef std::set<SceneManager*>                         SceneManagerMap;
    typedef SceneManagerMap::iterator                       SceneManagerIterator;
    typedef SceneManagerMap::const_iterator                 SceneManagerConstIterator;

    friend class SGRenderObjectListener;
    friend class SGSceneManagerListener;

    /** Class default constructor */
    ShaderGenerator();

    /** Class destructor */
    ~ShaderGenerator();

    /** Initialize the shader generator instance. */
    bool _initialize();
    
    /** Destory the shader generator instance. */
    void _destroy();
 
    /** Called from the sub class of the RenderObjectLister when single object is rendered. */
    void notifyRenderSingleObject(Renderable* rend, const Pass* pass,  const AutoParamDataSource* source, const LightList* pLightList, bool suppressRenderStateChanges);

    /** Called from the sub class of the SceneManager::Listener when finding visible object process starts. */
    void preFindVisibleObjects(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v);

    /** Create sub render state core extensions factories */
    void createBuiltinSRSFactories();

    /** Destroy sub render state core extensions factories */
    void destroyBuiltinSRSFactories();

    /** Create an instance of the SubRenderState based on script properties using the
    current sub render state factories.
    @see SubRenderStateFactory::createInstance  
    @param compiler The compiler instance.
    @param prop The abstract property node.
    @param pass The pass that is the parent context of this node.
    @param translator The translator for the specific SubRenderState
    */
    SubRenderState* createSubRenderState(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator);
    
    /** Create an instance of the SubRenderState based on script properties using the
    current sub render state factories.
    @see SubRenderStateFactory::createInstance  
    @param compiler The compiler instance.
    @param prop The abstract property node.
    @param texState The texture unit state that is the parent context of this node.
    @param translator The translator for the specific SubRenderState
    */
    SubRenderState* createSubRenderState(ScriptCompiler* compiler, PropertyAbstractNode* prop, TextureUnitState* texState, SGScriptTranslator* translator);

    /** Return a matching script translator. */
    ScriptTranslator* getTranslator(const AbstractNodePtr& node);

    /** Finds an entry iterator in the mMaterialEntriesMap map.
    This function is able to find materials with group specified as 
    AUTODETECT_RESOURCE_GROUP_NAME 
    */
    SGMaterialIterator findMaterialEntryIt(const String& materialName, const String& groupName);
    SGMaterialConstIterator findMaterialEntryIt(const String& materialName, const String& groupName) const;


    typedef std::pair<SGScheme*, bool> SchemeCreateOrRetrieveResult;
    /** 
    Returns a requested scheme. If the scheme does not exist this function creates it.
    @param schemeName The scheme name to retrieve.
    */
    SchemeCreateOrRetrieveResult createOrRetrieveScheme(const String& schemeName);

    /** Used to check if finalizing */
    bool getIsFinalizing() const;

    /** Internal method that creates list of SGPass instances composing the given material. */
    SGPassList createSGPassList(Material* mat) const;

    // Auto mutex.
    OGRE_AUTO_MUTEX;
    // The active scene manager.
    SceneManager* mActiveSceneMgr;
    // A map of all scene managers this generator is bound to.
    SceneManagerMap mSceneManagerMap;
    // Render object listener.
    std::unique_ptr<SGRenderObjectListener> mRenderObjectListener;
    // Scene manager listener.
    std::unique_ptr<SGSceneManagerListener> mSceneManagerListener;
    // Script translator manager.
    std::unique_ptr<SGScriptTranslatorManager> mScriptTranslatorManager;
    // Custom material Serializer listener - allows exporting material that contains shader generated techniques.
    std::unique_ptr<SGMaterialSerializerListener> mMaterialSerializerListener;
    // get notified if materials get dropped
    std::unique_ptr<SGResourceGroupListener> mResourceGroupListener;
    // The core translator of the RT Shader System.
    SGScriptTranslator mCoreScriptTranslator;
    // The target shader language (currently only cg supported).
    String mShaderLanguage;
    // The target vertex shader profile. Will be used as argument for program compilation.
    String mVertexShaderProfiles;
    // The target fragment shader profile. Will be used as argument for program compilation.
    String mFragmentShaderProfiles;
    // Path for caching the generated shaders.
    String mShaderCachePath;
    // Shader program manager.
    std::unique_ptr<ProgramManager> mProgramManager;
    // Shader program writer manager.
    std::unique_ptr<ProgramWriterManager> mProgramWriterManager;
    // File system layer manager.
    FileSystemLayer* mFSLayer;
    // Material entries map.
    SGMaterialMap mMaterialEntriesMap;
    // Scheme entries map.
    SGSchemeMap mSchemeEntriesMap;
    // All technique entries map.
    SGTechniqueMap mTechniqueEntriesMap;
    // Sub render state registered factories.
    SubRenderStateFactoryMap mSubRenderStateFactories;
    // Sub render state core extension factories.
    std::vector<SubRenderStateFactory*> mBuiltinSRSFactories;
    // True if active view port use a valid SGScheme.
    bool mActiveViewportValid;
    // Light count per light type.
    int mLightCount[3];
    // Vertex shader outputs compact policy.
    VSOutputCompactPolicy mVSOutputCompactPolicy;
    // Tells whether shaders are created for passes with shaders
    bool mCreateShaderOverProgrammablePass;
    // A flag to indicate finalizing
    bool mIsFinalizing;

    uint32 ID_RT_SHADER_SYSTEM;

    friend class SGPass;
    friend class SGScriptTranslatorManager;
    friend class SGScriptTranslator;
    friend class SGMaterialSerializerListener;
    
};

/** @} */
/** @} */

}
}

#endif

