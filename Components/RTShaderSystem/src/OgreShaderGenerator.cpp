/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

//-----------------------------------------------------------------------
template<>
RTShader::ShaderGenerator* Singleton<RTShader::ShaderGenerator>::msSingleton = 0;

namespace RTShader {

/** Shader generator RenderObjectListener sub class. */
class SGRenderObjectListener : public RenderObjectListener, public RTShaderSystemAlloc
{
public:
    SGRenderObjectListener(ShaderGenerator* owner) { mOwner = owner; }

    /**
    Listener overridden function notify the shader generator when rendering single object.
    */
    void notifyRenderSingleObject(Renderable* rend, const Pass* pass,
                                          const AutoParamDataSource* source, const LightList* pLightList,
                                          bool suppressRenderStateChanges) override
    {
        mOwner->notifyRenderSingleObject(rend, pass, source, pLightList, suppressRenderStateChanges);
    }

protected:
    ShaderGenerator* mOwner;
};

/** Shader generator scene manager sub class. */
class SGSceneManagerListener : public SceneManager::Listener, public RTShaderSystemAlloc
{
public:
    SGSceneManagerListener(ShaderGenerator* owner) { mOwner = owner; }

    /**
    Listener overridden function notify the shader generator when finding visible objects process started.
    */
    void preFindVisibleObjects(SceneManager* source, SceneManager::IlluminationRenderStage irs,
                                       Viewport* v) override
    {
        mOwner->preFindVisibleObjects(source, irs, v);
    }
protected:
    // The shader generator instance.
    ShaderGenerator* mOwner;
};

/** Shader generator ScriptTranslatorManager sub class. */
class SGScriptTranslatorManager : public ScriptTranslatorManager
{
public:
    SGScriptTranslatorManager(ShaderGenerator* owner) { mOwner = owner; }

    /// Returns a manager for the given object abstract node, or null if it is not supported
    ScriptTranslator* getTranslator(const AbstractNodePtr& node) override
    {
        return mOwner->getTranslator(node);
    }

protected:
    // The shader generator instance.
    ShaderGenerator* mOwner;
};

class SGResourceGroupListener : public ResourceGroupListener
{
public:
    SGResourceGroupListener(ShaderGenerator* owner) { mOwner = owner; }

