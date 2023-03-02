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
#include "OgreStableHeaders.h"

#include "OgreEntity.h"
#include "OgreControllerManager.h"
#include "OgreAnimation.h"
#include "OgreRenderObjectListener.h"
#include "OgreBillboardSet.h"
#include "OgreStaticGeometry.h"
#include "OgreSubEntity.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreBillboardChain.h"
#include "OgreRibbonTrail.h"
#include "OgreParticleSystem.h"
#include "OgreCompositorChain.h"
#include "OgreInstanceBatch.h"
#include "OgreInstancedEntity.h"
#include "OgreRenderTexture.h"
#include "OgreLodListener.h"
#include "OgreDefaultDebugDrawer.h"

// This class implements the most basic scene manager

#include <cstdio>

namespace Ogre {
//-----------------------------------------------------------------------
SceneManager::SceneManager(const String& name) :
mName(name),
mCameraInProgress(0),
mCurrentViewport(0),
mSkyPlane(this),
mSkyBox(this),
mSkyDome(this),
mFogMode(FOG_NONE),
mFogColour(),
mFogStart(0),
mFogEnd(0),
mFogDensity(0),
mSpecialCaseQueueMode(SCRQM_EXCLUDE),
mWorldGeometryRenderQueue(RENDER_QUEUE_WORLD_GEOMETRY_1),
mLastFrameNumber(0),
mResetIdentityView(false),
mResetIdentityProj(false),
mFlipCullingOnNegativeScale(true),
mLightsDirtyCounter(0),
mMovableNameGenerator("Ogre/MO"),
mShadowRenderer(this),
mDisplayNodes(false),
mShowBoundingBoxes(false),
mActiveCompositorChain(0),
mLateMaterialResolving(false),
mIlluminationStage(IRS_NONE),
mLightClippingInfoMapFrameNumber(999),
mVisibilityMask(0xFFFFFFFF),
mFindVisibleObjects(true),
mCameraRelativeRendering(false),
mLastLightHash(0),
mGpuParamsDirty((uint16)GPV_ALL)
{
    if (Root* root = Root::getSingletonPtr())
        _setDestinationRenderSystem(root->getRenderSystem());

    // Setup default queued renderable visitor
    mActiveQueuedRenderableVisitor = &mDefaultQueuedRenderableVisitor;

    // init shadow texture config
    setShadowTextureCount(1);

    mDebugDrawer.reset(new DefaultDebugDrawer());
    addListener(mDebugDrawer.get());

    // create the auto param data source instance
    mAutoParamDataSource.reset(createAutoParamDataSource());
}
//-----------------------------------------------------------------------
SceneManager::~SceneManager()
{
    fireSceneManagerDestroyed();
    clearScene();
    destroyAllCameras();

    // clear down movable object collection map
    {
            OGRE_LOCK_MUTEX(mMovableObjectCollectionMapMutex);
        for (MovableObjectCollectionMap::iterator i = mMovableObjectCollectionMap.begin();
            i != mMovableObjectCollectionMap.end(); ++i)
        {
            OGRE_DELETE_T(i->second, MovableObjectCollection, MEMCATEGORY_SCENE_CONTROL);
        }
        mMovableObjectCollectionMap.clear();
    }
}
//-----------------------------------------------------------------------
RenderQueue* SceneManager::getRenderQueue(void)
{
    if (!mRenderQueue)
    {
        initRenderQueue();
    }
    return mRenderQueue.get();
}
//-----------------------------------------------------------------------
void SceneManager::initRenderQueue(void)
{
    mRenderQueue.reset(new RenderQueue());
    // init render queues that do not need shadows
    mRenderQueue->getQueueGroup(RENDER_QUEUE_BACKGROUND)->setShadowsEnabled(false);
    mRenderQueue->getQueueGroup(RENDER_QUEUE_OVERLAY)->setShadowsEnabled(false);
    mRenderQueue->getQueueGroup(RENDER_QUEUE_SKIES_EARLY)->setShadowsEnabled(false);
    mRenderQueue->getQueueGroup(RENDER_QUEUE_SKIES_LATE)->setShadowsEnabled(false);
}
//-----------------------------------------------------------------------
void SceneManager::addSpecialCaseRenderQueue(uint8 qid)
{
    mSpecialCaseQueueList.insert(qid);
}
//-----------------------------------------------------------------------
void SceneManager::removeSpecialCaseRenderQueue(uint8 qid)
{
    mSpecialCaseQueueList.erase(qid);
}
//-----------------------------------------------------------------------
void SceneManager::clearSpecialCaseRenderQueues(void)
{
    mSpecialCaseQueueList.clear();
}
//-----------------------------------------------------------------------
void SceneManager::setSpecialCaseRenderQueueMode(SceneManager::SpecialCaseRenderQueueMode mode)
{
    mSpecialCaseQueueMode = mode;
}
//-----------------------------------------------------------------------
SceneManager::SpecialCaseRenderQueueMode SceneManager::getSpecialCaseRenderQueueMode(void)
{
    return mSpecialCaseQueueMode;
}
//-----------------------------------------------------------------------
bool SceneManager::isRenderQueueToBeProcessed(uint8 qid)
{
    bool inList = mSpecialCaseQueueList.find(qid) != mSpecialCaseQueueList.end();
    return (inList && mSpecialCaseQueueMode == SCRQM_INCLUDE)
        || (!inList && mSpecialCaseQueueMode == SCRQM_EXCLUDE);
}
//-----------------------------------------------------------------------
Camera* SceneManager::createCamera(const String& name)
{
    // Check name not used
    if (mCameras.find(name) != mCameras.end())
    {
        OGRE_EXCEPT(
            Exception::ERR_DUPLICATE_ITEM,
            "A camera with the name " + name + " already exists",
            "SceneManager::createCamera" );
    }

    Camera *c = OGRE_NEW Camera(name, this);
    mCameras.emplace(name, c);

    // create visible bounds aab map entry
    mCamVisibleObjectsMap[c] = VisibleObjectsBoundsInfo();

    return c;
}

//-----------------------------------------------------------------------
Camera* SceneManager::getCamera(const String& name) const
{
    CameraList::const_iterator i = mCameras.find(name);
    if (i == mCameras.end())
    {
        OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, 
            "Cannot find Camera with name " + name,
            "SceneManager::getCamera");
    }
    else
    {
        return i->second;
    }
}
//-----------------------------------------------------------------------
bool SceneManager::hasCamera(const String& name) const
{
    return (mCameras.find(name) != mCameras.end());
}

//-----------------------------------------------------------------------
void SceneManager::destroyCamera(Camera *cam)
{
    OgreAssert(cam, "Cannot destroy a null Camera");
    destroyCamera(cam->getName());
}

//-----------------------------------------------------------------------
void SceneManager::destroyCamera(const String& name)
{
    // Find in list
    CameraList::iterator i = mCameras.find(name);
    if (i != mCameras.end())
    {
        // Remove visible boundary AAB entry
        CamVisibleObjectsMap::iterator camVisObjIt = mCamVisibleObjectsMap.find( i->second );
        if ( camVisObjIt != mCamVisibleObjectsMap.end() )
            mCamVisibleObjectsMap.erase( camVisObjIt );

        // Remove light-shadow cam mapping entry
        auto camLightIt = mShadowRenderer.mShadowCamLightMapping.find( i->second );
        if ( camLightIt != mShadowRenderer.mShadowCamLightMapping.end() )
            mShadowRenderer.mShadowCamLightMapping.erase( camLightIt );

        // Notify render system
        if(mDestRenderSystem)
            mDestRenderSystem->_notifyCameraRemoved(i->second);
        OGRE_DELETE i->second;
        mCameras.erase(i);
    }

}