    /// sync our internal list if material gets dropped
    void resourceRemove(const ResourcePtr& resource) override
    {
        if (auto mat = dynamic_cast<Material*>(resource.get()))
        {
            mOwner->removeAllShaderBasedTechniques(*mat);
        }
    }

protected:
    // The shader generator instance.
    ShaderGenerator* mOwner;
};

String ShaderGenerator::DEFAULT_SCHEME_NAME     = MSN_SHADERGEN;
String ShaderGenerator::SGTechnique::UserKey    = "SGTechnique";

//-----------------------------------------------------------------------
ShaderGenerator* ShaderGenerator::getSingletonPtr()
{
    return msSingleton;
}

//-----------------------------------------------------------------------
ShaderGenerator& ShaderGenerator::getSingleton()
{
    assert( msSingleton );
    return ( *msSingleton );
}

//-----------------------------------------------------------------------------
ShaderGenerator::ShaderGenerator() :
    mActiveSceneMgr(NULL), mShaderLanguage(""),
    mFSLayer(0), mActiveViewportValid(false), mVSOutputCompactPolicy(VSOCP_LOW),
    mCreateShaderOverProgrammablePass(false), mIsFinalizing(false)
{
    mLightCount[0]              = 0;
    mLightCount[1]              = 0;
    mLightCount[2]              = 0;

    HighLevelGpuProgramManager& hmgr = HighLevelGpuProgramManager::getSingleton();

    if (hmgr.isLanguageSupported("glsles"))
    {
        mShaderLanguage = "glsles";
    }
    else if (hmgr.isLanguageSupported("glsl"))
    {
        mShaderLanguage = "glsl";
    }
    else if (hmgr.isLanguageSupported("hlsl"))
    {
        mShaderLanguage = "hlsl";
    }
    else if (hmgr.isLanguageSupported("glslang"))
    {
        mShaderLanguage = "glslang";
    }
    else
    {
        mShaderLanguage = "null"; // falling back to HLSL, for unit tests mainly
        LogManager::getSingleton().logWarning("ShaderGenerator: No supported language found. Falling back to 'null'");
    }

    setShaderProfiles(GPT_VERTEX_PROGRAM, "vs_3_0 vs_2_a vs_2_0 vs_1_1");
    setShaderProfiles(GPT_FRAGMENT_PROGRAM, "ps_3_0 ps_2_a ps_2_b ps_2_0 ps_1_4 ps_1_3 ps_1_2 ps_1_1");
}

//-----------------------------------------------------------------------------
ShaderGenerator::~ShaderGenerator()
{

}

//-----------------------------------------------------------------------------
bool ShaderGenerator::initialize()
{
    if (msSingleton == NULL)
    {
        msSingleton = OGRE_NEW ShaderGenerator;
        if (false == msSingleton->_initialize())
        {
            OGRE_DELETE msSingleton;
            msSingleton = NULL;
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::_initialize()
{
    OGRE_LOCK_AUTO_MUTEX;

    // Allocate program writer manager.
    mProgramWriterManager.reset(new ProgramWriterManager);

    // Allocate program manager.
    mProgramManager.reset(new ProgramManager);

    // Create extensions factories.
    createBuiltinSRSFactories();

    // Allocate script translator manager.
    mScriptTranslatorManager.reset(new SGScriptTranslatorManager(this));
    ScriptCompilerManager::getSingleton().addTranslatorManager(mScriptTranslatorManager.get());
    ID_RT_SHADER_SYSTEM = ScriptCompilerManager::getSingleton().registerCustomWordId("rtshader_system");

    // Create the default scheme.
    createScheme(MSN_SHADERGEN);

	mResourceGroupListener.reset(new SGResourceGroupListener(this));
	ResourceGroupManager::getSingleton().addResourceGroupListener(mResourceGroupListener.get());

    return true;
}



//-----------------------------------------------------------------------------
void ShaderGenerator::createBuiltinSRSFactories()
{
    OGRE_LOCK_AUTO_MUTEX;
    SubRenderStateFactory* curFactory;
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
    curFactory = OGRE_NEW FFPTransformFactory;
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mBuiltinSRSFactories.push_back(curFactory);

    curFactory = OGRE_NEW FFPColourFactory;
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mBuiltinSRSFactories.push_back(curFactory);

    curFactory = OGRE_NEW FFPLightingFactory;
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mBuiltinSRSFactories.push_back(curFactory);

    curFactory = OGRE_NEW FFPTexturingFactory;
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mBuiltinSRSFactories.push_back(curFactory);

    curFactory = OGRE_NEW FFPFogFactory;
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mBuiltinSRSFactories.push_back(curFactory);

    curFactory = OGRE_NEW FFPAlphaTestFactory;
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mBuiltinSRSFactories.push_back(curFactory);
#endif
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
    // check if we are running an old shader level in d3d11
    bool d3d11AndLowProfile = ( (GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1") ||
        GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_3"))
        && !GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0"));
    if(!d3d11AndLowProfile)
    {
        curFactory = OGRE_NEW PerPixelLightingFactory;
        addSubRenderStateFactory(curFactory);
        mBuiltinSRSFactories.push_back(curFactory);

        curFactory = OGRE_NEW NormalMapLightingFactory;
        addSubRenderStateFactory(curFactory);
        mBuiltinSRSFactories.push_back(curFactory);

        curFactory = new CookTorranceLightingFactory;
        addSubRenderStateFactory(curFactory);
        mBuiltinSRSFactories.push_back(curFactory);

        curFactory = new ImageBasedLightingFactory;
        addSubRenderStateFactory(curFactory);
        mBuiltinSRSFactories.push_back(curFactory);

        curFactory = OGRE_NEW IntegratedPSSM3Factory;
        addSubRenderStateFactory(curFactory);
        mBuiltinSRSFactories.push_back(curFactory);

        curFactory = OGRE_NEW LayeredBlendingFactory;
        addSubRenderStateFactory(curFactory);
        mBuiltinSRSFactories.push_back(curFactory);

        curFactory = OGRE_NEW HardwareSkinningFactory;
        addSubRenderStateFactory(curFactory);
        mBuiltinSRSFactories.push_back(curFactory);
    }

    curFactory = OGRE_NEW TriplanarTexturingFactory;
    addSubRenderStateFactory(curFactory);
    mBuiltinSRSFactories.push_back(curFactory);

    curFactory = OGRE_NEW GBufferFactory;
    addSubRenderStateFactory(curFactory);
    mBuiltinSRSFactories.push_back(curFactory);

    curFactory = OGRE_NEW WBOITFactory;
    addSubRenderStateFactory(curFactory);
    mBuiltinSRSFactories.push_back(curFactory);
#endif
}

//-----------------------------------------------------------------------------
void ShaderGenerator::destroy()
{
    if (msSingleton != NULL)
    {
        msSingleton->_destroy();

        OGRE_DELETE msSingleton;
        msSingleton = NULL;
    }
}

//-----------------------------------------------------------------------------
void ShaderGenerator::_destroy()
{
    OGRE_LOCK_AUTO_MUTEX;

    mIsFinalizing = true;

    // Delete technique entries.
    for (auto& e : mTechniqueEntriesMap)
    {
        OGRE_DELETE (e.second);
    }
    mTechniqueEntriesMap.clear();

    // Delete material entries.
    for (auto& m : mMaterialEntriesMap)
    {
        OGRE_DELETE (m.second);
    }
    mMaterialEntriesMap.clear();

    // Delete scheme entries.
    for (auto& s : mSchemeEntriesMap)
    {
        OGRE_DELETE (s.second);
    }
    mSchemeEntriesMap.clear();

    // Destroy extensions factories.
    destroyBuiltinSRSFactories();

    mProgramManager.reset();
    mProgramWriterManager.reset();

    // Delete script translator manager.
    if (mScriptTranslatorManager)
    {
        ScriptCompilerManager::getSingleton().removeTranslatorManager(mScriptTranslatorManager.get());
        mScriptTranslatorManager.reset();
    }

    mMaterialSerializerListener.reset();

    if (mResourceGroupListener)
    {
        ResourceGroupManager::getSingleton().removeResourceGroupListener(mResourceGroupListener.get());
        mResourceGroupListener.reset();
    }

    // Remove all scene managers.
    while (mSceneManagerMap.empty() == false)
    {
        removeSceneManager(*mSceneManagerMap.begin());
    }

    mRenderObjectListener.reset();
    mSceneManagerListener.reset();
}

//-----------------------------------------------------------------------------
void ShaderGenerator::destroyBuiltinSRSFactories()
{
    OGRE_LOCK_AUTO_MUTEX;

    for (auto f : mBuiltinSRSFactories)
    {
        removeSubRenderStateFactory(f);
        delete f;
    }
    mBuiltinSRSFactories.clear();
}

//-----------------------------------------------------------------------------
void ShaderGenerator::addSubRenderStateFactory(SubRenderStateFactory* factory)
{
    OGRE_LOCK_AUTO_MUTEX;

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
size_t ShaderGenerator::getNumSubRenderStateFactories() const
{
    return mSubRenderStateFactories.size();
}


//-----------------------------------------------------------------------------
SubRenderStateFactory*  ShaderGenerator::getSubRenderStateFactory(size_t index)
{
    {
        OGRE_LOCK_AUTO_MUTEX;

        SubRenderStateFactoryIterator itFind = mSubRenderStateFactories.begin();
        for(; index != 0 && itFind != mSubRenderStateFactories.end(); --index , ++itFind);

        if (itFind != mSubRenderStateFactories.end())
        {
            return itFind->second;
        }
    }

    OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
        "A factory on index " + StringConverter::toString(index) + " does not exist.",
        "ShaderGenerator::addSubRenderStateFactory");

    return NULL;
}
//-----------------------------------------------------------------------------
SubRenderStateFactory* ShaderGenerator::getSubRenderStateFactory(const String& type)
{
    OGRE_LOCK_AUTO_MUTEX;

    SubRenderStateFactoryIterator itFind = mSubRenderStateFactories.find(type);
    return (itFind != mSubRenderStateFactories.end()) ? itFind->second : NULL;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::removeSubRenderStateFactory(SubRenderStateFactory* factory)
{
    OGRE_LOCK_AUTO_MUTEX;

    SubRenderStateFactoryIterator itFind = mSubRenderStateFactories.find(factory->getType());

    if (itFind != mSubRenderStateFactories.end())
        mSubRenderStateFactories.erase(itFind);

}

//-----------------------------------------------------------------------------
SubRenderState* ShaderGenerator::createSubRenderState(const String& type)
{
    OGRE_LOCK_AUTO_MUTEX;

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
    OGRE_LOCK_AUTO_MUTEX;

    SubRenderStateFactoryIterator itFind = mSubRenderStateFactories.find(subRenderState->getType());

    if (itFind != mSubRenderStateFactories.end())
    {
         itFind->second->destroyInstance(subRenderState);
    }
}

//-----------------------------------------------------------------------------
SubRenderState* ShaderGenerator::createSubRenderState(ScriptCompiler* compiler,
                                                      PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    OGRE_LOCK_AUTO_MUTEX;
    SubRenderState* subRenderState = NULL;

    for (auto& s : mSubRenderStateFactories)
    {
        subRenderState = s.second->createInstance(compiler, prop, pass, translator);
        if (subRenderState != NULL)
            break;
    }

    return subRenderState;
}


//-----------------------------------------------------------------------------
SubRenderState* ShaderGenerator::createSubRenderState(ScriptCompiler* compiler,
                                                      PropertyAbstractNode* prop, TextureUnitState* texState, SGScriptTranslator* translator)
{
    OGRE_LOCK_AUTO_MUTEX;
    SubRenderState* subRenderState = NULL;

    for (auto& s : mSubRenderStateFactories)
    {
        subRenderState = s.second->createInstance(compiler, prop, texState, translator);
        if (subRenderState != NULL)
            break;
    }

    return subRenderState;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::createScheme(const String& schemeName)
{
    createOrRetrieveScheme(schemeName);
}

//-----------------------------------------------------------------------------
RenderState* ShaderGenerator::getRenderState(const String& schemeName)
{
    OGRE_LOCK_AUTO_MUTEX;

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
bool ShaderGenerator::hasRenderState(const String& schemeName) const
{
    OGRE_LOCK_AUTO_MUTEX;

    SGSchemeConstIterator itFind = mSchemeEntriesMap.find(schemeName);
    return itFind != mSchemeEntriesMap.end();
}

//-----------------------------------------------------------------------------
ShaderGenerator::RenderStateCreateOrRetrieveResult ShaderGenerator::createOrRetrieveRenderState(const String& schemeName)
{
    SchemeCreateOrRetrieveResult res = createOrRetrieveScheme(schemeName);
    return RenderStateCreateOrRetrieveResult(res.first->getRenderState(),res.second);
}

//-----------------------------------------------------------------------------
ShaderGenerator::SchemeCreateOrRetrieveResult ShaderGenerator::createOrRetrieveScheme(const String& schemeName)
{
    OGRE_LOCK_AUTO_MUTEX;

    bool wasCreated = false;
    SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);
    SGScheme* schemeEntry = NULL;

    if (itScheme == mSchemeEntriesMap.end())
    {
        schemeEntry = OGRE_NEW SGScheme(schemeName);
        mSchemeEntriesMap.emplace(schemeName, schemeEntry);
        wasCreated = true;
    }
    else
    {
        schemeEntry = itScheme->second;
    }

    return SchemeCreateOrRetrieveResult(schemeEntry, wasCreated);
}
//-----------------------------------------------------------------------------
RenderState* ShaderGenerator::getRenderState(const String& schemeName,
                                     const String& materialName,
                                     const String& groupName,
                                     unsigned short passIndex)
{
    OGRE_LOCK_AUTO_MUTEX;

    SGSchemeIterator itFind = mSchemeEntriesMap.find(schemeName);

    if (itFind == mSchemeEntriesMap.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
            "A scheme named'" + schemeName + "' doesn't exists.",
            "ShaderGenerator::getRenderState");
    }

    return itFind->second->getRenderState(materialName, groupName, passIndex);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::addSceneManager(SceneManager* sceneMgr)
{
    // Make sure this scene manager not exists in the set.
    if (!mSceneManagerMap.insert(sceneMgr).second)
        return;

    if (!mRenderObjectListener)
        mRenderObjectListener.reset(new SGRenderObjectListener(this));

    sceneMgr->addRenderObjectListener(mRenderObjectListener.get());

    if (!mSceneManagerListener)
        mSceneManagerListener.reset(new SGSceneManagerListener(this));

    sceneMgr->addListener(mSceneManagerListener.get());

    // Update the active scene manager.
    if (mActiveSceneMgr == NULL)
        mActiveSceneMgr = sceneMgr;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::removeSceneManager(SceneManager* sceneMgr)
{
    // Make sure this scene manager exists in the map.
    SceneManagerIterator itFind = mSceneManagerMap.find(sceneMgr);

    if (itFind != mSceneManagerMap.end())
    {
        (*itFind)->removeRenderObjectListener(mRenderObjectListener.get());
        (*itFind)->removeListener(mSceneManagerListener.get());

        mSceneManagerMap.erase(itFind);

        // Update the active scene manager.
        if (mActiveSceneMgr == sceneMgr) {
            mActiveSceneMgr = NULL;

            // force refresh global scene manager material
            invalidateMaterial(MSN_SHADERGEN, "Ogre/TextureShadowReceiver", RGN_INTERNAL);
        }
    }
}

//-----------------------------------------------------------------------------
SceneManager* ShaderGenerator::getActiveSceneManager()
{
    return mActiveSceneMgr;
}
//-----------------------------------------------------------------------------
void ShaderGenerator::_setActiveSceneManager(SceneManager* sceneManager)
{
    mActiveViewportValid &= (mActiveSceneMgr == sceneManager);
    mActiveSceneMgr = sceneManager;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::setShaderProfiles(GpuProgramType type, const String& shaderProfiles)
{
    switch(type)
    {
    case GPT_VERTEX_PROGRAM:
        mVertexShaderProfiles = shaderProfiles;
        break;
    case GPT_FRAGMENT_PROGRAM:
        mFragmentShaderProfiles = shaderProfiles;
        break;
    default:
        OgreAssert(false, "not implemented");
        break;
    }
}

const String& ShaderGenerator::getShaderProfiles(GpuProgramType type) const
{
    switch(type)
    {
    case GPT_VERTEX_PROGRAM:
        return mVertexShaderProfiles;
    case GPT_FRAGMENT_PROGRAM:
        return mFragmentShaderProfiles;
    default:
        return BLANKSTRING;
    }
}
//-----------------------------------------------------------------------------
bool ShaderGenerator::hasShaderBasedTechnique(const String& materialName,
                                                 const String& groupName,
                                                 const String& srcTechniqueSchemeName,
                                                 const String& dstTechniqueSchemeName) const
{
    OGRE_LOCK_AUTO_MUTEX;

    // Make sure material exists;
    if (false == MaterialManager::getSingleton().resourceExists(materialName, groupName))
        return false;


    SGMaterialConstIterator itMatEntry = findMaterialEntryIt(materialName, groupName);

    // Check if technique already created.
    if (itMatEntry != mMaterialEntriesMap.end())
    {
        const SGTechniqueList& techniqueEntires = itMatEntry->second->getTechniqueList();
        for (auto* t : techniqueEntires) {
            // Check requested mapping already exists.
            if (t->getSourceTechnique()->getSchemeName() == srcTechniqueSchemeName && t->getDestinationTechniqueSchemeName() == dstTechniqueSchemeName)
                return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
static bool hasFixedFunctionPass(Technique* tech)
{
    for (const auto& p : tech->getPasses())
    {
        if (!p->isProgrammable())
        {
            return true;
        }
    }
    return false;
}

static Technique* findSourceTechnique(const Material& mat, const String& srcTechniqueSchemeName, bool overProgrammable)
{
    // Find the source technique
    for (auto *t : mat.getTechniques())
    {
        if (t->getSchemeName() == srcTechniqueSchemeName && t->isSupported() &&
            (hasFixedFunctionPass(t) || overProgrammable))
        {
            return t;
        }
    }

    return NULL;
}
//-----------------------------------------------------------------------------
bool ShaderGenerator::createShaderBasedTechnique(const Material& srcMat,
                                                 const String& srcTechniqueSchemeName,
                                                 const String& dstTechniqueSchemeName,
                                                 bool overProgrammable)
{
    // No technique created -> check if one can be created from the given source technique scheme.
    Technique* srcTechnique = findSourceTechnique(srcMat, srcTechniqueSchemeName, overProgrammable);

    // No appropriate source technique found.
    if (!srcTechnique)
    {
        return false;
    }

    return createShaderBasedTechnique(srcTechnique, dstTechniqueSchemeName, overProgrammable);
}

bool ShaderGenerator::createShaderBasedTechnique(const Technique* srcTechnique, const String& dstTechniqueSchemeName,
                                                 bool overProgrammable)
{
    OGRE_LOCK_AUTO_MUTEX;

    // Update group name in case it is AUTODETECT_RESOURCE_GROUP_NAME
    Material* srcMat = srcTechnique->getParent();
    const String& materialName = srcMat->getName();
    const String& trueGroupName = srcMat->getGroup();

    SGMaterialIterator itMatEntry = findMaterialEntryIt(materialName, trueGroupName);

    // Check if technique already created.
    if (itMatEntry != mMaterialEntriesMap.end())
    {
        const SGTechniqueList& techniqueEntires = itMatEntry->second->getTechniqueList();
        for (auto* t : techniqueEntires)
        {
            // Case the requested mapping already exists.
            if (t->getSourceTechnique()->getSchemeName() == srcTechnique->getSchemeName() && t->getDestinationTechniqueSchemeName() == dstTechniqueSchemeName)
                return true;
            // Case a shader based technique with the same scheme name already defined based
            // on different source technique.
            // This state might lead to conflicts during shader generation - we prevent it by returning false here.
            else if (t->getDestinationTechniqueSchemeName() == dstTechniqueSchemeName)
                return false;
        }
    }

    // Create shader based technique from the given source technique.
    SGMaterial* matEntry = NULL;

    if (itMatEntry == mMaterialEntriesMap.end())
    {
        matEntry = OGRE_NEW SGMaterial(materialName, trueGroupName);
        mMaterialEntriesMap.emplace(MatGroupPair(materialName, trueGroupName), matEntry);
    }
    else
    {
        matEntry = itMatEntry->second;
    }

    // Create the new technique entry.
    SGTechnique* techEntry =
        OGRE_NEW SGTechnique(matEntry, srcTechnique, dstTechniqueSchemeName, overProgrammable);

    // Add to material entry map.
    matEntry->getTechniqueList().push_back(techEntry);

    // Add to all technique map.
    mTechniqueEntriesMap[techEntry] = techEntry;

    // Add to scheme.
    SGScheme* schemeEntry = createOrRetrieveScheme(dstTechniqueSchemeName).first;
    schemeEntry->addTechniqueEntry(techEntry);

    return true;
}

bool ShaderGenerator::removeShaderBasedTechnique(const Technique* srcTech, const String& dstTechniqueSchemeName)
{
    OGRE_LOCK_AUTO_MUTEX;

    // Make sure scheme exists.
    SGSchemeIterator itScheme = mSchemeEntriesMap.find(dstTechniqueSchemeName);
    SGScheme* schemeEntry = NULL;

    if (itScheme == mSchemeEntriesMap.end())
        return false;

    schemeEntry = itScheme->second;

    // Find the material entry.
    Material* srcMat = srcTech->getParent();
    SGMaterialIterator itMatEntry = findMaterialEntryIt(srcMat->getName(), srcMat->getGroup());

    // Case material not found.
    if (itMatEntry == mMaterialEntriesMap.end())
        return false;


    SGTechniqueList& matTechniqueEntires = itMatEntry->second->getTechniqueList();
    SGTechniqueIterator itTechEntry = matTechniqueEntires.begin();
    SGTechnique* dstTechnique = NULL;

    // Remove destination technique entry from material techniques list.
    for (; itTechEntry != matTechniqueEntires.end(); ++itTechEntry)
    {
        if ((*itTechEntry)->getSourceTechnique()->getSchemeName() == srcTech->getSchemeName() &&
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
bool ShaderGenerator::removeAllShaderBasedTechniques(const String& materialName, const String& groupName)
{
    OGRE_LOCK_AUTO_MUTEX;

    // Find the material entry.
    SGMaterialIterator itMatEntry = findMaterialEntryIt(materialName, groupName);

    // Case material not found.
    if (itMatEntry == mMaterialEntriesMap.end())
        return false;


    SGTechniqueList& matTechniqueEntires = itMatEntry->second->getTechniqueList();

    // Remove all technique entries from material techniques list.
    while (matTechniqueEntires.empty() == false)
    {
        SGTechniqueIterator itTechEntry = matTechniqueEntires.begin();

        removeShaderBasedTechnique((*itTechEntry)->getSourceTechnique(),
                                   (*itTechEntry)->getDestinationTechniqueSchemeName());
    }

    OGRE_DELETE itMatEntry->second;
    mMaterialEntriesMap.erase(itMatEntry);

    return true;
}

bool ShaderGenerator::cloneShaderBasedTechniques(Material& srcMat, Material& dstMat)
{
    if(&srcMat == &dstMat) return true; // nothing to do

    SGMaterialIterator itSrcMatEntry = findMaterialEntryIt(srcMat.getName(), srcMat.getGroup());

    //remove any techniques in the destination material so the new techniques may be copied
    removeAllShaderBasedTechniques(dstMat);

    //
    //remove any techniques from the destination material which have RTSS associated schemes from
    //the source material. This code is performed in case the user performed a clone of a material
    //which has already generated RTSS techniques in the source material.
    //

    //first gather the techniques to remove
    std::set<unsigned short> schemesToRemove;
    for(Technique* pSrcTech : srcMat.getTechniques())
    {
        Pass* pSrcPass = pSrcTech->getNumPasses() > 0 ? pSrcTech->getPass(0) : NULL;
        if (pSrcPass)
        {
            const Any& passUserData = pSrcPass->getUserObjectBindings().getUserAny(TargetRenderState::UserKey);
            if (passUserData.has_value())
            {
                schemesToRemove.insert(pSrcTech->_getSchemeIndex());
            }
        }
    }
    //remove the techniques from the destination material
    auto techCount = dstMat.getNumTechniques();
    for(unsigned short ti = techCount - 1 ; ti != (unsigned short)-1 ; --ti)
    {
        Technique* pDstTech = dstMat.getTechnique(ti);
        if (schemesToRemove.find(pDstTech->_getSchemeIndex()) != schemesToRemove.end())
        {
            dstMat.removeTechnique(ti);
        }
    }

    //
    // Clone the render states from source to destination
    //

    srcMat.load(); // ensure supported techniques are loaded

    // Check if RTSS techniques exist in the source material
    if (itSrcMatEntry != mMaterialEntriesMap.end())
    {
        const SGTechniqueList& techniqueEntires = itSrcMatEntry->second->getTechniqueList();

        //Go over all rtss techniques in the source material
        for (auto* t : techniqueEntires)
        {
            String srcFromTechniqueScheme = t->getSourceTechnique()->getSchemeName();
            String srcToTechniqueScheme = t->getDestinationTechniqueSchemeName();

            //for every technique in the source material create a shader based technique in the
            //destination material
            if (createShaderBasedTechnique(dstMat, srcFromTechniqueScheme, srcToTechniqueScheme))
            {
                //check for custom render states in the source material
                unsigned short passCount = t->getSourceTechnique()->getNumPasses();
                for(unsigned short pi = 0 ; pi < passCount ; ++pi)
                {
                    if (t->hasRenderState(pi))
                    {
                        //copy the custom render state from the source material to the destination material
                        RenderState* srcRenderState = t->getRenderState(pi);
                        RenderState* dstRenderState = getRenderState(srcToTechniqueScheme, dstMat, pi);

                        for(SubRenderState* srcSubState : srcRenderState->getSubRenderStates())
                        {
                            SubRenderState* dstSubState = createSubRenderState(srcSubState->getType());
                            (*dstSubState) = (*srcSubState);
                            dstRenderState->addTemplateSubRenderState(dstSubState);
                        }
                    }
                }
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::removeAllShaderBasedTechniques()
{
    OGRE_LOCK_AUTO_MUTEX;

    while (!mMaterialEntriesMap.empty())
    {
        SGMaterialIterator itMatEntry = mMaterialEntriesMap.begin();

        removeAllShaderBasedTechniques(itMatEntry->first.first, itMatEntry->first.second);
    }
}

//-----------------------------------------------------------------------------
 void ShaderGenerator::notifyRenderSingleObject(Renderable* rend,
     const Pass* pass,
     const AutoParamDataSource* source,
     const LightList* pLightList, bool suppressRenderStateChanges)
{
    if (mActiveViewportValid)
    {
        const Any& passUserData = pass->getUserObjectBindings().getUserAny(TargetRenderState::UserKey);

        if (!passUserData.has_value() || suppressRenderStateChanges)
            return;

        OGRE_LOCK_AUTO_MUTEX;

        auto renderState = any_cast<TargetRenderStatePtr>(passUserData);
        renderState->updateGpuProgramsParams(rend, pass, source, pLightList);
    }
}


//-----------------------------------------------------------------------------
void ShaderGenerator::preFindVisibleObjects(SceneManager* source,
                                            SceneManager::IlluminationRenderStage irs,
                                            Viewport* v)
{
    OGRE_LOCK_AUTO_MUTEX;

    const String& curMaterialScheme = v->getMaterialScheme();

    mActiveSceneMgr      = source;
    mActiveViewportValid = validateScheme(curMaterialScheme);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::invalidateScheme(const String& schemeName)
{
    OGRE_LOCK_AUTO_MUTEX;

    SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);

    if (itScheme != mSchemeEntriesMap.end())
        itScheme->second->invalidate();

}

//-----------------------------------------------------------------------------
bool ShaderGenerator::validateScheme(const String& schemeName)
{
    OGRE_LOCK_AUTO_MUTEX;

    SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);

    // No such scheme exists.
    if (itScheme == mSchemeEntriesMap.end())
        return false;

    itScheme->second->validate();

    return true;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::invalidateMaterial(const String& schemeName, const String& materialName, const String& groupName)
{
    OGRE_LOCK_AUTO_MUTEX;

    SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);

    if (itScheme != mSchemeEntriesMap.end())
        itScheme->second->invalidate(materialName, groupName);
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::validateMaterial(const String& schemeName, const String& materialName, const String& groupName)
{
    OGRE_LOCK_AUTO_MUTEX;

    SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);

    // No such scheme exists.
    if (itScheme == mSchemeEntriesMap.end())
        return false;

    return itScheme->second->validate(materialName, groupName);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::invalidateMaterialIlluminationPasses(const String& schemeName, const String& materialName, const String& groupName)
{
	OGRE_LOCK_AUTO_MUTEX;

	SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);

	if(itScheme != mSchemeEntriesMap.end())
		itScheme->second->invalidateIlluminationPasses(materialName, groupName);
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::validateMaterialIlluminationPasses(const String& schemeName, const String& materialName, const String& groupName)
{
	OGRE_LOCK_AUTO_MUTEX;

	SGSchemeIterator itScheme = mSchemeEntriesMap.find(schemeName);

	// No such scheme exists.
	if(itScheme == mSchemeEntriesMap.end())
		return false;

	return itScheme->second->validateIlluminationPasses(materialName, groupName);
}

//-----------------------------------------------------------------------------
MaterialSerializer::Listener* ShaderGenerator::getMaterialSerializerListener()
{
    if (!mMaterialSerializerListener)
        mMaterialSerializerListener.reset(new SGMaterialSerializerListener);

    return mMaterialSerializerListener.get();
}

//-----------------------------------------------------------------------------
void ShaderGenerator::flushShaderCache()
{
    // Release all programs.
    for (auto& t : mTechniqueEntriesMap)
    {
        t.second->releasePrograms();
    }

    ProgramManager::getSingleton().flushGpuProgramsCache();

    // Invalidate all schemes.
    for (auto& s : mSchemeEntriesMap)
    {
        s.second->invalidate();
    }
}

//-----------------------------------------------------------------------------
ScriptTranslator* ShaderGenerator::getTranslator(const AbstractNodePtr& node)
{
    OGRE_LOCK_AUTO_MUTEX;

    if(node->type != ANT_OBJECT)
        return NULL;

    ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

    if(obj->id == ID_RT_SHADER_SYSTEM)
        return &mCoreScriptTranslator;

    return NULL;
}

//-----------------------------------------------------------------------------
size_t ShaderGenerator::getShaderCount(GpuProgramType type) const
{
    return mProgramManager->getShaderCount(type);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::setTargetLanguage(const String& shaderLanguage)
{
    // Make sure that the shader language is supported.
    if (!mProgramWriterManager->isLanguageSupported(shaderLanguage))
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "'" + shaderLanguage + "' is not supported");
    }

    // Case target language changed -> flush the shaders cache.
    if (mShaderLanguage != shaderLanguage)
    {
        mShaderLanguage = shaderLanguage;
        flushShaderCache();
    }
}

//-----------------------------------------------------------------------------
void ShaderGenerator::setShaderCachePath( const String& cachePath )
{
    String stdCachePath = cachePath;

    // Standardise the cache path in case of none empty string.
    if (stdCachePath.empty() == false)
        stdCachePath = StringUtil::standardisePath(stdCachePath);

    if (mShaderCachePath != stdCachePath)
    {
        mShaderCachePath = stdCachePath;

        // Case this is a valid file path -> add as resource location in order to make sure that
        // generated shaders could be loaded by the file system archive.
        if (mShaderCachePath.empty() == false)
        {
            // Make sure this is a valid writable path.
            String outTestFileName(mShaderCachePath + "ShaderGenerator.tst");
            std::ofstream outFile(outTestFileName.c_str());

            if (!outFile)
            {
                OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE,
                    "Could not create output files in the given shader cache path '" + mShaderCachePath,
                    "ShaderGenerator::setShaderCachePath");
            }

            // Close and remove the test file.
            outFile.close();
            remove(outTestFileName.c_str());
        }
    }
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGMaterialIterator ShaderGenerator::findMaterialEntryIt(const String& materialName, const String& groupName)
{
    SGMaterialIterator itMatEntry;
    //check if we have auto detect request
    if (groupName == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)
    {
        //find the possible first entry
        itMatEntry = mMaterialEntriesMap.lower_bound(MatGroupPair(materialName,""));
        if ((itMatEntry != mMaterialEntriesMap.end()) &&
            (itMatEntry->first.first != materialName))
        {
            //no entry found
            itMatEntry = mMaterialEntriesMap.end();
        }
    }
    else
    {
        //find entry with group name specified
        itMatEntry = mMaterialEntriesMap.find(MatGroupPair(materialName,groupName));
    }
    return itMatEntry;
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGMaterialConstIterator ShaderGenerator::findMaterialEntryIt(const String& materialName, const String& groupName) const
{
    SGMaterialConstIterator itMatEntry;
    //check if we have auto detect request
    if (groupName == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)
    {
        //find the possible first entry
        itMatEntry = mMaterialEntriesMap.lower_bound(MatGroupPair(materialName,""));
        if ((itMatEntry != mMaterialEntriesMap.end()) &&
            (itMatEntry->first.first != materialName))
        {
            //no entry found
            itMatEntry = mMaterialEntriesMap.end();
        }
    }
    else
    {
        //find entry with group name specified
        itMatEntry = mMaterialEntriesMap.find(MatGroupPair(materialName,groupName));
    }
    return itMatEntry;
}
//-----------------------------------------------------------------------------
size_t ShaderGenerator::getRTShaderSchemeCount() const
{
    OGRE_LOCK_AUTO_MUTEX;
    return mSchemeEntriesMap.size();
}
//-----------------------------------------------------------------------------
const String& ShaderGenerator::getRTShaderScheme(size_t index) const
{
    OGRE_LOCK_AUTO_MUTEX;

    SGSchemeMap::const_iterator it = mSchemeEntriesMap.begin();
    while ((index != 0) && (it != mSchemeEntriesMap.end()))
    {
        --index;
        ++it;
    }

    assert((it != mSchemeEntriesMap.end()) && "Index out of bounds");
    if (it != mSchemeEntriesMap.end())
        return it->first;
    else return BLANKSTRING;
}

void ShaderGenerator::_markNonFFP(const TextureUnitState* tu)
{
    auto pass = tu->getParent();
    auto texureIdx = pass->getTextureUnitStateIndex(tu);

    // add to blacklist
    std::set<uint16> nonFFP_TUS;
    auto any = pass->getUserObjectBindings().getUserAny("_RTSS_nonFFP_TUS");
    if(any.has_value())
        nonFFP_TUS = any_cast<std::set<uint16>>(any);
    nonFFP_TUS.insert(texureIdx);
    pass->getUserObjectBindings().setUserAny("_RTSS_nonFFP_TUS", nonFFP_TUS);
}

//-----------------------------------------------------------------------------

bool ShaderGenerator::getIsFinalizing() const
{
    return mIsFinalizing;
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGPassList ShaderGenerator::createSGPassList(Material* mat) const
{
    SGPassList passList;

    SGMaterialConstIterator itMatEntry = findMaterialEntryIt(mat->getName(), mat->getGroup());

    // Check if material is managed.
    if (itMatEntry == mMaterialEntriesMap.end())
        return passList;

    for (auto sgtech : itMatEntry->second->getTechniqueList())
    {
        for(auto sgpass : sgtech->getPassList())
        {
            passList.push_back(sgpass);
        }
    }

    return passList;
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGPass::SGPass(SGTechnique* parent, Pass* srcPass, Pass* dstPass, IlluminationStage stage)
{
    mParent       = parent;
    mSrcPass      = srcPass;
    mDstPass			= dstPass;
    mStage				= stage;
    mCustomRenderState  = NULL;
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGPass::~SGPass()
{
    mDstPass->getUserObjectBindings().eraseUserAny(TargetRenderState::UserKey);
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGPass::buildTargetRenderState()
{
    if(mSrcPass->isProgrammable() && !mParent->overProgrammablePass() && !isIlluminationPass()) return;
    const String& schemeName = mParent->getDestinationTechniqueSchemeName();
    const RenderState* renderStateGlobal = ShaderGenerator::getSingleton().getRenderState(schemeName);


    auto targetRenderState = std::make_shared<TargetRenderState>();

    // Set light properties.
    int32 lightCount = 0;
    bool haveAreaLights = false;

    // Use light count definitions of the custom render state if exists.
    if (mCustomRenderState != NULL && mCustomRenderState->getLightCountAutoUpdate() == false)
    {
        lightCount = mCustomRenderState->getLightCount();
    }

    // Use light count definitions of the global render state if exists.
    else if (renderStateGlobal != NULL)
    {
        lightCount = renderStateGlobal->getLightCount();
        haveAreaLights = renderStateGlobal->haveAreaLights();
    }


    targetRenderState->setLightCount(lightCount);
    targetRenderState->setHaveAreaLights(haveAreaLights);

    // Link the target render state with the scheme render state of the shader generator.
    if (renderStateGlobal != NULL)
    {
        targetRenderState->link(*renderStateGlobal, mSrcPass, mDstPass);
    }

    // Link the target render state with the custom render state of this pass if exists.
    if (mCustomRenderState != NULL)
    {
        targetRenderState->link(*mCustomRenderState, mSrcPass, mDstPass);
    }

    targetRenderState->acquirePrograms(mDstPass);
    mDstPass->getUserObjectBindings().setUserAny(TargetRenderState::UserKey, targetRenderState);
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGTechnique::SGTechnique(SGMaterial* parent, const Technique* srcTechnique,
                                          const String& dstTechniqueSchemeName,
                                          bool overProgrammable)
    : mParent(parent), mSrcTechnique(srcTechnique), mDstTechnique(NULL), mBuildDstTechnique(true),
      mDstTechniqueSchemeName(dstTechniqueSchemeName), mOverProgrammable(overProgrammable)
{

}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::createSGPasses()
{
    // Create pass entry for each pass.
    for (unsigned short i=0; i < mSrcTechnique->getNumPasses(); ++i)
    {
        Pass* srcPass = mSrcTechnique->getPass(i);
        Pass* dstPass = mDstTechnique->getPass(i);

		SGPass* passEntry = OGRE_NEW SGPass(this, srcPass, dstPass, IS_UNKNOWN);

        if (i < mCustomRenderStates.size())
            passEntry->setCustomRenderState(mCustomRenderStates[i]);
        mPassEntries.push_back(passEntry);
    }
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::createIlluminationSGPasses()
{
	// Create pass entry for each illumination pass.
	const IlluminationPassList& passes = mDstTechnique->getIlluminationPasses();

	for(size_t i = 0; i < passes.size(); i++)
	{
		// process only autogenerated illumination passes
		IlluminationPass* p = passes[i];
		if(p->pass == p->originalPass)
			continue;

		SGPass* passEntry = OGRE_NEW SGPass(this, p->pass, p->pass, p->stage);
		const Any& origPassUserData = p->originalPass->getUserObjectBindings().getUserAny(TargetRenderState::UserKey);
		if(origPassUserData.has_value())
		{
            for(auto *sgp : mPassEntries)
            {
                if(sgp->getDstPass() == p->originalPass)
                {
                    passEntry->setCustomRenderState(sgp->getCustomRenderState());
                    break;
                }
            }
		}

		mPassEntries.push_back(passEntry);
	}
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::destroyIlluminationSGPasses()
{
	for(SGPassIterator itPass = mPassEntries.begin(); itPass != mPassEntries.end(); /* no increment*/ )
	{
		if((*itPass)->isIlluminationPass())
		{
			OGRE_DELETE(*itPass);
			itPass = mPassEntries.erase(itPass);
		}
		else
		{
			++itPass;
		}
	}
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGTechnique::~SGTechnique()
{
    const String& materialName = mParent->getMaterialName();
    const String& groupName = mParent->getGroupName();

    // Destroy the passes.
    destroySGPasses();

    if (MaterialManager::getSingleton().resourceExists(materialName, groupName))
    {
        MaterialPtr mat = MaterialManager::getSingleton().getByName(materialName, groupName);

        // Remove the destination technique from parent material.
        for (ushort i=0; i < mat->getNumTechniques(); ++i)
        {
            if (mDstTechnique == mat->getTechnique(i))
            {
                // Unload the generated technique in order tor free referenced resources.
                mDstTechnique->_unload();

                // Remove the generated technique in order to restore the material to its original state.
                mat->removeTechnique(i);

                // touch when finalizing - will reload the textures - so no touch if finalizing
                if (ShaderGenerator::getSingleton().getIsFinalizing() == false)
                {
                    // Make sure the material goes back to its original state.
                    mat->touch();
                }
                break;
            }
        }
    }

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
    for (auto& p : mPassEntries)
    {
        OGRE_DELETE p;
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
    mDstTechnique   = mSrcTechnique->getParent()->createTechnique();
    mDstTechnique->getUserObjectBindings().setUserAny(SGTechnique::UserKey, this);
    *mDstTechnique  = *mSrcTechnique;
    mDstTechnique->setSchemeName(mDstTechniqueSchemeName);
    createSGPasses();

    // Build render state for each pass.
    for (auto *p : mPassEntries)
    {
	assert(!p->isIlluminationPass()); // this is not so important, but intended to be so here.
        p->buildTargetRenderState();
    }

    // Turn off the build destination technique flag.
    mBuildDstTechnique = false;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGTechnique::buildIlluminationTargetRenderState()
{
	assert(mDstTechnique != NULL);
	assert(!getBuildDestinationTechnique());

	// Create the illumination passes.
	createIlluminationSGPasses();

	// Build render state for each pass.
	for(auto *p : mPassEntries) {

	    if(p->isIlluminationPass())
		    p->buildTargetRenderState();
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
bool ShaderGenerator::SGTechnique::hasRenderState(unsigned short passIndex)
{
    return (passIndex < mCustomRenderStates.size()) && (mCustomRenderStates[passIndex] != NULL);
}


//-----------------------------------------------------------------------------
ShaderGenerator::SGScheme::SGScheme(const String& schemeName) :
    mName(schemeName), mOutOfDate(true), mFogMode(FOG_NONE)
{
}

//-----------------------------------------------------------------------------
ShaderGenerator::SGScheme::~SGScheme()
{
}

//-----------------------------------------------------------------------------
RenderState* ShaderGenerator::SGScheme::getRenderState()
{
    if (!mRenderState)
    {
        mRenderState.reset(new RenderState);
        mRenderState->resetToBuiltinSubRenderStates();
    }

    return mRenderState.get();
}

//-----------------------------------------------------------------------------
RenderState* ShaderGenerator::SGScheme::getRenderState(const String& materialName, const String& groupName, unsigned short passIndex)
{
    // Find the desired technique.
    bool doAutoDetect = groupName == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
    for (auto *t : mTechniqueEntries)
    {
        Material* curMat = t->getSourceTechnique()->getParent();
        if ((curMat->getName() == materialName) &&
            ((doAutoDetect == true) || (curMat->getGroup() == groupName)))
        {
            return t->getRenderState(passIndex);
        }
    }

    return NULL;
}


//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::addTechniqueEntry(SGTechnique* techEntry)
{
    mTechniqueEntries.push_back(techEntry);

    // Mark as out of data.
    mOutOfDate = true;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::removeTechniqueEntry(SGTechnique* techEntry)
{
    SGTechniqueIterator itTech;

    // Build render state for each technique.
    for (itTech = mTechniqueEntries.begin(); itTech != mTechniqueEntries.end(); ++itTech)
    {
        SGTechnique* curTechEntry = *itTech;

        if (curTechEntry == techEntry)
        {
            mTechniqueEntries.erase(itTech);
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

    // Build render state for each technique and acquire GPU programs.
    for (SGTechnique* curTechEntry : mTechniqueEntries)
    {
        if (curTechEntry->getBuildDestinationTechnique())
            curTechEntry->buildTargetRenderState();
    }

    // Mark this scheme as up to date.
    mOutOfDate = false;
}

//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::synchronizeWithLightSettings()
{
    SceneManager* sceneManager = ShaderGenerator::getSingleton().getActiveSceneManager();
    RenderState* curRenderState = getRenderState();

    if (curRenderState->getLightCountAutoUpdate())
    {
        OgreAssert(sceneManager, "no active SceneManager. Did you forget to call ShaderGenerator::addSceneManager?");

        const LightList& lightList =  sceneManager->_getLightsAffectingFrustum();

        // area lights a costy, so we check whether there are any in the scene
        bool haveAreaLights = false;
        for (const Light* l : lightList)
            haveAreaLights = haveAreaLights || l->getType() == Light::LT_RECTLIGHT;

        int32 sceneLightCount = lightList.size();
        int32 currLightCount = mRenderState->getLightCount();

        // Case new light appeared -> invalidate. But dont invalidate the other way as shader compilation is costly.
        if ((currLightCount - sceneLightCount) < 0)
        {
            LogManager::getSingleton().stream(LML_TRIVIAL)
                << "RTSS: invalidating scheme " << mName << " - lights changed " << currLightCount
                << " -> " << sceneLightCount;
            curRenderState->setLightCount(sceneLightCount);
            invalidate();
        }

        if(!curRenderState->haveAreaLights() && haveAreaLights)
        {
            LogManager::getSingleton().stream(LML_TRIVIAL)
                << "RTSS: invalidating scheme " << mName << " - enabling area lights";
            curRenderState->setHaveAreaLights(true);
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
        LogManager::getSingleton().stream(LML_TRIVIAL)
            << "RTSS: invalidating scheme " << mName << " - fog settings changed";
        mFogMode = sceneManager->getFogMode();
        invalidate();
    }
}

//-----------------------------------------------------------------------------
bool ShaderGenerator::SGScheme::validate(const String& materialName, const String& groupName)
{
    // Synchronize with light settings.
    synchronizeWithLightSettings();

    // Synchronize with fog settings.
    synchronizeWithFogSettings();

    // Find the desired technique.
    bool doAutoDetect = groupName == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
    for (auto *t : mTechniqueEntries)
    {
        const SGMaterial* curMat = t->getParent();
        if ((curMat->getMaterialName() == materialName) &&
            ((doAutoDetect == true) || (curMat->getGroupName() == groupName)) && (t->getBuildDestinationTechnique())) {
            // Build render state for each technique and Acquire the CPU/GPU programs.
            t->buildTargetRenderState();

			return true;
        }
    }

    return false;
}
//-----------------------------------------------------------------------------
bool ShaderGenerator::SGScheme::validateIlluminationPasses(const String& materialName, const String& groupName)
{
	// Synchronize with light settings.
	synchronizeWithLightSettings();

	// Synchronize with fog settings.
	synchronizeWithFogSettings();

	// Find the desired technique.
	bool doAutoDetect = groupName == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
	for(SGTechnique* curTechEntry : mTechniqueEntries)
	{
		const SGMaterial* curMat = curTechEntry->getParent();
		if((curMat->getMaterialName() == materialName) &&
		   ((doAutoDetect == true) || (curMat->getGroupName() == groupName)))
		{
			// Build render state for each technique and Acquire the CPU/GPU programs.
			curTechEntry->buildIlluminationTargetRenderState();

			return true;
		}
	}

	return false;
}
//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::invalidateIlluminationPasses(const String& materialName, const String& groupName)
{
	// Find the desired technique.
	bool doAutoDetect = groupName == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
	for(auto *t : mTechniqueEntries)
	{
		const SGMaterial* curMat = t->getParent();
		if((curMat->getMaterialName() == materialName) &&
		   ((doAutoDetect == true) || (curMat->getGroupName() == groupName)))
		{
			t->destroyIlluminationSGPasses();
		}
	}
}
//-----------------------------------------------------------------------------
void ShaderGenerator::SGScheme::invalidate(const String& materialName, const String& groupName)
{
    // Find the desired technique.
    bool doAutoDetect = groupName == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
    for (SGTechnique* curTechEntry : mTechniqueEntries)
    {
        const SGMaterial* curMaterial = curTechEntry->getParent();
        if ((curMaterial->getMaterialName() == materialName) &&
            ((doAutoDetect == true) || (curMaterial->getGroupName() == groupName)))
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
    // Turn on the build destination technique flag of all techniques.
    for (SGTechnique* curTechEntry : mTechniqueEntries)
    {
        curTechEntry->setBuildDestinationTechnique(true);
    }

    mOutOfDate = true;
}

}
}