//-----------------------------------------------------------------------
void SceneManager::destroyAllCameras(void)
{
    CameraList::iterator camIt = mCameras.begin();
    while( camIt != mCameras.end() )
    {
        bool dontDelete = false;
         // dont destroy shadow texture cameras here. destroyAllCameras is public
        for(auto camShadowTex : mShadowRenderer.mShadowTextureCameras)
        {
            if( camShadowTex == camIt->second )
            {
                dontDelete = true;
                break;
            }
        }

        if( dontDelete )    // skip this camera
            ++camIt;
        else 
        {
            destroyCamera(camIt->second);
            camIt = mCameras.begin(); // recreate iterator
        }
    }

}
//-----------------------------------------------------------------------
Light* SceneManager::createLight(const String& name)
{
    return static_cast<Light*>(
        createMovableObject(name, LightFactory::FACTORY_TYPE_NAME));
}
//-----------------------------------------------------------------------
Light* SceneManager::createLight()
{
    String name = mMovableNameGenerator.generate();
    return createLight(name);
}
//-----------------------------------------------------------------------
Light* SceneManager::getLight(const String& name) const
{
    return static_cast<Light*>(
        getMovableObject(name, LightFactory::FACTORY_TYPE_NAME));
}
//-----------------------------------------------------------------------
bool SceneManager::hasLight(const String& name) const
{
    return hasMovableObject(name, LightFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyLight(const String& name)
{
    destroyMovableObject(name, LightFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyAllLights(void)
{
    destroyAllMovableObjectsByType(LightFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
const LightList& SceneManager::_getLightsAffectingFrustum(void) const
{
    return mLightsAffectingFrustum;
}
//-----------------------------------------------------------------------
void SceneManager::_populateLightList(const Vector3& position, Real radius, LightList& destList, uint32 lightMask)
{
    // Really basic trawl of the lights, then sort
    // Subclasses could do something smarter

    // Pre-allocate memory
    destList.clear();
    destList.reserve(mLightsAffectingFrustum.size());

    size_t lightIndex = 0;
    size_t numShadowTextures = isShadowTechniqueTextureBased() ? getShadowTextureConfigList().size() : 0;
    size_t numShadowCastingLights = 0;

    // Pick up the lights that affecting frustum only, which should has been
    // cached, so better than take all lights in the scene into account.
    // this is partitioned as: | shadow casting lights | other lights |
    // NOTE: no shadow casting lights might be in frustum, so we cannot rely on numShadowTextures
    for (Light* lt : mLightsAffectingFrustum)
    {
        // check whether or not this light is suppose to be taken into consideration for the current light mask set for this operation
        if(!(lt->getLightMask() & lightMask))
            continue; //skip this light

        // Calc squared distance
        lt->_calcTempSquareDist(position);

        // only add in-range lights, but ensure texture shadow casters are there
        if ((lt->getCastShadows() && lightIndex < numShadowTextures) || lt->isInLightRange(Sphere(position, radius)))
        {
            destList.push_back(lt);
        }

        numShadowCastingLights += int(lt->getCastShadows());
        lightIndex++;
    }

    auto start = destList.begin();
    // if we're using texture shadows, we actually want to use
    // the first few lights unchanged from the frustum list, matching the
    // texture shadows that were generated
    // Thus we only allow object-relative sorting on the remainder of the list
    std::advance(start, std::min(numShadowCastingLights, destList.size()));
    // Sort (stable to guarantee ordering on directional lights)
    std::stable_sort(start, destList.end(),
                     [](const Light* a, const Light* b) { return a->tempSquareDist < b->tempSquareDist; });

    // Now assign indexes in the list so they can be examined if needed
    lightIndex = 0;
    for (auto lt : destList)
    {
        lt->_notifyIndexInFrame(lightIndex++);
    }
}
//-----------------------------------------------------------------------
Entity* SceneManager::createEntity(
                                   const String& entityName,
                                   const String& meshName,
                                   const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
{
    // delegate to factory implementation
    NameValuePairList params;
    params["mesh"] = meshName;
    params["resourceGroup"] = groupName;
    return static_cast<Entity*>(
        createMovableObject(entityName, EntityFactory::FACTORY_TYPE_NAME, 
            &params));

}
//---------------------------------------------------------------------
Entity* SceneManager::createEntity(const String& entityName, const MeshPtr& pMesh)
{
    return createEntity(entityName, pMesh->getName(), pMesh->getGroup());
}
//---------------------------------------------------------------------
Entity* SceneManager::createEntity(const String& meshName)
{
    String name = mMovableNameGenerator.generate();
    // note, we can't allow groupName to be passes, it would be ambiguous (2 string params)
    return createEntity(name, meshName, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
}
//---------------------------------------------------------------------
Entity* SceneManager::createEntity(const MeshPtr& pMesh)
{
    String name = mMovableNameGenerator.generate();
    return createEntity(name, pMesh);
}
//-----------------------------------------------------------------------
Entity* SceneManager::getEntity(const String& name) const
{
    return static_cast<Entity*>(
        getMovableObject(name, EntityFactory::FACTORY_TYPE_NAME));
}
//-----------------------------------------------------------------------
bool SceneManager::hasEntity(const String& name) const
{
    return hasMovableObject(name, EntityFactory::FACTORY_TYPE_NAME);
}

//-----------------------------------------------------------------------
void SceneManager::destroyEntity(const String& name)
{
    destroyMovableObject(name, EntityFactory::FACTORY_TYPE_NAME);

}

//-----------------------------------------------------------------------
void SceneManager::destroyAllEntities(void)
{

    destroyAllMovableObjectsByType(EntityFactory::FACTORY_TYPE_NAME);
}

//-----------------------------------------------------------------------
void SceneManager::destroyAllBillboardSets(void)
{
    destroyAllMovableObjectsByType(BillboardSetFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
ManualObject* SceneManager::createManualObject(const String& name)
{
    return static_cast<ManualObject*>(
        createMovableObject(name, ManualObjectFactory::FACTORY_TYPE_NAME));
}
//-----------------------------------------------------------------------
ManualObject* SceneManager::createManualObject()
{
    String name = mMovableNameGenerator.generate();
    return createManualObject(name);
}
//-----------------------------------------------------------------------
ManualObject* SceneManager::getManualObject(const String& name) const
{
    return static_cast<ManualObject*>(
        getMovableObject(name, ManualObjectFactory::FACTORY_TYPE_NAME));

}
//-----------------------------------------------------------------------
bool SceneManager::hasManualObject(const String& name) const
{
    return hasMovableObject(name, ManualObjectFactory::FACTORY_TYPE_NAME);

}
//-----------------------------------------------------------------------
void SceneManager::destroyManualObject(const String& name)
{
    destroyMovableObject(name, ManualObjectFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyAllManualObjects(void)
{
    destroyAllMovableObjectsByType(ManualObjectFactory::FACTORY_TYPE_NAME);
}
Rectangle2D* SceneManager::createScreenSpaceRect(const String& name, bool includeTextureCoords)
{
    NameValuePairList params;
    if(includeTextureCoords)
        params["includeTextureCoords"] = "true";
    return static_cast<Rectangle2D*>(createMovableObject(name, Rectangle2DFactory::FACTORY_TYPE_NAME, &params));
}
Rectangle2D* SceneManager::createScreenSpaceRect(bool includeTextureCoords)
{
    return createScreenSpaceRect(mMovableNameGenerator.generate(), includeTextureCoords);
}

bool SceneManager::hasScreenSpaceRect(const String& name) const
{
    return hasMovableObject(name, Rectangle2DFactory::FACTORY_TYPE_NAME);
}
Rectangle2D* SceneManager::getScreenSpaceRect(const String& name) const
{
    return static_cast<Rectangle2D*>(getMovableObject(name, Rectangle2DFactory::FACTORY_TYPE_NAME));
}
//-----------------------------------------------------------------------
BillboardChain* SceneManager::createBillboardChain(const String& name)
{
    return static_cast<BillboardChain*>(
        createMovableObject(name, BillboardChainFactory::FACTORY_TYPE_NAME));
}
//-----------------------------------------------------------------------
BillboardChain* SceneManager::createBillboardChain()
{
    String name = mMovableNameGenerator.generate();
    return createBillboardChain(name);
}
//-----------------------------------------------------------------------
BillboardChain* SceneManager::getBillboardChain(const String& name) const
{
    return static_cast<BillboardChain*>(
        getMovableObject(name, BillboardChainFactory::FACTORY_TYPE_NAME));

}
//-----------------------------------------------------------------------
bool SceneManager::hasBillboardChain(const String& name) const
{
    return hasMovableObject(name, BillboardChainFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyBillboardChain(const String& name)
{
    destroyMovableObject(name, BillboardChainFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyAllBillboardChains(void)
{
    destroyAllMovableObjectsByType(BillboardChainFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
RibbonTrail* SceneManager::createRibbonTrail(const String& name)
{
    return static_cast<RibbonTrail*>(
        createMovableObject(name, RibbonTrailFactory::FACTORY_TYPE_NAME));
}
//-----------------------------------------------------------------------
RibbonTrail* SceneManager::createRibbonTrail()
{
    String name = mMovableNameGenerator.generate();
    return createRibbonTrail(name);
}
//-----------------------------------------------------------------------
RibbonTrail* SceneManager::getRibbonTrail(const String& name) const
{
    return static_cast<RibbonTrail*>(
        getMovableObject(name, RibbonTrailFactory::FACTORY_TYPE_NAME));

}
//-----------------------------------------------------------------------
bool SceneManager::hasRibbonTrail(const String& name) const
{
    return hasMovableObject(name, RibbonTrailFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyRibbonTrail(const String& name)
{
    destroyMovableObject(name, RibbonTrailFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyAllRibbonTrails(void)
{
    destroyAllMovableObjectsByType(RibbonTrailFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
ParticleSystem* SceneManager::createParticleSystem(const String& name,
    const String& templateName)
{
    NameValuePairList params;
    params["templateName"] = templateName;
    
    return static_cast<ParticleSystem*>(
        createMovableObject(name, ParticleSystemFactory::FACTORY_TYPE_NAME, 
            &params));
}
//-----------------------------------------------------------------------
ParticleSystem* SceneManager::createParticleSystem(const String& name,
    size_t quota, const String& group)
{
    NameValuePairList params;
    params["quota"] = StringConverter::toString(quota);
    params["resourceGroup"] = group;
    
    return static_cast<ParticleSystem*>(
        createMovableObject(name, ParticleSystemFactory::FACTORY_TYPE_NAME, 
            &params));
}
//-----------------------------------------------------------------------
ParticleSystem* SceneManager::createParticleSystem(size_t quota, const String& group)
{
    String name = mMovableNameGenerator.generate();
    return createParticleSystem(name, quota, group);
}

//-----------------------------------------------------------------------
ParticleSystem* SceneManager::getParticleSystem(const String& name) const
{
    return static_cast<ParticleSystem*>(
        getMovableObject(name, ParticleSystemFactory::FACTORY_TYPE_NAME));

}
//-----------------------------------------------------------------------
bool SceneManager::hasParticleSystem(const String& name) const
{
    return hasMovableObject(name, ParticleSystemFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyParticleSystem(const String& name)
{
    destroyMovableObject(name, ParticleSystemFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyAllParticleSystems(void)
{
    destroyAllMovableObjectsByType(ParticleSystemFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::clearScene(void)
{
    mShadowRenderer.destroyShadowTextures();
    destroyAllStaticGeometry();
    destroyAllInstanceManagers();
    destroyAllMovableObjects();

    // Clear root node of all children
    getRootSceneNode()->removeAllChildren();
    getRootSceneNode()->detachAllObjects();

    // Delete all SceneNodes, except root that is
    for (auto *n : mSceneNodes)
    {
        OGRE_DELETE n;
    }
    mSceneNodes.clear();
    mNamedNodes.clear();
    mAutoTrackingSceneNodes.clear();


    
    // Clear animations
    destroyAllAnimations();

    // Clear render queue, empty completely
    if (mRenderQueue)
        mRenderQueue->clear(true);

    // Reset ParamDataSource, when a resource is removed the mAutoParamDataSource keep bad references
    mAutoParamDataSource.reset(createAutoParamDataSource());
}
//-----------------------------------------------------------------------
SceneNode* SceneManager::createSceneNodeImpl(void)
{
    return OGRE_NEW SceneNode(this);
}
//-----------------------------------------------------------------------
SceneNode* SceneManager::createSceneNodeImpl(const String& name)
{
    return OGRE_NEW SceneNode(this, name);
}//-----------------------------------------------------------------------
SceneNode* SceneManager::createSceneNode(void)
{
    SceneNode* sn = createSceneNodeImpl();
    mSceneNodes.push_back(sn);
    sn->mGlobalIndex = mSceneNodes.size() - 1;
    return sn;
}
//-----------------------------------------------------------------------
SceneNode* SceneManager::createSceneNode(const String& name)
{
    // Check name not used
    if (hasSceneNode(name))
    {
        OGRE_EXCEPT(
            Exception::ERR_DUPLICATE_ITEM,
            "A scene node with the name " + name + " already exists",
            "SceneManager::createSceneNode" );
    }

    SceneNode* sn = createSceneNodeImpl(name);
    mSceneNodes.push_back(sn);
    mNamedNodes[name] = sn;
    sn->mGlobalIndex = mSceneNodes.size() - 1;
    return sn;
}
//-----------------------------------------------------------------------
void SceneManager::destroySceneNode(const String& name)
{
    OgreAssert(!name.empty(), "name must not be empty");
    auto i = mNamedNodes.find(name);
    destroySceneNode(i != mNamedNodes.end() ? i->second : NULL);
}

void SceneManager::_destroySceneNode(SceneNodeList::iterator i)
{

    if (i == mSceneNodes.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "SceneNode not found.",
                    "SceneManager::_destroySceneNode");
    }

    // Find any scene nodes which are tracking this node, and turn them off
    AutoTrackingSceneNodes::iterator ai, aiend;
    aiend = mAutoTrackingSceneNodes.end();
    for (ai = mAutoTrackingSceneNodes.begin(); ai != aiend; )
    {
        // Pre-increment incase we delete
        AutoTrackingSceneNodes::iterator curri = ai++;
        SceneNode* n = *curri;
        // Tracking this node
        if (n->getAutoTrackTarget() == *i)
        {
            // turn off, this will notify SceneManager to remove
            n->setAutoTracking(false);
        }
        // node is itself a tracker
        else if (n == *i)
        {
            mAutoTrackingSceneNodes.erase(curri);
        }
    }

    // detach from parent (don't do this in destructor since bulk destruction
    // behaves differently)
    Node* parentNode = (*i)->getParent();
    if (parentNode)
    {
        parentNode->removeChild(*i);
    }
    if(!(*i)->getName().empty())
        mNamedNodes.erase((*i)->getName());
    OGRE_DELETE *i;
    if (std::next(i) != mSceneNodes.end())
    {
       std::swap(*i, mSceneNodes.back());
       (*i)->mGlobalIndex = i - mSceneNodes.begin();
    }
    mSceneNodes.pop_back();
}
//---------------------------------------------------------------------
void SceneManager::destroySceneNode(SceneNode* sn)
{
    OgreAssert(sn, "Cannot destroy a null SceneNode");

    auto pos = sn->mGlobalIndex < mSceneNodes.size() &&
                       sn == *(mSceneNodes.begin() + sn->mGlobalIndex)
                   ? mSceneNodes.begin() + sn->mGlobalIndex
                   : mSceneNodes.end();

    _destroySceneNode(pos);
}
//-----------------------------------------------------------------------
SceneNode* SceneManager::getRootSceneNode(void)
{
    if (!mSceneRoot)
    {
        // Create root scene node
        mSceneRoot.reset(createSceneNodeImpl("Ogre/SceneRoot"));
        mSceneRoot->_notifyRootNode();
    }

    return mSceneRoot.get();
}
//-----------------------------------------------------------------------
SceneNode* SceneManager::getSceneNode(const String& name, bool throwExceptionIfNotFound) const
{
    OgreAssert(!name.empty(), "name must not be empty");
    auto i = mNamedNodes.find(name);

    if (i != mNamedNodes.end())
        return i->second;

    if (throwExceptionIfNotFound)
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "SceneNode '" + name + "' not found.");
    return NULL;
}

//-----------------------------------------------------------------------
const Pass* SceneManager::_setPass(const Pass* pass, bool evenIfSuppressed, 
                                   bool shadowDerivation)
{
    //If using late material resolving, swap now.
    if (isLateMaterialResolving()) 
    {
        Technique* lateTech = pass->getParent()->getParent()->getBestTechnique();
        if (lateTech->getNumPasses() > pass->getIndex())
        {
            pass = lateTech->getPass(pass->getIndex());
        }
        else
        {
            pass = lateTech->getPass(0);
        }
        //Should we warn or throw an exception if an illegal state was achieved?
    }

    if (mIlluminationStage == IRS_RENDER_TO_TEXTURE && shadowDerivation)
    {
        // Derive a special shadow caster pass from this one
        pass = mShadowRenderer.deriveShadowCasterPass(pass);
    }
    else if (mIlluminationStage == IRS_RENDER_RECEIVER_PASS && shadowDerivation)
    {
        pass = mShadowRenderer.deriveShadowReceiverPass(pass);
    }

    // Tell params about current pass
    mAutoParamDataSource->setCurrentPass(pass);

    GpuProgram* vprog = pass->hasVertexProgram() ? pass->getVertexProgram().get() : 0;
    GpuProgram* fprog = pass->hasFragmentProgram() ? pass->getFragmentProgram().get() : 0;

    bool passSurfaceAndLightParams = !vprog || vprog->getPassSurfaceAndLightStates();

    if (vprog)
    {
        bindGpuProgram(vprog->_getBindingDelegate());
    }
    else if (!mDestRenderSystem->getCapabilities()->hasCapability(RSC_FIXED_FUNCTION))
    {
        OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                    "RenderSystem does not support FixedFunction, "
                    "but technique of '" +
                        pass->getParent()->getParent()->getName() +
                        "' has no Vertex Shader. Use the RTSS or write custom shaders.",
                    "SceneManager::_setPass");
    }
    else
    {
        // Unbind program?
        if (mDestRenderSystem->isGpuProgramBound(GPT_VERTEX_PROGRAM))
        {
            mDestRenderSystem->unbindGpuProgram(GPT_VERTEX_PROGRAM);
        }
        // Set fixed-function vertex parameters
    }

    if (pass->hasGeometryProgram())
    {
        bindGpuProgram(pass->getGeometryProgram()->_getBindingDelegate());
        // bind parameters later
    }
    else
    {
        // Unbind program?
        if (mDestRenderSystem->isGpuProgramBound(GPT_GEOMETRY_PROGRAM))
        {
            mDestRenderSystem->unbindGpuProgram(GPT_GEOMETRY_PROGRAM);
        }
    }
    if (pass->hasTessellationHullProgram())
    {
        bindGpuProgram(pass->getTessellationHullProgram()->_getBindingDelegate());
        // bind parameters later
    }
    else
    {
        // Unbind program?
        if (mDestRenderSystem->isGpuProgramBound(GPT_HULL_PROGRAM))
        {
            mDestRenderSystem->unbindGpuProgram(GPT_HULL_PROGRAM);
        }
    }

    if (pass->hasTessellationDomainProgram())
    {
        bindGpuProgram(pass->getTessellationDomainProgram()->_getBindingDelegate());
        // bind parameters later
    }
    else
    {
        // Unbind program?
        if (mDestRenderSystem->isGpuProgramBound(GPT_DOMAIN_PROGRAM))
        {
            mDestRenderSystem->unbindGpuProgram(GPT_DOMAIN_PROGRAM);
        }
    }

    if (pass->hasComputeProgram())
    {
        bindGpuProgram(pass->getComputeProgram()->_getBindingDelegate());
        // bind parameters later
    }
    else
    {
        // Unbind program?
        if (mDestRenderSystem->isGpuProgramBound(GPT_COMPUTE_PROGRAM))
        {
            mDestRenderSystem->unbindGpuProgram(GPT_COMPUTE_PROGRAM);
        }
    }

    if (passSurfaceAndLightParams)
    {
        // Dynamic lighting enabled?
        mDestRenderSystem->setLightingEnabled(pass->getLightingEnabled());
    }

    // Using a fragment program?
    if (fprog)
    {
        bindGpuProgram(fprog->_getBindingDelegate());
    }
    else if (!mDestRenderSystem->getCapabilities()->hasCapability(RSC_FIXED_FUNCTION) &&
             !pass->hasGeometryProgram())
    {
        OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                    "RenderSystem does not support FixedFunction, "
                    "but technique of '" +
                        pass->getParent()->getParent()->getName() +
                        "' has no Fragment Shader. Use the RTSS or write custom shaders.",
                    "SceneManager::_setPass");
    }
    else
    {
        // Unbind program?
        if (mDestRenderSystem->isGpuProgramBound(GPT_FRAGMENT_PROGRAM))
        {
            mDestRenderSystem->unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
        }
        // Set fixed-function fragment settings
    }

    // fog params can either be from scene or from material
    const auto& newFogColour = pass->getFogOverride() ? pass->getFogColour() : mFogColour;
    FogMode newFogMode;
    Real newFogStart, newFogEnd, newFogDensity;
    if (pass->getFogOverride())
    {
        // fog params from material
        newFogMode = pass->getFogMode();
        newFogStart = pass->getFogStart();
        newFogEnd = pass->getFogEnd();
        newFogDensity = pass->getFogDensity();
    }
    else
    {
        // fog params from scene
        newFogMode = mFogMode;
        newFogStart = mFogStart;
        newFogEnd = mFogEnd;
        newFogDensity = mFogDensity;
    }

    mAutoParamDataSource->setFog(newFogMode, newFogColour, newFogDensity, newFogStart, newFogEnd);

    // The rest of the settings are the same no matter whether we use programs or not

    if(mDestRenderSystem->getCapabilities()->hasCapability(RSC_FIXED_FUNCTION) && passSurfaceAndLightParams)
    {
        mFixedFunctionParams = mDestRenderSystem->getFixedFunctionParams(pass->getVertexColourTracking(), newFogMode);
    }

    // Set scene blending
    mDestRenderSystem->setColourBlendState(pass->getBlendState());

    // Line width
    if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_WIDE_LINES))
        mDestRenderSystem->_setLineWidth(pass->getLineWidth());

    // Set point parameters
    mDestRenderSystem->_setPointParameters(pass->isPointAttenuationEnabled(), pass->getPointMinSize(),
                                           pass->getPointMaxSize());

    if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_POINT_SPRITES))
        mDestRenderSystem->_setPointSpritesEnabled(pass->getPointSpritesEnabled());

    mAutoParamDataSource->setPointParameters(pass->isPointAttenuationEnabled(), pass->getPointAttenuation());

    // Texture unit settings
    size_t unit = 0;
    // Reset the shadow texture index for each pass
    size_t startLightIndex = pass->getStartLight();
    size_t shadowTexUnitIndex = 0;
    size_t shadowTexIndex = mShadowRenderer.getShadowTexIndex(startLightIndex);
    for(auto *pTex : pass->getTextureUnitStates())
    {
        if (!pass->getIteratePerLight() && isShadowTechniqueTextureBased() &&
            pTex->getContentType() == TextureUnitState::CONTENT_SHADOW)
        {
            // Need to bind the correct shadow texture, based on the start light
            // Even though the light list can change per object, our restrictions
            // say that when texture shadows are enabled, the lights up to the
            // number of texture shadows will be fixed for all objects
            // to match the shadow textures that have been generated
            // see Listener::sortLightsAffectingFrustum and
            // MovableObject::Listener::objectQueryLights
            // Note that light iteration throws the indexes out so we don't bind here
            // if that's the case, we have to bind when lights are iterated
            // in renderSingleObject

            mShadowRenderer.resolveShadowTexture(pTex, shadowTexIndex, shadowTexUnitIndex);
            ++shadowTexIndex;
            ++shadowTexUnitIndex;
        }
        else if (mIlluminationStage == IRS_NONE)
        {
            // Manually set texture projector for shaders if present
            // This won't get set any other way if using manual projection
            auto effi = pTex->getEffects().find(TextureUnitState::ET_PROJECTIVE_TEXTURE);
            if (effi != pTex->getEffects().end())
            {
                mAutoParamDataSource->setTextureProjector(effi->second.frustum, unit);
            }
        }
        if (pTex->getContentType() == TextureUnitState::CONTENT_COMPOSITOR)
        {
            CompositorChain* currentChain = _getActiveCompositorChain();
            if (!currentChain)
                OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                            "TextureUnitState references a compositor, but current viewport of '" +
                                mCurrentViewport->getTarget()->getName() + "' does not have a CompositorChain");
            auto compName = pTex->getReferencedCompositorName();
            CompositorInstance* refComp = currentChain->getCompositor(compName);
            if (!refComp)
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Current CompositorChain does not contain compositor named " + compName);

            auto texName = pTex->getReferencedTextureName();
            TexturePtr refTex = refComp->getTextureInstance(texName, pTex->getReferencedMRTIndex());

            if (!refTex)
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Compositor " + compName + " does not declare texture " + texName);
            pTex->_setTexturePtr(refTex);
        }
        mDestRenderSystem->_setTextureUnitSettings(unit, *pTex);
        ++unit;
    }
    // Disable remaining texture units
    mDestRenderSystem->_disableTextureUnitsFrom(pass->getNumTextureUnitStates());

    // Set up non-texture related material settings
    // Depth buffer settings
    mDestRenderSystem->_setDepthBufferParams(pass->getDepthCheckEnabled(), pass->getDepthWriteEnabled(),
                                             pass->getDepthFunction());
    mDestRenderSystem->_setDepthBias(pass->getDepthBiasConstant(), pass->getDepthBiasSlopeScale());
    // Alpha-reject settings
    mDestRenderSystem->_setAlphaRejectSettings(pass->getAlphaRejectFunction(),
                                               pass->getAlphaRejectValue(),
                                               pass->isAlphaToCoverageEnabled());

    // Culling mode
    if (isShadowTechniqueTextureBased() && mIlluminationStage == IRS_RENDER_TO_TEXTURE &&
        mShadowRenderer.mShadowCasterRenderBackFaces && pass->getCullingMode() == CULL_CLOCKWISE)
    {
        // render back faces into shadow caster, can help with depth comparison
        mPassCullingMode = CULL_ANTICLOCKWISE;
    }
    else
    {
        mPassCullingMode = pass->getCullingMode();
    }
    mDestRenderSystem->_setCullingMode(mPassCullingMode);
    mDestRenderSystem->setShadingType(pass->getShadingMode());

    mAutoParamDataSource->setPassNumber( pass->getIndex() );
    // mark global params as dirty
    mGpuParamsDirty |= (uint16)GPV_GLOBAL;

    return pass;
}
//-----------------------------------------------------------------------
void SceneManager::prepareRenderQueue(void)
{
    RenderQueue* q = getRenderQueue();
    // Clear the render queue
    q->clear(Root::getSingleton().getRemoveRenderQueueStructuresOnClear());

    // Prep the ordering options
    // We need this here to reset if coming out of a render queue sequence,
    // but doing it resets any specialised settings set globally per render queue
    // so only do it when necessary - it's nice to allow people to set the organisation
    // mode manually for example

    // Default all the queue groups that are there, new ones will be created
    // with defaults too
    for (size_t i = 0; i < RENDER_QUEUE_COUNT; ++i)
    {
        if(!q->_getQueueGroups()[i])
            continue;

        q->_getQueueGroups()[i]->defaultOrganisationMode();
    }

    // Global split options
    mShadowRenderer.updateSplitOptions(q);
}
//-----------------------------------------------------------------------
void SceneManager::_renderScene(Camera* camera, Viewport* vp, bool includeOverlays)
{
    assert(camera);
    OgreProfileGroup(camera->getName(), OGREPROF_GENERAL);

    auto prevSceneManager = Root::getSingleton()._getCurrentSceneManager();
    Root::getSingleton()._setCurrentSceneManager(this);
    mActiveQueuedRenderableVisitor->targetSceneMgr = this;
    mAutoParamDataSource->setCurrentSceneManager(this);

    // preserve the previous scheme, in case this is a RTT update with an outer _renderScene pending
    MaterialManager& matMgr = MaterialManager::getSingleton();
    String prevMaterialScheme = matMgr.getActiveScheme();

    // Also set the internal viewport pointer at this point, for calls that need it
    // However don't call setViewport just yet (see below)
    mCurrentViewport = vp;

	// Set current draw buffer (default is CBT_BACK)
	mDestRenderSystem->setDrawBuffer(mCurrentViewport->getDrawBuffer());
	
    // reset light hash so even if light list is the same, we refresh the content every frame
    useLights(NULL, 0);

    // Perform a quick pre-check to see whether we should override far distance
    // When using stencil volumes we have to use infinite far distance
    // to prevent dark caps getting clipped
    if (isShadowTechniqueStencilBased() && 
        camera->getProjectionType() == PT_PERSPECTIVE &&
        camera->getFarClipDistance() != 0 &&
        mShadowRenderer.mShadowUseInfiniteFarPlane)
    {
        // infinite far distance
        camera->setFarClipDistance(0);
    }

    mCameraInProgress = camera;


    // Update controllers 
    ControllerManager::getSingleton().updateAllControllers();

    // Update the scene, only do this once per frame
    unsigned long thisFrameNumber = Root::getSingleton().getNextFrameNumber();
    if (thisFrameNumber != mLastFrameNumber)
    {
        // Update animations
        _applySceneAnimations();
        updateDirtyInstanceManagers();
        mLastFrameNumber = thisFrameNumber;
    }

    {
        // Lock scene graph mutex, no more changes until we're ready to render
            OGRE_LOCK_MUTEX(sceneGraphMutex);

        // Update scene graph for this camera (can happen multiple times per frame)
        {
            OgreProfileGroup("_updateSceneGraph", OGREPROF_GENERAL);
            _updateSceneGraph(camera);

            // Auto-track nodes
            for (auto *a : mAutoTrackingSceneNodes)
            {
                a->_autoTrack();
            }
#ifdef OGRE_NODELESS_POSITIONING
            // Auto-track camera if required
            OGRE_IGNORE_DEPRECATED_BEGIN
            camera->_autoTrack();
            OGRE_IGNORE_DEPRECATED_END
#endif
        }

        if (mIlluminationStage != IRS_RENDER_TO_TEXTURE && mFindVisibleObjects)
        {
            // Locate any lights which could be affecting the frustum
            findLightsAffectingFrustum(camera);

            // Prepare shadow textures if texture shadow based shadowing
            // technique in use
            if (isShadowTechniqueTextureBased() && vp->getShadowsEnabled())
            {
                OgreProfileGroup("prepareShadowTextures", OGREPROF_GENERAL);

                // *******
                // WARNING
                // *******
                // This call will result in re-entrant calls to this method
                // therefore anything which comes before this is NOT
                // guaranteed persistent. Make sure that anything which
                // MUST be specific to this camera / target is done
                // AFTER THIS POINT
                prepareShadowTextures(camera, vp);
                // reset the cameras & viewport because of the re-entrant call
                mCameraInProgress = camera;
                mCurrentViewport = vp;
            }
        }

        // Invert vertex winding?
        mDestRenderSystem->setInvertVertexWinding(camera->isReflected());

        // Set the viewport - this is deliberately after the shadow texture update
        setViewport(vp);

        // Tell params about camera
        mAutoParamDataSource->setCurrentCamera(camera, mCameraRelativeRendering);
        // Set autoparams for finite dir light extrusion
        mAutoParamDataSource->setShadowDirLightExtrusionDistance(mShadowRenderer.mShadowDirLightExtrudeDist);

        // Tell params about render target
        mAutoParamDataSource->setCurrentRenderTarget(vp->getTarget());


        // Set camera window clipping planes (if any)
        if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_USER_CLIP_PLANES))
        {
            mDestRenderSystem->setClipPlanes(camera->isWindowSet() ? camera->getWindowPlanes() : PlaneList());
        }

        // Prepare render queue for receiving new objects
        {
            OgreProfileGroup("prepareRenderQueue", OGREPROF_GENERAL);
            prepareRenderQueue();
        }

        if (mFindVisibleObjects)
        {
            OgreProfileGroup("_findVisibleObjects", OGREPROF_CULLING);

            // Assemble an AAB on the fly which contains the scene elements visible
            // by the camera.
            CamVisibleObjectsMap::iterator camVisObjIt = mCamVisibleObjectsMap.find( camera );

            assert (camVisObjIt != mCamVisibleObjectsMap.end() &&
                "Should never fail to find a visible object bound for a camera, "
                "did you override SceneManager::createCamera or something?");

            // reset the bounds
            camVisObjIt->second.reset();

            // Parse the scene and tag visibles
            firePreFindVisibleObjects(vp);
            _findVisibleObjects(camera, &(camVisObjIt->second),
                mIlluminationStage == IRS_RENDER_TO_TEXTURE? true : false);
            firePostFindVisibleObjects(vp);

            mAutoParamDataSource->setMainCamBoundsInfo(&(camVisObjIt->second));
        }
    } // end lock on scene graph mutex

    mDestRenderSystem->_beginGeometryCount();
    // Clear the viewport if required
    if (mCurrentViewport->getClearEveryFrame())
    {
        mDestRenderSystem->clearFrameBuffer(
            mCurrentViewport->getClearBuffers(), 
            mCurrentViewport->getBackgroundColour(),
            mCurrentViewport->getDepthClear() );
    }        
    // Begin the frame
    mDestRenderSystem->_beginFrame();

    mDestRenderSystem->_setTextureProjectionRelativeTo(mCameraRelativeRendering, camera->getDerivedPosition());

    // Render scene content
    {
        OgreProfileGroup("_renderVisibleObjects", OGREPROF_RENDERING);
        _renderVisibleObjects();
    }

    // End frame
    mDestRenderSystem->_endFrame();

    // Notify camera of vis faces
    camera->_notifyRenderedFaces(mDestRenderSystem->_getFaceCount());

    // Notify camera of vis batches
    camera->_notifyRenderedBatches(mDestRenderSystem->_getBatchCount());

    matMgr.setActiveScheme(prevMaterialScheme);
    Root::getSingleton()._setCurrentSceneManager(prevSceneManager);
}
//-----------------------------------------------------------------------
void SceneManager::_setDestinationRenderSystem(RenderSystem* sys)
{
    mDestRenderSystem = sys;
    mShadowRenderer.mDestRenderSystem = sys;
}
//-----------------------------------------------------------------------
void SceneManager::_releaseManualHardwareResources()
{
    // release stencil shadows index buffer
    mShadowRenderer.mShadowIndexBuffer.reset();

    // release hardware resources inside all movable objects
    OGRE_LOCK_MUTEX(mMovableObjectCollectionMapMutex);
    for (const auto& m : mMovableObjectCollectionMap)
    {
        MovableObjectCollection* coll = m.second;
        OGRE_LOCK_MUTEX(coll->mutex);
        for(auto& i : coll->map)
            i.second->_releaseManualHardwareResources();
    }
}
//-----------------------------------------------------------------------
void SceneManager::_restoreManualHardwareResources()
{
    // restore stencil shadows index buffer
    if(isShadowTechniqueStencilBased())
    {
        mShadowRenderer.mShadowIndexBuffer = HardwareBufferManager::getSingleton().
            createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
                mShadowRenderer.mShadowIndexBufferSize,
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
                false);
    }

    // restore hardware resources inside all movable objects
    OGRE_LOCK_MUTEX(mMovableObjectCollectionMapMutex);
    for(const auto& m : mMovableObjectCollectionMap)
    {
        MovableObjectCollection* coll = m.second;
        OGRE_LOCK_MUTEX(coll->mutex);
        for(auto& i : coll->map)
            i.second->_restoreManualHardwareResources();
    }
}
//-----------------------------------------------------------------------
void SceneManager::setWorldGeometry(const String& filename)
{
    // This default implementation cannot handle world geometry
    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
        "World geometry is not supported by the generic SceneManager.",
        "SceneManager::setWorldGeometry");
}
//-----------------------------------------------------------------------
void SceneManager::setWorldGeometry(DataStreamPtr& stream, 
    const String& typeName)
{
    // This default implementation cannot handle world geometry
    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
        "World geometry is not supported by the generic SceneManager.",
        "SceneManager::setWorldGeometry");
}
//-----------------------------------------------------------------------
void SceneManager::setSkyPlane(
                               bool enable,
                               const Plane& plane,
                               const String& materialName,
                               Real gscale,
                               Real tiling,
                               bool drawFirst,
                               Real bow,
                               int xsegments, int ysegments,
                               const String& groupName)
{
    _setSkyPlane(enable, plane, materialName, gscale, tiling,
                 drawFirst ? RENDER_QUEUE_SKIES_EARLY : RENDER_QUEUE_SKIES_LATE, bow, xsegments, ysegments,
                 groupName);
}

void SceneManager::_setSkyPlane(bool enable, const Plane& plane, const String& materialName,
                                Real gscale, Real tiling, uint8 renderQueue, Real bow,
                                int xsegments, int ysegments, const String& groupName)
{
    mSkyPlane.setSkyPlane(enable, plane, materialName, gscale, tiling, renderQueue, bow,
                             xsegments, ysegments, groupName);
}

//-----------------------------------------------------------------------
void SceneManager::setSkyBox(
                             bool enable,
                             const String& materialName,
                             Real distance,
                             bool drawFirst,
                             const Quaternion& orientation,
                             const String& groupName)
{
    _setSkyBox(enable, materialName, distance,
               drawFirst ? RENDER_QUEUE_SKIES_EARLY : RENDER_QUEUE_SKIES_LATE, orientation, groupName);
}

void SceneManager::_setSkyBox(bool enable, const String& materialName, Real distance,
                              uint8 renderQueue, const Quaternion& orientation,
                              const String& groupName)
{
    mSkyBox.setSkyBox(enable, materialName, distance, renderQueue, orientation, groupName);
}

//-----------------------------------------------------------------------
void SceneManager::setSkyDome(
                              bool enable,
                              const String& materialName,
                              Real curvature,
                              Real tiling,
                              Real distance,
                              bool drawFirst,
                              const Quaternion& orientation,
                              int xsegments, int ysegments, int ySegmentsToKeep,
                              const String& groupName)
{
    _setSkyDome(enable, materialName, curvature, tiling, distance,
                drawFirst ? RENDER_QUEUE_SKIES_EARLY : RENDER_QUEUE_SKIES_LATE, orientation, xsegments,
                ysegments, ySegmentsToKeep, groupName);
}

void SceneManager::_setSkyDome(bool enable, const String& materialName, Real curvature, Real tiling,
                               Real distance, uint8 renderQueue, const Quaternion& orientation,
                               int xsegments, int ysegments, int ysegments_keep,
                               const String& groupName)
{
    mSkyDome.setSkyDome(enable, materialName, curvature, tiling, distance, renderQueue,
                            orientation, xsegments, ysegments, ysegments_keep, groupName);
}

//-----------------------------------------------------------------------
void SceneManager::_updateSceneGraph(Camera* cam)
{
    firePreUpdateSceneGraph(cam);

    // Process queued needUpdate calls 
    Node::processQueuedUpdates();

    // Cascade down the graph updating transforms & world bounds
    // In this implementation, just update from the root
    // Smarter SceneManager subclasses may choose to update only
    //   certain scene graph branches
    getRootSceneNode()->_update(true, false);

    firePostUpdateSceneGraph(cam);
}
//-----------------------------------------------------------------------
void SceneManager::_findVisibleObjects(
    Camera* cam, VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters)
{
    // Tell nodes to find, cascade down all nodes
    getRootSceneNode()->_findVisibleObjects(cam, getRenderQueue(), visibleBounds, true, 
        mDisplayNodes, onlyShadowCasters);

}
//-----------------------------------------------------------------------
void SceneManager::renderVisibleObjectsDefaultSequence(void)
{
    firePreRenderQueues();

    // Render each separate queue
    const RenderQueue::RenderQueueGroupMap& groups = getRenderQueue()->_getQueueGroups();

    for (uint8 qId = 0; qId < RENDER_QUEUE_COUNT; ++qId)
    {
        if(!groups[qId])
            continue;

        // Get queue group id
        RenderQueueGroup* pGroup = groups[qId].get();
        // Skip this one if not to be processed
        if (!isRenderQueueToBeProcessed(qId))
            continue;


        bool repeatQueue = false;
        do // for repeating queues
        {
            // Fire queue started event
            if (fireRenderQueueStarted(qId, mCameraInProgress->getName()))
            {
                // Someone requested we skip this queue
                break;
            }

            _renderQueueGroupObjects(pGroup, QueuedRenderableCollection::OM_PASS_GROUP);

            // Fire queue ended event
            if (fireRenderQueueEnded(qId, mCameraInProgress->getName()))
            {
                // Someone requested we repeat this queue
                repeatQueue = true;
            }
            else
            {
                repeatQueue = false;
            }
        } while (repeatQueue);

    } // for each queue group

    firePostRenderQueues();

}
//-----------------------------------------------------------------------
void SceneManager::SceneMgrQueuedRenderableVisitor::visit(const Pass* p, RenderableList& rs)
{
    // Give SM a chance to eliminate this pass
    if (!targetSceneMgr->validatePassForRendering(p))
        return;

    OgreProfileBeginGPUEvent(p->getParent()->getParent()->getName());
    // Set pass, store the actual one used
    mUsedPass = targetSceneMgr->_setPass(p);

    SubMesh* lastsm = 0;
    RenderableList instances;

    bool useInstancing = mUsedPass->hasVertexProgram() && mUsedPass->getVertexProgram()->isInstancingIncluded();

    for (Renderable* r : rs)
    {
        // Give SM a chance to eliminate
        if (!targetSceneMgr->validateRenderableForRendering(mUsedPass, r))
            continue;

        if(useInstancing)
        {
            if(auto se = dynamic_cast<SubEntity*>(r))
            {
                if(lastsm == se->getSubMesh())
                {
                    instances.push_back(r);
                    continue;
                }

                // different SubMesh -> flush and restart instances
                if(!instances.empty())
                {
                    targetSceneMgr->renderInstancedObject(instances, mUsedPass, scissoring, autoLights, manualLightList);
                    instances.clear();
                }

                lastsm = se->getSubMesh();
                instances.push_back(r);
                continue;
            }

            // not a SubEntity -> flush instances and render this one
            if(!instances.empty())
            {
                targetSceneMgr->renderInstancedObject(instances, mUsedPass, scissoring, autoLights, manualLightList);
                instances.clear();
            }
        }

        // Render a single object, this will set up auto params if required
        targetSceneMgr->renderSingleObject(r, mUsedPass, scissoring, autoLights, manualLightList);
    }

    if (!instances.empty())
        targetSceneMgr->renderInstancedObject(instances, mUsedPass, scissoring, autoLights, manualLightList);

    OgreProfileEndGPUEvent(p->getParent()->getParent()->getName());
}
//-----------------------------------------------------------------------
void SceneManager::SceneMgrQueuedRenderableVisitor::visit(RenderablePass* rp)
{
    // Skip this one if we're in transparency cast shadows mode & it doesn't
    // Don't need to implement this one in the other visit methods since
    // transparents are never grouped, always sorted
    if (transparentShadowCastersMode && 
        !rp->pass->getParent()->getParent()->getTransparencyCastsShadows())
        return;

    // Give SM a chance to eliminate
    if (targetSceneMgr->validateRenderableForRendering(rp->pass, rp->renderable))
    {
        mUsedPass = targetSceneMgr->_setPass(rp->pass);
        OgreProfileBeginGPUEvent(mUsedPass->getParent()->getParent()->getName());
        targetSceneMgr->renderSingleObject(rp->renderable, mUsedPass, scissoring, 
            autoLights, manualLightList);
        OgreProfileEndGPUEvent(mUsedPass->getParent()->getParent()->getName());
    }
}
//-----------------------------------------------------------------------
void SceneManager::SceneMgrQueuedRenderableVisitor::renderObjects(const QueuedRenderableCollection& objs,
                                                    QueuedRenderableCollection::OrganisationMode om,
                                                    bool lightScissoringClipping, bool doLightIteration,
                                                    const LightList* _manualLightList, bool _transparentShadowCastersMode)
{
    autoLights = doLightIteration;
    manualLightList = _manualLightList;
    transparentShadowCastersMode = _transparentShadowCastersMode;
    scissoring = lightScissoringClipping;
    // Use visitor
    objs.acceptVisitor(this, om);
    transparentShadowCastersMode = false;
}

void SceneManager::SceneMgrQueuedRenderableVisitor::renderTransparents(const RenderPriorityGroup* priorityGrp,
                                                                       QueuedRenderableCollection::OrganisationMode om)
{
    // Do unsorted transparents
    renderObjects(priorityGrp->getTransparentsUnsorted(), om, true, true);
    // Do transparents (always descending sort)
    renderObjects(priorityGrp->getTransparents(), QueuedRenderableCollection::OM_SORT_DESCENDING, true, true);
}

//-----------------------------------------------------------------------
bool SceneManager::validatePassForRendering(const Pass* pass)
{
    // Bypass if we're doing a texture shadow render and 
    // this pass is after the first (only 1 pass needed for shadow texture render, and 
    // one pass for shadow texture receive for modulative technique)
    // Also bypass if passes above the first if render state changes are
    // suppressed since we're not actually using this pass data anyway
    if (mCurrentViewport->getShadowsEnabled() &&
        ((isShadowTechniqueModulative() && mIlluminationStage == IRS_RENDER_RECEIVER_PASS) ||
         mIlluminationStage == IRS_RENDER_TO_TEXTURE) &&
        pass->getIndex() > 0)
    {
        return false;
    }

    // If using late material resolving, check if there is a pass with the same index
    // as this one in the 'late' material. If not, skip.
    if (isLateMaterialResolving())
    {
        Technique* lateTech = pass->getParent()->getParent()->getBestTechnique();
        if (lateTech->getNumPasses() <= pass->getIndex())
        {
            return false;
        }
    }

    return true;
}
//-----------------------------------------------------------------------
bool SceneManager::validateRenderableForRendering(const Pass* pass, const Renderable* rend)
{
    // Skip this renderable if we're doing modulative texture shadows, it casts shadows
    // and we're doing the render receivers pass and we're not self-shadowing
    // also if pass number > 0
    if (mCurrentViewport->getShadowsEnabled() && isShadowTechniqueTextureBased())
    {
        if (mIlluminationStage == IRS_RENDER_RECEIVER_PASS && 
            rend->getCastsShadows() && !mShadowRenderer.mShadowTextureSelfShadow)
        {
            return false;
        }
        // Some duplication here with validatePassForRendering, for transparents
        if (((isShadowTechniqueModulative() && mIlluminationStage == IRS_RENDER_RECEIVER_PASS) ||
             mIlluminationStage == IRS_RENDER_TO_TEXTURE) &&
            pass->getIndex() > 0)
        {
            return false;
        }
    }

    return true;

}
//-----------------------------------------------------------------------
void SceneManager::_renderQueueGroupObjects(RenderQueueGroup* pGroup, 
                                           QueuedRenderableCollection::OrganisationMode om)
{
    bool doShadows = pGroup->getShadowsEnabled() && mCurrentViewport->getShadowsEnabled();

    // Modulative texture shadows in use
    if (isShadowTechniqueTextureBased() && mIlluminationStage == IRS_RENDER_TO_TEXTURE)
    {
        // Shadow caster pass
        if (mCurrentViewport->getShadowsEnabled())
        {
            mShadowRenderer.renderTextureShadowCasterQueueGroupObjects(pGroup, om);
        }
        return;
    }

    // Ordinary + receiver pass
    if (doShadows && mShadowRenderer.mShadowTechnique && !isShadowTechniqueIntegrated())
    {
        mShadowRenderer.render(pGroup, om);
        return;
    }

    // No shadows, ordinary pass
    renderBasicQueueGroupObjects(pGroup, om);
}
//-----------------------------------------------------------------------
void SceneManager::renderBasicQueueGroupObjects(RenderQueueGroup* pGroup, 
                                                QueuedRenderableCollection::OrganisationMode om)
{
    // Basic render loop
    // Iterate through priorities
    auto visitor = mActiveQueuedRenderableVisitor;

    for (const auto& pg : pGroup->getPriorityGroups())
    {
        RenderPriorityGroup* pPriorityGrp = pg.second;

        // Sort the queue first
        pPriorityGrp->sort(mCameraInProgress);

        // Do solids
        visitor->renderObjects(pPriorityGrp->getSolidsBasic(), om, true, true);
        visitor->renderTransparents(pPriorityGrp, om);
    }// for each priority
}
//-----------------------------------------------------------------------
void SceneManager::setWorldTransform(Renderable* rend)
{
    // Issue view / projection changes if any
    // Check view matrix
    if (rend->getUseIdentityView())
    {
        mGpuParamsDirty |= (uint16)GPV_GLOBAL;
        mResetIdentityView = true;
    }

    if (rend->getUseIdentityProjection())
    {
        mGpuParamsDirty |= (uint16)GPV_GLOBAL;

        mResetIdentityProj = true;
    }

    // mark per-object params as dirty
    mGpuParamsDirty |= (uint16)GPV_PER_OBJECT;
}
//-----------------------------------------------------------------------
void SceneManager::issueRenderWithLights(Renderable* rend, const Pass* pass,
                                         const LightList* pLightListToUse,
                                         bool lightScissoringClipping)
{
    useLights(pLightListToUse, pass->getMaxSimultaneousLights());
    fireRenderSingleObject(rend, pass, mAutoParamDataSource.get(), pLightListToUse, false);

    // optional light scissoring & clipping
    ClipResult scissored = CLIPPED_NONE;
    ClipResult clipped = CLIPPED_NONE;
    if (pLightListToUse && lightScissoringClipping &&
        (pass->getLightScissoringEnabled() || pass->getLightClipPlanesEnabled()))
    {
        // if there's no lights hitting the scene, then we might as
        // well stop since clipping cannot include anything
        if (pLightListToUse->empty())
            return;

        if (pass->getLightScissoringEnabled())
            scissored = buildAndSetScissor(*pLightListToUse, mCameraInProgress);

        if (pass->getLightClipPlanesEnabled())
            clipped = buildAndSetLightClip(*pLightListToUse);

        if (scissored == CLIPPED_ALL || clipped == CLIPPED_ALL)
            return;
    }

    // nfz: set up multipass rendering
    mDestRenderSystem->setCurrentPassIterationCount(pass->getPassIterationCount());
    _issueRenderOp(rend, pass);

     if (scissored == CLIPPED_SOME)
         resetScissor();
     if (clipped == CLIPPED_SOME)
         resetLightClip();
}
//-----------------------------------------------------------------------
static void injectGlobalInstancingDeclaration(RenderOperation& ro, const RenderSystem* rs)
{
    if (!ro.useGlobalInstancingVertexBufferIsAvailable)
        return;

    // Create variables related to instancing.
    VertexDeclaration* instanceDecl = rs->getGlobalInstanceVertexBufferVertexDeclaration();

    if(!instanceDecl || instanceDecl->getElements().empty())
        return;

    auto instancingSrc = ro.vertexData->vertexDeclaration->getMaxSource();

    auto testElement = instanceDecl->getElements().front();
    if(!ro.vertexData->vertexDeclaration->findElementBySemantic(testElement.getSemantic(), testElement.getIndex()))
    {
        instancingSrc += 1;
        for (auto el : instanceDecl->getElements())
        {
            ro.vertexData->vertexDeclaration->addElement(instancingSrc, el.getOffset(), el.getType(), el.getSemantic(),
                                                         el.getIndex());
        }
    }
    ro.vertexData->vertexBufferBinding->setBinding(instancingSrc, rs->getGlobalInstanceVertexBuffer());

    ro.numberOfInstances *= rs->getGlobalNumberOfInstances();
}

static PolygonMode derivePolygonMode(const Pass* pass, const Renderable* rend, const Camera* cam)
{
    // Set up the solid / wireframe override
    // Precedence is Camera, Object, Material
    // Camera might not override object if not overrideable
    PolygonMode reqMode = pass->getPolygonMode();
    if (pass->getPolygonModeOverrideable() && rend->getPolygonModeOverrideable())
    {
        PolygonMode camPolyMode = cam->getPolygonMode();
        // check camera detial only when render detail is overridable
        if (reqMode > camPolyMode)
        {
            // only downgrade detail; if cam says wireframe we don't go up to solid
            reqMode = camPolyMode;
        }
    }
    return reqMode;
}

void SceneManager::renderInstancedObject(const RenderableList& rends, const Pass* pass, bool lightScissoringClipping,
                                         bool doLightIteration, const LightList* manualLightList)
{
    mAutoParamDataSource->setCurrentRenderable(rends.front());
    // override: this is passed through the instance buffer
    mAutoParamDataSource->setWorldMatrices(&Affine3::IDENTITY, 1);

    // Shader only path -> no normalise normals

    // We batch world matrices, so we skip setWorldTransform for each individual renderable

    // this copes with returning from negative scale in previous render op
    // for same pass
    if (mFlipCullingOnNegativeScale && mPassCullingMode != mDestRenderSystem->_getCullingMode())
        mDestRenderSystem->_setCullingMode(mPassCullingMode);

    mDestRenderSystem->_setPolygonMode(derivePolygonMode(pass, rends.front(), mCameraInProgress));

    // TODO: manually driving lights

    // collect lights of all renderables, thus cannot handle start-light without re-sorting
    std::set<Light*> batchLights;
    for (auto r : rends)
    {
        const LightList& rendLightList = r->getLights();
        batchLights.insert(rendLightList.begin(), rendLightList.end());
    }
    LightList lightListToUse;

    if(pass->getLightMask() == 0xFFFFFFFF)
    {
        lightListToUse = LightList(batchLights.begin(), batchLights.end());
    }
    else
    {
        for (auto l : batchLights)
            if (pass->getLightMask() & l->getLightMask())
                lightListToUse.push_back(l);
    }

    // TODO IterationDepthBias

    // TODO light scissoring & clipping

    useLights(&lightListToUse, pass->getMaxSimultaneousLights());
    fireRenderSingleObject(rends.front(), pass, mAutoParamDataSource.get(), &lightListToUse, false);

    updateGpuProgramParameters(pass);

    if(!rends.front()->preRender(this, mDestRenderSystem))
    {
        rends.front()->postRender(this, mDestRenderSystem);
        return;
    }

    using Matrix3x4f = TransformBase<3, float>;

    // update instance buffer
    if (!mInstanceBuffer || mInstanceBuffer->getNumVertices() < rends.size())
    {
        mInstanceBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(Matrix3x4f), rends.size(), HBU_CPU_TO_GPU);
        mInstanceBuffer->setIsInstanceData(true);
        mInstanceBuffer->setInstanceDataStepRate(1);
    }

    // fill instance data
    {
        HardwareBufferLockGuard lock(mInstanceBuffer, HardwareBuffer::HBL_DISCARD);
        auto instanceData = static_cast<Matrix3x4f*>(lock.pData);

        auto camPos = mAutoParamDataSource->getCurrentCamera()->getDerivedPosition();

        for (auto r : rends)
        {
            auto worldT = static_cast<SubEntity*>(r)->getParent()->_getParentNodeFullTransform();
            if(mCameraRelativeRendering)
                worldT.setTrans(worldT.getTrans() - camPos);
            *instanceData++ = Matrix3x4f(worldT[0]);
        }
    }

    RenderOperation ro;
    rends.front()->getRenderOperation(ro);
    ro.srcRenderable = rends.front();
    ro.numberOfInstances = rends.size();

    auto ielem = ro.vertexData->vertexDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, 1);

    auto instancingSrc = ro.vertexData->vertexDeclaration->getMaxSource();
    if(!ielem || instancingSrc == 0)
    {
        ro.vertexData->vertexDeclaration->removeElement(VES_TEXTURE_COORDINATES, 1);

        instancingSrc += 1;
        auto vec4sz = VertexElement::getTypeSize(VET_FLOAT4);
        ro.vertexData->vertexDeclaration->addElement(instancingSrc, 0 * vec4sz, VET_FLOAT4, VES_TEXTURE_COORDINATES, 1);
        ro.vertexData->vertexDeclaration->addElement(instancingSrc, 1 * vec4sz, VET_FLOAT4, VES_TEXTURE_COORDINATES, 2);
        ro.vertexData->vertexDeclaration->addElement(instancingSrc, 2 * vec4sz, VET_FLOAT4, VES_TEXTURE_COORDINATES, 3);
        // TODO ACT_CUSTOM
    }

    // set again -> might have been recreated
    ro.vertexData->vertexBufferBinding->setBinding(instancingSrc, mInstanceBuffer);

    injectGlobalInstancingDeclaration(ro, mDestRenderSystem);

    mDestRenderSystem->_render(ro);

    rends.front()->postRender(this, mDestRenderSystem);
}

void SceneManager::renderSingleObject(Renderable* rend, const Pass* pass,
                                      bool lightScissoringClipping, bool doLightIteration,
                                      const LightList* manualLightList)
{
    // Tell auto params object about the renderable change
    mAutoParamDataSource->setCurrentRenderable(rend);

    setWorldTransform(rend);

    // Sort out negative scaling
    // Assume first world matrix representative
    if (mFlipCullingOnNegativeScale)
    {
        CullingMode cullMode = mPassCullingMode;

        if (mAutoParamDataSource->getWorldMatrix().linear().hasNegativeScale())
        {
            switch(mPassCullingMode)
            {
            case CULL_CLOCKWISE:
                cullMode = CULL_ANTICLOCKWISE;
                break;
            case CULL_ANTICLOCKWISE:
                cullMode = CULL_CLOCKWISE;
                break;
            case CULL_NONE:
                break;
            };
        }

        // this also copes with returning from negative scale in previous render op
        // for same pass
        if (cullMode != mDestRenderSystem->_getCullingMode())
            mDestRenderSystem->_setCullingMode(cullMode);
    }

    mDestRenderSystem->_setPolygonMode(derivePolygonMode(pass, rend, mCameraInProgress));

    if (!doLightIteration)
    {
        // Even if manually driving lights, check light type passes
        if (!pass->getRunOnlyForOneLightType() ||
            (manualLightList && (manualLightList->size() != 1 ||
                                 manualLightList->front()->getType() == pass->getOnlyLightType())))
        {
            issueRenderWithLights(rend, pass, manualLightList, lightScissoringClipping);
        }

        // Reset view / projection changes if any
        resetViewProjMode();
        return;
    }

    // Here's where we issue the rendering operation to the render system
    // Note that we may do this once per light, therefore it's in a loop
    // and the light parameters are updated once per traversal through the
    // loop
    const LightList& rendLightList = rend->getLights();

    bool iteratePerLight = pass->getIteratePerLight();

    // deliberately unsigned in case start light exceeds number of lights
    // in which case this pass would be skipped
    int lightsLeft = 1;
    if (iteratePerLight)
    {
        // Don't allow total light count for all iterations to exceed max per pass
        lightsLeft = std::min<int>(rendLightList.size() - pass->getStartLight(),
                                   pass->getMaxSimultaneousLights());
    }

    const LightList* pLightListToUse;
    // Start counting from the start light
    size_t lightIndex = pass->getStartLight();
    size_t depthInc = 0;

    // Create local light list for faster light iteration setup
    static LightList localLightList;

    while (lightsLeft > 0)
    {
        // Determine light list to use
        if (iteratePerLight)
        {
            // Starting shadow texture index.
            size_t shadowTexIndex = mShadowRenderer.getShadowTexIndex(lightIndex);
            localLightList.resize(pass->getLightCountPerIteration());

            LightList::iterator destit = localLightList.begin();
            unsigned short numShadowTextureLights = 0;
            for (; destit != localLightList.end() && lightIndex < rendLightList.size();
                 ++lightIndex, --lightsLeft)
            {
                Light* currLight = rendLightList[lightIndex];

                // Check whether we need to filter this one out
                if ((pass->getRunOnlyForOneLightType() &&
                     pass->getOnlyLightType() != currLight->getType()) ||
                    (pass->getLightMask() & currLight->getLightMask()) == 0)
                {
                    // Skip
                    // Also skip shadow texture(s)
                    if (isShadowTechniqueTextureBased())
                    {
                        shadowTexIndex += mShadowRenderer.mShadowTextureCountPerType[currLight->getType()];
                    }
                    continue;
                }

                *destit++ = currLight;

                if (!isShadowTechniqueTextureBased())
                    continue;

                // potentially need to update content_type shadow texunit
                // corresponding to this light
                size_t textureCountPerLight = mShadowRenderer.mShadowTextureCountPerType[currLight->getType()];
                for (size_t j = 0; j < textureCountPerLight && shadowTexIndex < mShadowRenderer.mShadowTextures.size(); ++j)
                {
                    // link the numShadowTextureLights'th shadow texture unit
                    ushort tuindex = pass->_getTextureUnitWithContentTypeIndex(
                        TextureUnitState::CONTENT_SHADOW, numShadowTextureLights);
                    if (tuindex > pass->getNumTextureUnitStates()) break;

                    TextureUnitState* tu = pass->getTextureUnitState(tuindex);
                    const TexturePtr& shadowTex = mShadowRenderer.mShadowTextures[shadowTexIndex];
                    tu->_setTexturePtr(shadowTex);
                    Camera *cam = shadowTex->getBuffer()->getRenderTarget()->getViewport(0)->getCamera();
                    tu->setProjectiveTexturing(!pass->hasVertexProgram(), cam);
                    mAutoParamDataSource->setTextureProjector(cam, numShadowTextureLights);
                    ++numShadowTextureLights;
                    ++shadowTexIndex;
                    // Have to set TU on rendersystem right now, although
                    // autoparams will be set later
                    mDestRenderSystem->_setTextureUnitSettings(tuindex, *tu);
                }
            }
            // Did we run out of lights before slots? e.g. 5 lights, 2 per iteration
            if (destit != localLightList.end())
            {
                localLightList.erase(destit, localLightList.end());
                lightsLeft = 0;
            }
            pLightListToUse = &localLightList;

            // deal with the case where we found no lights
            // since this is light iteration, we shouldn't render at all
            if (pLightListToUse->empty())
                break;
        }
        else // !iterate per light
        {
            // Use complete light list potentially adjusted by start light
            if (pass->getStartLight() || pass->getMaxSimultaneousLights() != OGRE_MAX_SIMULTANEOUS_LIGHTS ||
                pass->getLightMask() != 0xFFFFFFFF)
            {
                // out of lights?
                // skip manual 2nd lighting passes onwards if we run out of lights, but never the first one
                if (pass->getStartLight() > 0 && pass->getStartLight() >= rendLightList.size())
                {
                    break;
                }

                localLightList.clear();
                LightList::const_iterator copyStart = rendLightList.begin();
                std::advance(copyStart, pass->getStartLight());
                // Clamp lights to copy to avoid overrunning the end of the list
                size_t lightsCopied = 0, lightsToCopy = std::min(
                    static_cast<size_t>(pass->getMaxSimultaneousLights()),
                    rendLightList.size() - pass->getStartLight());

                // Copy lights over
                for(LightList::const_iterator iter = copyStart; iter != rendLightList.end() && lightsCopied < lightsToCopy; ++iter)
                {
                    if((pass->getLightMask() & (*iter)->getLightMask()) != 0)
                    {
                        localLightList.push_back(*iter);
                        lightsCopied++;
                    }
                }

                pLightListToUse = &localLightList;
            }
            else
            {
                pLightListToUse = &rendLightList;
            }
            lightsLeft = 0;
        }

        // issue the render op

        // We might need to update the depth bias each iteration
        if (pass->getIterationDepthBias() != 0.0f)
        {
            float depthBiasBase =
                pass->getDepthBiasConstant() + pass->getIterationDepthBias() * depthInc;
            // depthInc deals with light iteration

            // Note that we have to set the depth bias here even if the depthInc
            // is zero (in which case you would think there is no change from
            // what was set in _setPass(). The reason is that if there are
            // multiple Renderables with this Pass, we won't go through _setPass
            // again at the start of the iteration for the next Renderable
            // because of Pass state grouping. So set it always

            // Set modified depth bias right away
            mDestRenderSystem->_setDepthBias(depthBiasBase, pass->getDepthBiasSlopeScale());

            // Set to increment internally too if rendersystem iterates
            mDestRenderSystem->setDeriveDepthBias(true,
                depthBiasBase, pass->getIterationDepthBias(),
                pass->getDepthBiasSlopeScale());
        }
        else
        {
            mDestRenderSystem->setDeriveDepthBias(false);
        }
        depthInc += pass->getPassIterationCount();

        issueRenderWithLights(rend, pass, pLightListToUse, lightScissoringClipping);
    } // possibly iterate per light
    
    // Reset view / projection changes if any
    resetViewProjMode();
}
//-----------------------------------------------------------------------
void SceneManager::setAmbientLight(const ColourValue& colour)
{
    mGpuParamsDirty |= GPV_GLOBAL;
    mAutoParamDataSource->setAmbientLightColour(colour);
}
//-----------------------------------------------------------------------
const ColourValue& SceneManager::getAmbientLight(void) const
{
    return mAutoParamDataSource->getAmbientLightColour();
}
//-----------------------------------------------------------------------
ViewPoint SceneManager::getSuggestedViewpoint(bool random)
{
    // By default return the origin
    ViewPoint vp;
    vp.position = Vector3::ZERO;
    vp.orientation = Quaternion::IDENTITY;
    return vp;
}
//-----------------------------------------------------------------------
void SceneManager::setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
{
    mFogMode = mode;
    mFogColour = colour;
    mFogStart = start;
    mFogEnd = end;
    mFogDensity = density;
}
//-----------------------------------------------------------------------
FogMode SceneManager::getFogMode(void) const
{
    return mFogMode;
}
//-----------------------------------------------------------------------
const ColourValue& SceneManager::getFogColour(void) const
{
    return mFogColour;
}
//-----------------------------------------------------------------------
Real SceneManager::getFogStart(void) const
{
    return mFogStart;
}
//-----------------------------------------------------------------------
Real SceneManager::getFogEnd(void) const
{
    return mFogEnd;
}
//-----------------------------------------------------------------------
Real SceneManager::getFogDensity(void) const
{
    return mFogDensity;
}
//-----------------------------------------------------------------------
BillboardSet* SceneManager::createBillboardSet(const String& name, unsigned int poolSize)
{
    NameValuePairList params;
    params["poolSize"] = StringConverter::toString(poolSize);
    return static_cast<BillboardSet*>(
        createMovableObject(name, BillboardSetFactory::FACTORY_TYPE_NAME, &params));
}
//-----------------------------------------------------------------------
BillboardSet* SceneManager::createBillboardSet(unsigned int poolSize)
{
    String name = mMovableNameGenerator.generate();
    return createBillboardSet(name, poolSize);
}
//-----------------------------------------------------------------------
BillboardSet* SceneManager::getBillboardSet(const String& name) const
{
    return static_cast<BillboardSet*>(
        getMovableObject(name, BillboardSetFactory::FACTORY_TYPE_NAME));
}
//-----------------------------------------------------------------------
bool SceneManager::hasBillboardSet(const String& name) const
{
    return hasMovableObject(name, BillboardSetFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::destroyBillboardSet(const String& name)
{
    destroyMovableObject(name, BillboardSetFactory::FACTORY_TYPE_NAME);
}
//-----------------------------------------------------------------------
void SceneManager::setDisplaySceneNodes(bool display)
{
    mDisplayNodes = display;
}
//-----------------------------------------------------------------------
Animation* SceneManager::createAnimation(const String& name, Real length)
{
    OGRE_LOCK_MUTEX(mAnimationsListMutex);

    // Check name not used
    if (mAnimationsList.find(name) != mAnimationsList.end())
    {
        OGRE_EXCEPT(
            Exception::ERR_DUPLICATE_ITEM,
            "An animation with the name " + name + " already exists",
            "SceneManager::createAnimation" );
    }

    Animation* pAnim = OGRE_NEW Animation(name, length);
    mAnimationsList[name] = pAnim;
    return pAnim;
}
//-----------------------------------------------------------------------
Animation* SceneManager::getAnimation(const String& name) const
{
    OGRE_LOCK_MUTEX(mAnimationsListMutex);

    AnimationList::const_iterator i = mAnimationsList.find(name);
    if (i == mAnimationsList.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
            "Cannot find animation with name " + name, 
            "SceneManager::getAnimation");
    }
    return i->second;
}
//-----------------------------------------------------------------------
bool SceneManager::hasAnimation(const String& name) const
{
    OGRE_LOCK_MUTEX(mAnimationsListMutex);
    return (mAnimationsList.find(name) != mAnimationsList.end());
}
//-----------------------------------------------------------------------
void SceneManager::destroyAnimation(const String& name)
{
    OGRE_LOCK_MUTEX(mAnimationsListMutex);

    // Also destroy any animation states referencing this animation
    mAnimationStates.removeAnimationState(name);

    AnimationList::iterator i = mAnimationsList.find(name);
    if (i == mAnimationsList.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
            "Cannot find animation with name " + name, 
            "SceneManager::getAnimation");
    }

    // Free memory
    OGRE_DELETE i->second;

    mAnimationsList.erase(i);

}
//-----------------------------------------------------------------------
void SceneManager::destroyAllAnimations(void)
{
    OGRE_LOCK_MUTEX(mAnimationsListMutex);
    // Destroy all states too, since they cannot reference destroyed animations
    destroyAllAnimationStates();

    for (auto& i : mAnimationsList)
    {
        // destroy
        OGRE_DELETE i.second;
    }
    mAnimationsList.clear();
}
//-----------------------------------------------------------------------
AnimationState* SceneManager::createAnimationState(const String& animName)
{
    // Get animation, this will throw an exception if not found
    Animation* anim = getAnimation(animName);

    // Create new state
    return mAnimationStates.createAnimationState(animName, 0, anim->getLength());

}
//-----------------------------------------------------------------------
AnimationState* SceneManager::getAnimationState(const String& animName) const
{
    return mAnimationStates.getAnimationState(animName);

}
//-----------------------------------------------------------------------
bool SceneManager::hasAnimationState(const String& name) const
{
    return mAnimationStates.hasAnimationState(name);
}
//-----------------------------------------------------------------------
void SceneManager::destroyAnimationState(const String& name)
{
    mAnimationStates.removeAnimationState(name);
}
//-----------------------------------------------------------------------
void SceneManager::destroyAllAnimationStates(void)
{
    mAnimationStates.removeAllAnimationStates();
}
//-----------------------------------------------------------------------
void SceneManager::_applySceneAnimations(void)
{
    // manual lock over states (extended duration required)
    OGRE_LOCK_MUTEX(mAnimationStates.OGRE_AUTO_MUTEX_NAME);

    // Iterate twice, once to reset, once to apply, to allow blending
    for(auto *state : mAnimationStates.getEnabledAnimationStates())
    {
        Animation* anim = getAnimation(state->getAnimationName());

        // Reset any nodes involved
        for (const auto& it : anim->_getNodeTrackList())
        {
            if (Node* nd = it.second->getAssociatedNode())
                nd->resetToInitialState();
        }

        for (const auto& it : anim->_getNumericTrackList())
        {
            if (const auto& animPtr = it.second->getAssociatedAnimable())
                animPtr->resetToBaseValue();
        }
    }

    // this should allow blended animations
    for(auto *state : mAnimationStates.getEnabledAnimationStates())
    {
        Animation* anim = getAnimation(state->getAnimationName());
        // Apply the animation
        anim->apply(state->getTimePosition(), state->getWeight());
    }
}
//---------------------------------------------------------------------
void SceneManager::manualRender(RenderOperation* rend, 
                                Pass* pass, Viewport* vp, const Affine3& worldMatrix,
                                const Affine3& viewMatrix, const Matrix4& projMatrix,
                                bool doBeginEndFrame) 
{
    if (vp)
        setViewport(vp);

    if (doBeginEndFrame)
        mDestRenderSystem->_beginFrame();

    auto usedPass = _setPass(pass);
    mAutoParamDataSource->setCurrentRenderable(0);
    if (vp)
    {
        mAutoParamDataSource->setCurrentRenderTarget(vp->getTarget());
    }
    mAutoParamDataSource->setCurrentSceneManager(this);
    mAutoParamDataSource->setWorldMatrices(&worldMatrix, 1);
    Camera dummyCam(BLANKSTRING, 0);
    dummyCam.setCustomViewMatrix(true, viewMatrix);
    dummyCam.setCustomProjectionMatrix(true, projMatrix);
    mAutoParamDataSource->setCurrentCamera(&dummyCam, false);
    updateGpuProgramParameters(usedPass);
    mDestRenderSystem->_render(*rend);

    if (doBeginEndFrame)
        mDestRenderSystem->_endFrame();

}
//---------------------------------------------------------------------
void SceneManager::manualRender(Renderable* rend, const Pass* pass, Viewport* vp,
    const Affine3& viewMatrix,
    const Matrix4& projMatrix,bool doBeginEndFrame,
    bool lightScissoringClipping, bool doLightIteration, const LightList* manualLightList)
{
    if (vp)
        setViewport(vp);

    if (doBeginEndFrame)
        mDestRenderSystem->_beginFrame();

    auto usedPass = _setPass(pass);
    Camera dummyCam(BLANKSTRING, 0);
    dummyCam.setCustomViewMatrix(true, viewMatrix);
    dummyCam.setCustomProjectionMatrix(true, projMatrix);

    if (vp)
    {
        mAutoParamDataSource->setCurrentRenderTarget(vp->getTarget());
    }

    const Camera* oldCam = mAutoParamDataSource->getCurrentCamera();

    mAutoParamDataSource->setCurrentSceneManager(this);
    mAutoParamDataSource->setCurrentCamera(&dummyCam, false);

    renderSingleObject(rend, usedPass, lightScissoringClipping, doLightIteration, manualLightList);

    mAutoParamDataSource->setCurrentCamera(oldCam, false);

    if (doBeginEndFrame)
        mDestRenderSystem->_endFrame();

}
//---------------------------------------------------------------------
void SceneManager::resetViewProjMode()
{
    if (mResetIdentityView)
    {
        // Coming back to normal from identity view
        mGpuParamsDirty |= (uint16)GPV_GLOBAL;

        mResetIdentityView = false;
    }
    
    if (mResetIdentityProj)
    {
        // Coming back from flat projection
        mGpuParamsDirty |= (uint16)GPV_GLOBAL;

        mResetIdentityProj = false;
    }
    

}
//---------------------------------------------------------------------
void SceneManager::addRenderQueueListener(RenderQueueListener* newListener)
{
    mRenderQueueListeners.push_back(newListener);
}
//---------------------------------------------------------------------
void SceneManager::removeRenderQueueListener(RenderQueueListener* delListener)
{
    RenderQueueListenerList::iterator i, iend;
    iend = mRenderQueueListeners.end();
    for (i = mRenderQueueListeners.begin(); i != iend; ++i)
    {
        if (*i == delListener)
        {
            mRenderQueueListeners.erase(i);
            break;
        }
    }

}
//---------------------------------------------------------------------
void SceneManager::addRenderObjectListener(RenderObjectListener* newListener)
{
    mRenderObjectListeners.push_back(newListener);
}
//---------------------------------------------------------------------
void SceneManager::removeRenderObjectListener(RenderObjectListener* delListener)
{
    RenderObjectListenerList::iterator i, iend;
    iend = mRenderObjectListeners.end();
    for (i = mRenderObjectListeners.begin(); i != iend; ++i)
    {
        if (*i == delListener)
        {
            mRenderObjectListeners.erase(i);
            break;
        }
    }
}
void SceneManager::addListener(Listener* newListener)
{
    if (std::find(mListeners.begin(), mListeners.end(), newListener) == mListeners.end())
        mListeners.push_back(newListener);
}
//---------------------------------------------------------------------
void SceneManager::removeListener(Listener* delListener)
{
    ListenerList::iterator i = std::find(mListeners.begin(), mListeners.end(), delListener);
    if (i != mListeners.end())
        mListeners.erase(i);
}
void SceneManager::addShadowTextureListener(ShadowTextureListener* newListener)
{
    if (std::find(mShadowRenderer.mListeners.begin(), mShadowRenderer.mListeners.end(), newListener) ==
        mShadowRenderer.mListeners.end())
        mShadowRenderer.mListeners.push_back(newListener);
}
//---------------------------------------------------------------------
void SceneManager::removeShadowTextureListener(ShadowTextureListener* delListener)
{
    auto i = std::find(mShadowRenderer.mListeners.begin(), mShadowRenderer.mListeners.end(), delListener);
    if (i != mShadowRenderer.mListeners.end())
        mShadowRenderer.mListeners.erase(i);
}
//---------------------------------------------------------------------
void SceneManager::firePreRenderQueues()
{
    for (auto *l : mRenderQueueListeners)
    {
        l->preRenderQueues();
    }
}
//---------------------------------------------------------------------
void SceneManager::firePostRenderQueues()
{
    for (auto *l : mRenderQueueListeners)
    {
        l->postRenderQueues();
    }
}
//---------------------------------------------------------------------
bool SceneManager::fireRenderQueueStarted(uint8 id, const String& cameraName)
{
    bool skip = false;
    for (auto *l : mRenderQueueListeners)
    {
        l->renderQueueStarted(id, cameraName, skip);
    }
    return skip;
}
//---------------------------------------------------------------------
bool SceneManager::fireRenderQueueEnded(uint8 id, const String& cameraName)
{
    bool repeat = false;
    for (auto *l : mRenderQueueListeners)
    {
        l->renderQueueEnded(id, cameraName, repeat);
    }
    return repeat;
}
//---------------------------------------------------------------------
void SceneManager::fireRenderSingleObject(Renderable* rend, const Pass* pass,
                                           const AutoParamDataSource* source, 
                                           const LightList* pLightList, bool suppressRenderStateChanges)
{
    for (auto *l : mRenderObjectListeners)
    {
        l->notifyRenderSingleObject(rend, pass, source, pLightList, suppressRenderStateChanges);
    }
}
//---------------------------------------------------------------------
void SceneManager::firePreUpdateSceneGraph(Camera* camera)
{
    ListenerList listenersCopy = mListeners;
    for (auto *l : listenersCopy)
    {
        l->preUpdateSceneGraph(this, camera);
    }
}
//---------------------------------------------------------------------
void SceneManager::firePostUpdateSceneGraph(Camera* camera)
{
    ListenerList listenersCopy = mListeners;
    for (auto *l : listenersCopy)
    {
        l->postUpdateSceneGraph(this, camera);
    }
}

//---------------------------------------------------------------------
void SceneManager::firePreFindVisibleObjects(Viewport* v)
{
    ListenerList listenersCopy = mListeners;
    for (auto *l : listenersCopy)
    {
        l->preFindVisibleObjects(this, mIlluminationStage, v);
    }

}
//---------------------------------------------------------------------
void SceneManager::firePostFindVisibleObjects(Viewport* v)
{
    ListenerList listenersCopy = mListeners;
    for (auto *l : listenersCopy)
    {
        l->postFindVisibleObjects(this, mIlluminationStage, v);
    }
}
//---------------------------------------------------------------------
void SceneManager::fireSceneManagerDestroyed()
{
    ListenerList listenersCopy = mListeners;
    for (auto *l : listenersCopy)
    {
        l->sceneManagerDestroyed(this);
    }
}
//---------------------------------------------------------------------
void SceneManager::setViewport(Viewport* vp)
{
    mCurrentViewport = vp;
    // Tell params about viewport
    mAutoParamDataSource->setCurrentViewport(vp);
    // Set viewport in render system
    mDestRenderSystem->_setViewport(vp);
    // Set the active material scheme for this viewport
    MaterialManager::getSingleton().setActiveScheme(vp->getMaterialScheme());
}
//---------------------------------------------------------------------
void SceneManager::showBoundingBoxes(bool bShow) 
{
    mShowBoundingBoxes = bShow;
}
//---------------------------------------------------------------------
bool SceneManager::getShowBoundingBoxes() const
{
    return mShowBoundingBoxes;
}
//---------------------------------------------------------------------
void SceneManager::_notifyAutotrackingSceneNode(SceneNode* node, bool autoTrack)
{
    if (autoTrack)
    {
        mAutoTrackingSceneNodes.insert(node);
    }
    else
    {
        mAutoTrackingSceneNodes.erase(node);
    }
}
void SceneManager::setShadowTechnique(ShadowTechnique technique)
{
    mShadowRenderer.setShadowTechnique(technique);
}
//-----------------------------------------------------------------------
void SceneManager::_notifyLightsDirty(void)
{
    ++mLightsDirtyCounter;
}
//---------------------------------------------------------------------
void SceneManager::findLightsAffectingFrustum(const Camera* camera)
{
    // Basic iteration for this SM

    MovableObjectCollection* lights =
        getMovableObjectCollection(LightFactory::FACTORY_TYPE_NAME);


    {
            OGRE_LOCK_MUTEX(lights->mutex);

        // Pre-allocate memory
        mTestLightInfos.clear();
        mTestLightInfos.reserve(lights->map.size());

        MovableObjectIterator it(lights->map.begin(), lights->map.end());

        while(it.hasMoreElements())
        {
            Light* l = static_cast<Light*>(it.getNext());

            if (mCameraRelativeRendering)
                l->_setCameraRelative(mCameraInProgress);
            else
                l->_setCameraRelative(0);

            if (l->isVisible())
            {
                LightInfo lightInfo;
                lightInfo.light = l;
                lightInfo.type = l->getType();
                lightInfo.lightMask = l->getLightMask();
                if (lightInfo.type == Light::LT_DIRECTIONAL)
                {
                    // Always visible
                    lightInfo.position = Vector3::ZERO;
                    lightInfo.range = 0;
                    mTestLightInfos.push_back(lightInfo);
                }
                else
                {
                    // NB treating spotlight as point for simplicity
                    // Just see if the lights attenuation range is within the frustum
                    lightInfo.range = l->getAttenuationRange();
                    lightInfo.position = l->getDerivedPosition();
                    Sphere sphere(lightInfo.position, lightInfo.range);
                    if (camera->isVisible(sphere))
                    {
                        mTestLightInfos.push_back(lightInfo);
                    }
                }
            }
        }
    } // release lock on lights collection

    // Update lights affecting frustum if changed
    if (mCachedLightInfos != mTestLightInfos)
    {
        mLightsAffectingFrustum.resize(mTestLightInfos.size());
        LightInfoList::const_iterator i;
        LightList::iterator j = mLightsAffectingFrustum.begin();
        for (i = mTestLightInfos.begin(); i != mTestLightInfos.end(); ++i, ++j)
        {
            *j = i->light;
            // add cam distance for sorting if texture shadows
            if (isShadowTechniqueTextureBased())
            {
                (*j)->_calcTempSquareDist(camera->getDerivedPosition());
            }
        }

        mShadowRenderer.sortLightsAffectingFrustum(mLightsAffectingFrustum);
        // Use swap instead of copy operator for efficiently
        mCachedLightInfos.swap(mTestLightInfos);

        // notify light dirty, so all movable objects will re-populate
        // their light list next time
        _notifyLightsDirty();
    }

}
void SceneManager::initShadowVolumeMaterials()
{
    mShadowRenderer.initShadowVolumeMaterials();
}
//---------------------------------------------------------------------
const RealRect& SceneManager::getLightScissorRect(Light* l, const Camera* cam)
{
    checkCachedLightClippingInfo();

    // Re-use calculations if possible
    LightClippingInfoMap::iterator ci = mLightClippingInfoMap.emplace(l, LightClippingInfo()).first;
    if (!ci->second.scissorValid)
    {

        buildScissor(l, cam, ci->second.scissorRect);
        ci->second.scissorValid = true;
    }

    return ci->second.scissorRect;

}
//---------------------------------------------------------------------
ClipResult SceneManager::buildAndSetScissor(const LightList& ll, const Camera* cam)
{
    RealRect finalRect;
    // init (inverted since we want to grow from nothing)
    finalRect.left = finalRect.bottom = 1.0f;
    finalRect.right = finalRect.top = -1.0f;

    for (auto *l : ll)
    {
        // a directional light is being used, no scissoring can be done, period.
        if (l->getType() == Light::LT_DIRECTIONAL)
            return CLIPPED_NONE;

        const RealRect& scissorRect = getLightScissorRect(l, cam);

        // merge with final
        finalRect.left = std::min(finalRect.left, scissorRect.left);
        finalRect.bottom = std::min(finalRect.bottom, scissorRect.bottom);
        finalRect.right= std::max(finalRect.right, scissorRect.right);
        finalRect.top = std::max(finalRect.top, scissorRect.top);


    }

    if (finalRect.left >= 1.0f || finalRect.right <= -1.0f ||
        finalRect.top <= -1.0f || finalRect.bottom >= 1.0f)
    {
        // rect was offscreen
        return CLIPPED_ALL;
    }

    // Some scissoring?
    if (finalRect.left > -1.0f || finalRect.right < 1.0f || 
        finalRect.bottom > -1.0f || finalRect.top < 1.0f)
    {
        // Turn normalised device coordinates into pixels
        Rect vp = mCurrentViewport->getActualDimensions();

        Rect scissor(vp.left + ((finalRect.left + 1) * 0.5 * vp.width()),
                     vp.top + ((-finalRect.top + 1) * 0.5 * vp.height()),
                     vp.left + ((finalRect.right + 1) * 0.5 * vp.width()),
                     vp.top + ((-finalRect.bottom + 1) * 0.5 * vp.height()));
        mDestRenderSystem->setScissorTest(true, scissor);

        return CLIPPED_SOME;
    }
    else
        return CLIPPED_NONE;

}
//---------------------------------------------------------------------
void SceneManager::buildScissor(const Light* light, const Camera* cam, RealRect& rect)
{
    // Project the sphere onto the camera
    Sphere sphere(light->getDerivedPosition(), light->getAttenuationRange());
    cam->Frustum::projectSphere(sphere, &(rect.left), &(rect.top), &(rect.right), &(rect.bottom));
}
//---------------------------------------------------------------------
void SceneManager::resetScissor()
{
    mDestRenderSystem->setScissorTest(false);
}
//---------------------------------------------------------------------
void SceneManager::invalidatePerFrameScissorRectCache()
{
	checkCachedLightClippingInfo(true);
}
//---------------------------------------------------------------------
void SceneManager::checkCachedLightClippingInfo(bool forceScissorRectsInvalidation)
{
    unsigned long frame = Root::getSingleton().getNextFrameNumber();
    if (frame != mLightClippingInfoMapFrameNumber)
    {
        // reset cached clip information
        mLightClippingInfoMap.clear();
        mLightClippingInfoMapFrameNumber = frame;
    }
    else if(forceScissorRectsInvalidation)
    {
        for(auto& c : mLightClippingInfoMap)
            c.second.scissorValid = false;
    }
}
//---------------------------------------------------------------------
const PlaneList& SceneManager::getLightClippingPlanes(Light* l)
{
    checkCachedLightClippingInfo();

    // Try to re-use clipping info if already calculated
    LightClippingInfoMap::iterator ci = mLightClippingInfoMap.emplace(l, LightClippingInfo()).first;

    if (!ci->second.clipPlanesValid)
    {
        buildLightClip(l, ci->second.clipPlanes);
        ci->second.clipPlanesValid = true;
    }
    return ci->second.clipPlanes;
    
}
//---------------------------------------------------------------------
ClipResult SceneManager::buildAndSetLightClip(const LightList& ll)
{
    if (!mDestRenderSystem->getCapabilities()->hasCapability(RSC_USER_CLIP_PLANES))
        return CLIPPED_NONE;

    Light* clipBase = 0;
    for (auto *l : ll)
    {
        // a directional light is being used, no clipping can be done, period.
        if (l->getType() == Light::LT_DIRECTIONAL)
            return CLIPPED_NONE;

        if (clipBase)
        {
            // we already have a clip base, so we had more than one light
            // in this list we could clip by, so clip none
            return CLIPPED_NONE;
        }
        clipBase = l;
    }

    if (clipBase)
    {
        const PlaneList& clipPlanes = getLightClippingPlanes(clipBase);
        
        mDestRenderSystem->setClipPlanes(clipPlanes);
        return CLIPPED_SOME;
    }
    else
    {
        // Can only get here if no non-directional lights from which to clip from
        // ie list must be empty
        return CLIPPED_ALL;
    }


}
//---------------------------------------------------------------------
void SceneManager::buildLightClip(const Light* l, PlaneList& planes)
{
    if (!mDestRenderSystem->getCapabilities()->hasCapability(RSC_USER_CLIP_PLANES))
        return;

    planes.clear();

    Vector3 pos = l->getDerivedPosition();
    Real r = l->getAttenuationRange();
    switch(l->getType())
    {
    case Light::LT_POINT:
        {
            planes.push_back(Plane(Vector3::UNIT_X, pos + Vector3(-r, 0, 0)));
            planes.push_back(Plane(Vector3::NEGATIVE_UNIT_X, pos + Vector3(r, 0, 0)));
            planes.push_back(Plane(Vector3::UNIT_Y, pos + Vector3(0, -r, 0)));
            planes.push_back(Plane(Vector3::NEGATIVE_UNIT_Y, pos + Vector3(0, r, 0)));
            planes.push_back(Plane(Vector3::UNIT_Z, pos + Vector3(0, 0, -r)));
            planes.push_back(Plane(Vector3::NEGATIVE_UNIT_Z, pos + Vector3(0, 0, r)));
        }
        break;
    case Light::LT_SPOTLIGHT:
        {
            Vector3 dir = l->getDerivedDirection();
            // near & far planes
            planes.push_back(Plane(dir, pos + dir * l->getSpotlightNearClipDistance()));
            planes.push_back(Plane(-dir, pos + dir * r));
            // 4 sides of pyramids
            // derive orientation
            Vector3 up = Vector3::UNIT_Y;
            // Check it's not coincident with dir
            if (Math::Abs(up.dotProduct(dir)) >= 1.0f)
            {
                up = Vector3::UNIT_Z;
            }
            // Derive rotation from axes (negate dir since -Z)
            Matrix3 q = Math::lookRotation(-dir, up);

            // derive pyramid corner vectors in world orientation
            Vector3 tl, tr, bl, br;
            Real d = Math::Tan(l->getSpotlightOuterAngle() * 0.5) * r;
            tl = q * Vector3(-d, d, -r);
            tr = q * Vector3(d, d, -r);
            bl = q * Vector3(-d, -d, -r);
            br = q * Vector3(d, -d, -r);

            // use cross product to derive normals, pass through light world pos
            // top
            planes.push_back(Plane(tl.crossProduct(tr).normalisedCopy(), pos));
            // right
            planes.push_back(Plane(tr.crossProduct(br).normalisedCopy(), pos));
            // bottom
            planes.push_back(Plane(br.crossProduct(bl).normalisedCopy(), pos));
            // left
            planes.push_back(Plane(bl.crossProduct(tl).normalisedCopy(), pos));

        }
        break;
    default:
        // do nothing
        break;
    };

}
//---------------------------------------------------------------------
void SceneManager::resetLightClip()
{
    if (!mDestRenderSystem->getCapabilities()->hasCapability(RSC_USER_CLIP_PLANES))
        return;

    mDestRenderSystem->setClipPlanes(PlaneList());
}
//---------------------------------------------------------------------
const ColourValue& SceneManager::getShadowColour(void) const
{
    return mShadowRenderer.mShadowColour;
}
//---------------------------------------------------------------------
void SceneManager::setShadowFarDistance(Real distance)
{
    mShadowRenderer.mDefaultShadowFarDist = distance;
    mShadowRenderer.mDefaultShadowFarDistSquared = distance * distance;
}
//---------------------------------------------------------------------
void SceneManager::setShadowDirectionalLightExtrusionDistance(Real dist)
{
    mShadowRenderer.mShadowDirLightExtrudeDist = dist;
}
//---------------------------------------------------------------------
Real SceneManager::getShadowDirectionalLightExtrusionDistance(void) const
{
    return mShadowRenderer.mShadowDirLightExtrudeDist;
}
void SceneManager::setShadowIndexBufferSize(size_t size)
{
    mShadowRenderer.setShadowIndexBufferSize(size);
}
//---------------------------------------------------------------------
ConstShadowTextureConfigIterator SceneManager::getShadowTextureConfigIterator() const
{
    return ConstShadowTextureConfigIterator(
        mShadowRenderer.mShadowTextureConfigList.begin(), mShadowRenderer.mShadowTextureConfigList.end());

}
//---------------------------------------------------------------------
void SceneManager::setShadowTextureSelfShadow(bool selfShadow) 
{ 
    mShadowRenderer.mShadowTextureSelfShadow = selfShadow;
    if (isShadowTechniqueTextureBased())
        getRenderQueue()->setShadowCastersCannotBeReceivers(!selfShadow);
}
//---------------------------------------------------------------------
void SceneManager::setShadowCameraSetup(const ShadowCameraSetupPtr& shadowSetup)
{
    mShadowRenderer.mDefaultShadowCameraSetup = shadowSetup;

}
//---------------------------------------------------------------------
const ShadowCameraSetupPtr& SceneManager::getShadowCameraSetup() const
{
    return mShadowRenderer.mDefaultShadowCameraSetup;
}
void SceneManager::ensureShadowTexturesCreated()
{
    mShadowRenderer.ensureShadowTexturesCreated();
}
void SceneManager::destroyShadowTextures(void)
{
    mShadowRenderer.destroyShadowTextures();
}
void SceneManager::prepareShadowTextures(Camera* cam, Viewport* vp, const LightList* lightList)
{
        // Set the illumination stage, prevents recursive calls
    IlluminationRenderStage savedStage = mIlluminationStage;
    mIlluminationStage = IRS_RENDER_TO_TEXTURE;

    if (lightList == 0)
        lightList = &mLightsAffectingFrustum;

    try
    {
        mShadowRenderer.prepareShadowTextures(cam, vp, lightList);
    }
    catch (Exception&)
    {
        // we must reset the illumination stage if an exception occurs
        mIlluminationStage = savedStage;
        throw;
    }

    mIlluminationStage = savedStage;
}
//---------------------------------------------------------------------
SceneManager::RenderContext* SceneManager::_pauseRendering()
{
    RenderContext* context = new RenderContext;
    context->renderQueue = mRenderQueue.release();
    context->viewport = mCurrentViewport;
    context->camera = mCameraInProgress;
    context->activeChain = _getActiveCompositorChain();

    mDestRenderSystem->_endFrame();
    mRenderQueue = 0;
    return context;
}
//---------------------------------------------------------------------
void SceneManager::_resumeRendering(SceneManager::RenderContext* context) 
{
    mRenderQueue.reset(context->renderQueue);
    _setActiveCompositorChain(context->activeChain);
    Ogre::Viewport* vp = context->viewport;
    Ogre::Camera* camera = context->camera;

    // Set the viewport - this is deliberately after the shadow texture update
    setViewport(vp);

    // Tell params about camera
    mAutoParamDataSource->setCurrentCamera(camera, mCameraRelativeRendering);
    // Set autoparams for finite dir light extrusion
    mAutoParamDataSource->setShadowDirLightExtrusionDistance(mShadowRenderer.mShadowDirLightExtrudeDist);

    // Tell params about render target
    mAutoParamDataSource->setCurrentRenderTarget(vp->getTarget());


    // Set camera window clipping planes (if any)
    if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_USER_CLIP_PLANES))
    {
        mDestRenderSystem->setClipPlanes(camera->isWindowSet() ? camera->getWindowPlanes() : PlaneList());
    }
    mCameraInProgress = context->camera;
    mDestRenderSystem->_beginFrame();
    
    mDestRenderSystem->_setTextureProjectionRelativeTo(mCameraRelativeRendering, mCameraInProgress->getDerivedPosition());
    delete context;
}
//---------------------------------------------------------------------
StaticGeometry* SceneManager::createStaticGeometry(const String& name)
{
    // Check not existing
    if (mStaticGeometryList.find(name) != mStaticGeometryList.end())
    {
        OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
            "StaticGeometry with name '" + name + "' already exists!", 
            "SceneManager::createStaticGeometry");
    }
    StaticGeometry* ret = OGRE_NEW StaticGeometry(this, name);
    mStaticGeometryList[name] = ret;
    return ret;
}
//---------------------------------------------------------------------
StaticGeometry* SceneManager::getStaticGeometry(const String& name) const
{
    StaticGeometryList::const_iterator i = mStaticGeometryList.find(name);
    if (i == mStaticGeometryList.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
            "StaticGeometry with name '" + name + "' not found", 
            "SceneManager::createStaticGeometry");
    }
    return i->second;
}
//-----------------------------------------------------------------------
bool SceneManager::hasStaticGeometry(const String& name) const
{
    return (mStaticGeometryList.find(name) != mStaticGeometryList.end());
}

//---------------------------------------------------------------------
void SceneManager::destroyStaticGeometry(StaticGeometry* geom)
{
    destroyStaticGeometry(geom->getName());
}
//---------------------------------------------------------------------
void SceneManager::destroyStaticGeometry(const String& name)
{
    StaticGeometryList::iterator i = mStaticGeometryList.find(name);
    if (i != mStaticGeometryList.end())
    {
        OGRE_DELETE i->second;
        mStaticGeometryList.erase(i);
    }

}
//---------------------------------------------------------------------
void SceneManager::destroyAllStaticGeometry(void)
{
    for (auto& s : mStaticGeometryList)
    {
        OGRE_DELETE s.second;
    }
    mStaticGeometryList.clear();
}
//---------------------------------------------------------------------
InstanceManager* SceneManager::createInstanceManager( const String &customName, const String &meshName,
                                                      const String &groupName,
                                                      InstanceManager::InstancingTechnique technique,
                                                      size_t numInstancesPerBatch, uint16 flags,
                                                      unsigned short subMeshIdx )
{
    if (mInstanceManagerMap.find(customName) != mInstanceManagerMap.end())
    {
        OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM, 
            "InstancedManager with name '" + customName + "' already exists!", 
            "SceneManager::createInstanceManager");
    }

    InstanceManager *retVal = new InstanceManager( customName, this, meshName, groupName, technique,
                                                    flags, numInstancesPerBatch, subMeshIdx );

    mInstanceManagerMap[customName] = retVal;
    return retVal;
}
//---------------------------------------------------------------------
InstanceManager* SceneManager::getInstanceManager( const String &managerName ) const
{
    InstanceManagerMap::const_iterator itor = mInstanceManagerMap.find(managerName);

    if (itor == mInstanceManagerMap.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "InstancedManager with name '" + managerName + "' not found", 
                "SceneManager::getInstanceManager");
    }

    return itor->second;
}
//---------------------------------------------------------------------
bool SceneManager::hasInstanceManager( const String &managerName ) const
{
    InstanceManagerMap::const_iterator itor = mInstanceManagerMap.find(managerName);
    return itor != mInstanceManagerMap.end();
}
//---------------------------------------------------------------------
void SceneManager::destroyInstanceManager( const String &name )
{
    //The manager we're trying to destroy might have been scheduled for updating
    //while we haven't yet rendered a frame. Update now to avoid a dangling ptr
    updateDirtyInstanceManagers();

    InstanceManagerMap::iterator i = mInstanceManagerMap.find(name);
    if (i != mInstanceManagerMap.end())
    {
        OGRE_DELETE i->second;
        mInstanceManagerMap.erase(i);
    }
}
//---------------------------------------------------------------------
void SceneManager::destroyInstanceManager( InstanceManager *instanceManager )
{
    destroyInstanceManager( instanceManager->getName() );
}
//---------------------------------------------------------------------
void SceneManager::destroyAllInstanceManagers(void)
{
    for (auto& m : mInstanceManagerMap)
    {
        OGRE_DELETE m.second;
    }

    mInstanceManagerMap.clear();
    mDirtyInstanceManagers.clear();
}
//---------------------------------------------------------------------
size_t SceneManager::getNumInstancesPerBatch( const String &meshName, const String &groupName,
                                              const String &materialName,
                                              InstanceManager::InstancingTechnique technique,
                                              size_t numInstancesPerBatch, uint16 flags,
                                              unsigned short subMeshIdx )
{
    InstanceManager tmpMgr( "TmpInstanceManager", this, meshName, groupName,
                            technique, flags, numInstancesPerBatch, subMeshIdx );
    
    return tmpMgr.getMaxOrBestNumInstancesPerBatch( materialName, numInstancesPerBatch, flags );
}
//---------------------------------------------------------------------
InstancedEntity* SceneManager::createInstancedEntity( const String &materialName, const String &managerName )
{
    InstanceManagerMap::const_iterator itor = mInstanceManagerMap.find(managerName);

    if (itor == mInstanceManagerMap.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "InstancedManager with name '" + managerName + "' not found", 
                "SceneManager::createInstanceEntity");
    }

    return itor->second->createInstancedEntity( materialName );
}
//---------------------------------------------------------------------
void SceneManager::destroyInstancedEntity( InstancedEntity *instancedEntity )
{
    instancedEntity->_getOwner()->removeInstancedEntity( instancedEntity );
}
//---------------------------------------------------------------------
void SceneManager::_addDirtyInstanceManager( InstanceManager *dirtyManager )
{
    mDirtyInstanceManagers.push_back( dirtyManager );
}
//---------------------------------------------------------------------
void SceneManager::updateDirtyInstanceManagers(void)
{
    //Copy all dirty mgrs to a temporary buffer to iterate through them. We need this because
    //if two InstancedEntities from different managers belong to the same SceneNode, one of the
    //managers may have been tagged as dirty while the other wasn't, and _addDirtyInstanceManager
    //will get called while iterating through them. The "while" loop will update all mgrs until
    //no one is dirty anymore (i.e. A makes B aware it's dirty, B makes C aware it's dirty)
    //mDirtyInstanceMgrsTmp isn't a local variable to prevent allocs & deallocs every frame.
    mDirtyInstanceMgrsTmp.insert( mDirtyInstanceMgrsTmp.end(), mDirtyInstanceManagers.begin(),
                                    mDirtyInstanceManagers.end() );
    mDirtyInstanceManagers.clear();

    while( !mDirtyInstanceMgrsTmp.empty() )
    {
        for(auto *d : mDirtyInstanceMgrsTmp)
        {
            d->_updateDirtyBatches();
        }

        //Clear temp buffer
        mDirtyInstanceMgrsTmp.clear();

        //Do it again?
        mDirtyInstanceMgrsTmp.insert( mDirtyInstanceMgrsTmp.end(), mDirtyInstanceManagers.begin(),
                                    mDirtyInstanceManagers.end() );
        mDirtyInstanceManagers.clear();
    }
}
//---------------------------------------------------------------------
AxisAlignedBoxSceneQuery* 
SceneManager::createAABBQuery(const AxisAlignedBox& box, uint32 mask)
{
    DefaultAxisAlignedBoxSceneQuery* q = OGRE_NEW DefaultAxisAlignedBoxSceneQuery(this);
    q->setBox(box);
    q->setQueryMask(mask);
    return q;
}
//---------------------------------------------------------------------
SphereSceneQuery* 
SceneManager::createSphereQuery(const Sphere& sphere, uint32 mask)
{
    DefaultSphereSceneQuery* q = OGRE_NEW DefaultSphereSceneQuery(this);
    q->setSphere(sphere);
    q->setQueryMask(mask);
    return q;
}
//---------------------------------------------------------------------
PlaneBoundedVolumeListSceneQuery* 
SceneManager::createPlaneBoundedVolumeQuery(const PlaneBoundedVolumeList& volumes, 
                                            uint32 mask)
{
    DefaultPlaneBoundedVolumeListSceneQuery* q = OGRE_NEW DefaultPlaneBoundedVolumeListSceneQuery(this);
    q->setVolumes(volumes);
    q->setQueryMask(mask);
    return q;
}

//---------------------------------------------------------------------
RaySceneQuery* 
SceneManager::createRayQuery(const Ray& ray, uint32 mask)
{
    DefaultRaySceneQuery* q = OGRE_NEW DefaultRaySceneQuery(this);
    q->setRay(ray);
    q->setQueryMask(mask);
    return q;
}
//---------------------------------------------------------------------
IntersectionSceneQuery* 
SceneManager::createIntersectionQuery(uint32 mask)
{

    DefaultIntersectionSceneQuery* q = OGRE_NEW DefaultIntersectionSceneQuery(this);
    q->setQueryMask(mask);
    return q;
}
//---------------------------------------------------------------------
void SceneManager::destroyQuery(SceneQuery* query)
{
    OGRE_DELETE query;
}
//---------------------------------------------------------------------
SceneManager::MovableObjectCollection* 
SceneManager::getMovableObjectCollection(const String& typeName)
{
    // lock collection mutex
    OGRE_LOCK_MUTEX(mMovableObjectCollectionMapMutex);

    MovableObjectCollectionMap::iterator i = 
        mMovableObjectCollectionMap.find(typeName);
    if (i == mMovableObjectCollectionMap.end())
    {
        // create
        MovableObjectCollection* newCollection = OGRE_NEW_T(MovableObjectCollection, MEMCATEGORY_SCENE_CONTROL)();
        mMovableObjectCollectionMap[typeName] = newCollection;
        return newCollection;
    }
    else
    {
        return i->second;
    }
}
//---------------------------------------------------------------------
const SceneManager::MovableObjectCollection* 
SceneManager::getMovableObjectCollection(const String& typeName) const
{
    // lock collection mutex
    OGRE_LOCK_MUTEX(mMovableObjectCollectionMapMutex);

    MovableObjectCollectionMap::const_iterator i = 
        mMovableObjectCollectionMap.find(typeName);
    if (i == mMovableObjectCollectionMap.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
            "Object collection named '" + typeName + "' does not exist.", 
            "SceneManager::getMovableObjectCollection");
    }
    else
    {
        return i->second;
    }
}
//---------------------------------------------------------------------
MovableObject* SceneManager::createMovableObject(const String& name, 
    const String& typeName, const NameValuePairList* params)
{
    // Nasty hack to make generalised Camera functions work without breaking add-on SMs
    if (typeName == "Camera")
    {
        return createCamera(name);
    }
    MovableObjectFactory* factory = 
        Root::getSingleton().getMovableObjectFactory(typeName);
    // Check for duplicate names
    MovableObjectCollection* objectMap = getMovableObjectCollection(typeName);

    {
            OGRE_LOCK_MUTEX(objectMap->mutex);

        if (objectMap->map.find(name) != objectMap->map.end())
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
                "An object of type '" + typeName + "' with name '" + name
                + "' already exists.", 
                "SceneManager::createMovableObject");
        }

        MovableObject* newObj = factory->createInstance(name, this, params);
        objectMap->map[name] = newObj;
        return newObj;
    }

}
//---------------------------------------------------------------------
MovableObject* SceneManager::createMovableObject(const String& typeName, const NameValuePairList* params /* = 0 */)
{
    String name = mMovableNameGenerator.generate();
    return createMovableObject(name, typeName, params);
}
//---------------------------------------------------------------------
void SceneManager::destroyMovableObject(const String& name, const String& typeName)
{
    // Nasty hack to make generalised Camera functions work without breaking add-on SMs
    if (typeName == "Camera")
    {
        destroyCamera(name);
        return;
    }
    MovableObjectCollection* objectMap = getMovableObjectCollection(typeName);
    MovableObjectFactory* factory = 
        Root::getSingleton().getMovableObjectFactory(typeName);

    {
            OGRE_LOCK_MUTEX(objectMap->mutex);

        MovableObjectMap::iterator mi = objectMap->map.find(name);
        if (mi != objectMap->map.end())
        {
            factory->destroyInstance(mi->second);
            objectMap->map.erase(mi);
        }
    }
}
//---------------------------------------------------------------------
void SceneManager::destroyAllMovableObjectsByType(const String& typeName)
{
    // Nasty hack to make generalised Camera functions work without breaking add-on SMs
    if (typeName == "Camera")
    {
        destroyAllCameras();
        return;
    }
    MovableObjectCollection* objectMap = getMovableObjectCollection(typeName);
    MovableObjectFactory* factory = 
        Root::getSingleton().getMovableObjectFactory(typeName);
    
    {
        OGRE_LOCK_MUTEX(objectMap->mutex);
        for (auto& m : objectMap->map)
        {
            // Only destroy our own
            if (m.second->_getManager() == this)
            {
                factory->destroyInstance(m.second);
            }
        }
        objectMap->map.clear();
    }
}
//---------------------------------------------------------------------
void SceneManager::destroyAllMovableObjects(void)
{
    // Lock collection mutex
    OGRE_LOCK_MUTEX(mMovableObjectCollectionMapMutex);
    for(auto& c : mMovableObjectCollectionMap)
    {
        MovableObjectCollection* coll = c.second;

        // lock map mutex
        OGRE_LOCK_MUTEX(coll->mutex);

        if (Root::getSingleton().hasMovableObjectFactory(c.first))
        {
            // Only destroy if we have a factory instance; otherwise must be injected
            MovableObjectFactory* factory = 
                Root::getSingleton().getMovableObjectFactory(c.first);
            for (auto& i : coll->map)
            {
                if (i.second->_getManager() == this)
                {
                    factory->destroyInstance(i.second);
                }
            }
        }
        coll->map.clear();
    }
}
//---------------------------------------------------------------------
MovableObject* SceneManager::getMovableObject(const String& name, const String& typeName) const
{
    // Nasty hack to make generalised Camera functions work without breaking add-on SMs
    if (typeName == "Camera")
    {
        return getCamera(name);
    }

    const MovableObjectCollection* objectMap = getMovableObjectCollection(typeName);
    
    {
            OGRE_LOCK_MUTEX(objectMap->mutex);
        MovableObjectMap::const_iterator mi = objectMap->map.find(name);
        if (mi == objectMap->map.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Object named '" + name + "' does not exist.", 
                "SceneManager::getMovableObject");
        }
        return mi->second;
    }
    
}
//-----------------------------------------------------------------------
bool SceneManager::hasMovableObject(const String& name, const String& typeName) const
{
    // Nasty hack to make generalised Camera functions work without breaking add-on SMs
    if (typeName == "Camera")
    {
        return hasCamera(name);
    }
    OGRE_LOCK_MUTEX(mMovableObjectCollectionMapMutex);

    MovableObjectCollectionMap::const_iterator i = 
        mMovableObjectCollectionMap.find(typeName);
    if (i == mMovableObjectCollectionMap.end())
        return false;
    
    {
            OGRE_LOCK_MUTEX(i->second->mutex);
        return (i->second->map.find(name) != i->second->map.end());
    }
}

//---------------------------------------------------------------------
const SceneManager::MovableObjectMap&
SceneManager::getMovableObjects(const String& typeName)
{
    MovableObjectCollection* objectMap = getMovableObjectCollection(typeName);
    return objectMap->map;
}
SceneManager::MovableObjectIterator 
SceneManager::getMovableObjectIterator(const String& typeName)
{
    MovableObjectCollection* objectMap = getMovableObjectCollection(typeName);
    // Iterator not thread safe! Warned in header.
    return MovableObjectIterator(objectMap->map.begin(), objectMap->map.end());
}
//---------------------------------------------------------------------
void SceneManager::destroyMovableObject(MovableObject* m)
{
    OgreAssert(m, "Cannot destroy a null MovableObject");
    destroyMovableObject(m->getName(), m->getMovableType());
}
//---------------------------------------------------------------------
void SceneManager::injectMovableObject(MovableObject* m)
{
    MovableObjectCollection* objectMap = getMovableObjectCollection(m->getMovableType());
    {
            OGRE_LOCK_MUTEX(objectMap->mutex);

        objectMap->map[m->getName()] = m;
    }
}
//---------------------------------------------------------------------
void SceneManager::extractMovableObject(const String& name, const String& typeName)
{
    MovableObjectCollection* objectMap = getMovableObjectCollection(typeName);
    {
            OGRE_LOCK_MUTEX(objectMap->mutex);
        MovableObjectMap::iterator mi = objectMap->map.find(name);
        if (mi != objectMap->map.end())
        {
            // no delete
            objectMap->map.erase(mi);
        }
    }

}
//---------------------------------------------------------------------
void SceneManager::extractMovableObject(MovableObject* m)
{
    extractMovableObject(m->getName(), m->getMovableType());
}
//---------------------------------------------------------------------
void SceneManager::extractAllMovableObjectsByType(const String& typeName)
{
    MovableObjectCollection* objectMap = getMovableObjectCollection(typeName);
    {
            OGRE_LOCK_MUTEX(objectMap->mutex);
        // no deletion
        objectMap->map.clear();
    }
}
//---------------------------------------------------------------------
void SceneManager::_injectRenderWithPass(Pass *pass, Renderable *rend, bool shadowDerivation,
    bool doLightIteration, const LightList* manualLightList)
{
    // render something as if it came from the current queue
    const Pass *usedPass = _setPass(pass, false, shadowDerivation);
    renderSingleObject(rend, usedPass, false, doLightIteration, manualLightList);
}
//---------------------------------------------------------------------
RenderSystem *SceneManager::getDestinationRenderSystem()
{
    return mDestRenderSystem;
}
//---------------------------------------------------------------------
uint32 SceneManager::_getCombinedVisibilityMask(void) const
{
    return mCurrentViewport ?
        mCurrentViewport->getVisibilityMask() & mVisibilityMask : mVisibilityMask;

}
//---------------------------------------------------------------------
const VisibleObjectsBoundsInfo& 
SceneManager::getVisibleObjectsBoundsInfo(const Camera* cam) const
{
    static VisibleObjectsBoundsInfo nullBox;

    CamVisibleObjectsMap::const_iterator camVisObjIt = mCamVisibleObjectsMap.find( cam );

    if ( camVisObjIt == mCamVisibleObjectsMap.end() )
        return nullBox;
    else
        return camVisObjIt->second;
}
const VisibleObjectsBoundsInfo&
SceneManager::getShadowCasterBoundsInfo( const Light* light, size_t iteration ) const
{
    return mShadowRenderer.getShadowCasterBoundsInfo(light, iteration);
}
//---------------------------------------------------------------------
void SceneManager::setQueuedRenderableVisitor(SceneManager::SceneMgrQueuedRenderableVisitor* visitor)
{
    if (visitor)
        mActiveQueuedRenderableVisitor = visitor;
    else
        mActiveQueuedRenderableVisitor = &mDefaultQueuedRenderableVisitor;
}
//---------------------------------------------------------------------
void SceneManager::addLodListener(LodListener *listener)
{
    mLodListeners.insert(listener);
}
//---------------------------------------------------------------------
void SceneManager::removeLodListener(LodListener *listener)
{
    LodListenerSet::iterator it = mLodListeners.find(listener);
    if (it != mLodListeners.end())
        mLodListeners.erase(it);
}
//---------------------------------------------------------------------
void SceneManager::_notifyMovableObjectLodChanged(MovableObjectLodChangedEvent& evt)
{
    // Notify listeners and determine if event needs to be queued
    bool queueEvent = false;
    for (auto *l : mLodListeners)
    {
        if (l->prequeueMovableObjectLodChanged(evt))
            queueEvent = true;
    }

    // Push event onto queue if requested
    if (queueEvent)
        mMovableObjectLodChangedEvents.push_back(evt);
}
//---------------------------------------------------------------------
void SceneManager::_notifyEntityMeshLodChanged(EntityMeshLodChangedEvent& evt)
{
    // Notify listeners and determine if event needs to be queued
    bool queueEvent = false;
    for (auto *l : mLodListeners)
    {
        if (l->prequeueEntityMeshLodChanged(evt))
            queueEvent = true;
    }

    // Push event onto queue if requested
    if (queueEvent)
        mEntityMeshLodChangedEvents.push_back(evt);
}
//---------------------------------------------------------------------
void SceneManager::_notifyEntityMaterialLodChanged(EntityMaterialLodChangedEvent& evt)
{
    // Notify listeners and determine if event needs to be queued
    bool queueEvent = false;
    for (auto *l : mLodListeners)
    {
        if (l->prequeueEntityMaterialLodChanged(evt))
            queueEvent = true;
    }

    // Push event onto queue if requested
    if (queueEvent)
        mEntityMaterialLodChangedEvents.push_back(evt);
}
//---------------------------------------------------------------------
void SceneManager::_handleLodEvents()
{
    // Handle events with each listener
    for (auto *l : mLodListeners)
    {
        for (auto& e : mMovableObjectLodChangedEvents)
            l->postqueueMovableObjectLodChanged(e);

        for (auto& e : mEntityMeshLodChangedEvents)
            l->postqueueEntityMeshLodChanged(e);

        for (auto& e : mEntityMaterialLodChangedEvents)
            l->postqueueEntityMaterialLodChanged(e);
    }

    // Clear event queues
    mMovableObjectLodChangedEvents.clear();
    mEntityMeshLodChangedEvents.clear();
    mEntityMaterialLodChangedEvents.clear();
}
//---------------------------------------------------------------------
void SceneManager::useLights(const LightList* lights, ushort limit)
{
    static LightList NULL_LIGHTS;
    lights = lights ? lights : &NULL_LIGHTS;

    auto hash = FastHash((const char*)lights->data(), lights->size() * sizeof(Light*));
    if(hash != mLastLightHash)
    {
        mLastLightHash = hash;

        // Update any automatic gpu params for lights
        // Other bits of information will have to be looked up
        mAutoParamDataSource->setCurrentLightList(lights);
        mGpuParamsDirty |= GPV_LIGHTS;
    }

    mDestRenderSystem->_useLights(std::min<ushort>(limit, lights->size()));
}
//---------------------------------------------------------------------
void SceneManager::bindGpuProgram(GpuProgram* prog)
{
    // need to dirty the light hash, and params that need resetting, since program params will have been invalidated
    // Use 1 to guarantee changing it (using 0 could result in no change if list is empty)
    // Hash == 1 is almost impossible to achieve otherwise
    mLastLightHash = 1;
    mGpuParamsDirty = (uint16)GPV_ALL;
    mDestRenderSystem->bindGpuProgram(prog);
}
//---------------------------------------------------------------------
void SceneManager::_markGpuParamsDirty(uint16 mask)
{
    mGpuParamsDirty |= mask;
}
//---------------------------------------------------------------------
void SceneManager::updateGpuProgramParameters(const Pass* pass)
{
    if (!mGpuParamsDirty)
        return;

    if (pass->isProgrammable())
    {
        pass->_updateAutoParams(mAutoParamDataSource.get(), mGpuParamsDirty);

        for (int i = 0; i < GPT_COMPUTE_PROGRAM; i++) // compute program is bound via RSComputeOperation
        {
            GpuProgramType t = (GpuProgramType)i;
            if (pass->hasGpuProgram(t))
            {
                mDestRenderSystem->bindGpuProgramParameters(t, pass->getGpuProgramParameters(t),
                                                            mGpuParamsDirty);
            }
        }
    }

    // GLSL and HLSL2 allow FFP state access
    if(mFixedFunctionParams)
    {
        mFixedFunctionParams->_updateAutoParams(mAutoParamDataSource.get(), mGpuParamsDirty);
        mDestRenderSystem->applyFixedFunctionParams(mFixedFunctionParams, mGpuParamsDirty);
    }

    mGpuParamsDirty = 0;
}
//---------------------------------------------------------------------
void SceneManager::_issueRenderOp(Renderable* rend, const Pass* pass)
{
    // Finalise GPU parameter bindings
    if(pass)
        updateGpuProgramParameters(pass);

    if(rend->preRender(this, mDestRenderSystem))
    {
        RenderOperation ro;
        ro.srcRenderable = rend;

        rend->getRenderOperation(ro);

        injectGlobalInstancingDeclaration(ro, mDestRenderSystem);

        mDestRenderSystem->_render(ro);
    }

    rend->postRender(this, mDestRenderSystem);
}
//---------------------------------------------------------------------
VisibleObjectsBoundsInfo::VisibleObjectsBoundsInfo()
{
    reset();
}
//---------------------------------------------------------------------
void VisibleObjectsBoundsInfo::reset()
{
    aabb.setNull();
    receiverAabb.setNull();
    minDistance = minDistanceInFrustum = std::numeric_limits<Real>::infinity();
    maxDistance = maxDistanceInFrustum = 0;
}
//---------------------------------------------------------------------
void VisibleObjectsBoundsInfo::merge(const AxisAlignedBox& boxBounds, const Sphere& sphereBounds, const Camera* cam, bool receiver)
{
    aabb.merge(boxBounds);
    if (receiver)
        receiverAabb.merge(boxBounds);
    // use view matrix to determine distance, works with custom view matrices
    Vector3 vsSpherePos = cam->getViewMatrix(true) * sphereBounds.getCenter();
    Real camDistToCenter = vsSpherePos.length();
    minDistance = std::min(minDistance, std::max((Real)0, camDistToCenter - sphereBounds.getRadius()));
    maxDistance = std::max(maxDistance, camDistToCenter + sphereBounds.getRadius());
    minDistanceInFrustum = std::min(minDistanceInFrustum, std::max((Real)0, camDistToCenter - sphereBounds.getRadius()));
    maxDistanceInFrustum = std::max(maxDistanceInFrustum, camDistToCenter + sphereBounds.getRadius());
}
//---------------------------------------------------------------------
void VisibleObjectsBoundsInfo::mergeNonRenderedButInFrustum(const AxisAlignedBox& boxBounds, const Sphere& sphereBounds, const Camera* cam)
{
    (void)boxBounds;
    // use view matrix to determine distance, works with custom view matrices
    Vector3 vsSpherePos = cam->getViewMatrix(true) * sphereBounds.getCenter();
    Real camDistToCenter = vsSpherePos.length();
    minDistanceInFrustum = std::min(minDistanceInFrustum, std::max((Real)0, camDistToCenter - sphereBounds.getRadius()));
    maxDistanceInFrustum = std::max(maxDistanceInFrustum, camDistToCenter + sphereBounds.getRadius());

}
//---------------------------------------------------------------------

}
