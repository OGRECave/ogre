#ifndef __SkeletalAnimation_H__
#define __SkeletalAnimation_H__

#include "SdkSample.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderExHardwareSkinning.h"
#endif
#include "OgreBillboard.h"
#include "TimeEvents.h"

using namespace Ogre;
using namespace OgreBites;


/*  The RootMotionApplier applies the transforms from a specified
    NodeAnimationTrack to an Entity's parent SceneNode instead of
    to its Skeleton's Bone.

    Won’t work if root bone has an ancestor that is not
    origin-positioned, identity-oriented, and unity-scaled.

    If root bone has a non-origin-positioned, non-identity-oriented,
    or non-unity-scaled binding pose, playing multiple root-movement
    animations will apply the binding pose to the SceneNode multiple
    times (ie mess it up) (i think)

    TODO:

    - add setEnabled() or disable() or unapply() that unapplies all,
      some, or no components of applied rotation.

    - add option of which components of translation and rotation to
      retain/reset when looping (instead of the currently hardcoded
      reseting of translation.y and retaining of rotation.y)

    - maybe implement scale for shrinking and growing animations lol
*/
class RootMotionApplier
{
public:

    RootMotionApplier(AnimationState* animationState, Entity* entity, NodeAnimationTrack* track)
    : mEntity(entity)
    , mTrack(track)
    , mAppliedTranslation(Vector3::ZERO)
    , mAppliedRotation(Quaternion::IDENTITY)
    {
        assert(animationState);
        assert(entity);
        assert(track);

        // Get root bone binding pose.

        Bone* rootBone = entity->getSkeleton()->getBone(track->getHandle());
        mRootBindingPosition = rootBone->getInitialPosition();
        mRootBindingOrientation = rootBone->getInitialOrientation();
        mRootBindingOrientationInverse = mRootBindingOrientation.Inverse();

        // Measure total transformation for root.

        TransformKeyFrame * tkfBeg = track->getNodeKeyFrame(0);
        TransformKeyFrame * tkfEnd = track->getNodeKeyFrame(track->getNumKeyFrames() - 1);

        Quaternion begRotation = mRootBindingOrientation * tkfBeg->getRotation() * mRootBindingOrientationInverse;
        Quaternion endRotation = mRootBindingOrientation * tkfEnd->getRotation() * mRootBindingOrientationInverse;
        mLoopRotation = endRotation * begRotation.Inverse();

        // TODO: there's probably a smarter way to limit rotation to y-axis
        Matrix3 mat;
        mLoopRotation.ToRotationMatrix(mat);
        Radian yAngle, zAngle, xAngle;
        mat.ToEulerAnglesYZX(yAngle, zAngle, xAngle);
        mat.FromEulerAnglesYZX(yAngle, Radian(0.0f), Radian(0.0f));
        mLoopRotation.FromRotationMatrix(mat);

        mLoopRotationInverse = mLoopRotation.Inverse();

        Vector3 begTranslation = mRootBindingPosition + tkfBeg->getTranslate() - begRotation * mRootBindingPosition;
        Vector3 endTranslation = mRootBindingPosition + tkfEnd->getTranslate() - endRotation * mRootBindingPosition;
        mLoopTranslation = endTranslation - mLoopRotation * begTranslation;
        mLoopTranslation.y = 0.0f;

        // Suppress root bone movement

        if (!animationState->hasBlendMask())
        {
            animationState->createBlendMask(entity->getSkeleton()->getNumBones(), 1.0f);
        }
        animationState->setBlendMaskEntry(track->getHandle(), 0.0f);
    }

    void apply(int loops, float thisTime)
    {
        SceneNode* sceneNode = mEntity->getParentSceneNode();
        TransformKeyFrame tkf(0, 0);

        // Unapply transform from last frame

        sceneNode->rotate(mAppliedRotation.Inverse());
        sceneNode->translate(-mAppliedTranslation, Node::TS_LOCAL);

        // Apply periodic loop transforms

        while (loops < 0)
        {
            sceneNode->rotate(mLoopRotationInverse);
            sceneNode->translate(-mLoopTranslation, Node::TS_LOCAL);
            loops++;
        }
        while (loops > 0)
        {
            sceneNode->translate(mLoopTranslation, Node::TS_LOCAL);
            sceneNode->rotate(mLoopRotation);
            loops--;
        }

        // Apply transform from this frame

        mTrack->getInterpolatedKeyFrame(thisTime, &tkf);

        mAppliedRotation = mRootBindingOrientation * tkf.getRotation() * mRootBindingOrientationInverse;
        mAppliedTranslation = mRootBindingPosition + tkf.getTranslate() - mAppliedRotation * mRootBindingPosition;

        sceneNode->translate(mAppliedTranslation, Node::TS_LOCAL);
        sceneNode->rotate(mAppliedRotation);
    }

private:
    Entity* mEntity;
    NodeAnimationTrack* mTrack;

    Vector3 mRootBindingPosition;
    Quaternion mRootBindingOrientation;
    Quaternion mRootBindingOrientationInverse;

    Vector3 mLoopTranslation;
    Quaternion mLoopRotation;
    Quaternion mLoopRotationInverse;

    Vector3 mAppliedTranslation;
    Quaternion mAppliedRotation;
};


class AnimationUpdater : public ControllerValue<float>
{
public:
    AnimationUpdater(AnimationState* animationState)
    : mAnimationState(animationState)
    {
        assert(animationState);
    }

    float getValue(void) const override
    {
        return mAnimationState->getTimePosition() / mAnimationState->getLength();
    }

    void setValue(float timeDelta) override
    {
        // Don't assume AnimationState::addTime()'s internal time-updating (ie modulo) implementation.

        float lastTime = mAnimationState->getTimePosition();
        mAnimationState->addTime(timeDelta);
        float thisTime = mAnimationState->getTimePosition();

        float length = mAnimationState->getLength();
        bool loop = mAnimationState->getLoop();
        int loops = loop ? (int)std::round((lastTime + timeDelta - thisTime) / length) : 0;

        // Apply Movement

        if (mRootMotionApplier)
        {
            mRootMotionApplier->apply(loops, thisTime);
        }

        // Dispatch Events

        if (mTimeEventDispatcher)
        {
            mTimeEventDispatcher->dispatch(lastTime, thisTime, loops, length);
        }
    }

    void setUseRootMotion(Entity* entity, NodeAnimationTrack* track)
    {
        mRootMotionApplier.reset(new RootMotionApplier(mAnimationState, entity, track));
    }

    RootMotionApplier * getRootMotionApplier() { return mRootMotionApplier.get(); }

    void setUseTimeEvents(bool use)
    {
        if (use)
        {
            if (!mTimeEventDispatcher)
            {
                mTimeEventDispatcher.reset(new TimeEventDispatcher);
            }
        }
        else
        {
            mTimeEventDispatcher.reset();
        }
    }

    TimeEventDispatcher * getTimeEventDispatcher() { return mTimeEventDispatcher.get(); }

private:
    AnimationState* mAnimationState;
    std::unique_ptr<RootMotionApplier> mRootMotionApplier;
    std::unique_ptr<TimeEventDispatcher> mTimeEventDispatcher;
};


/*-----------------------------------------------------------------------------
| The jaiqua mesh has the vertices baked quite a distance from local origin.
| This moves the mesh to the origin and moves the skeleton's Spineroot bone.
-----------------------------------------------------------------------------*/
static void tweakJaiquaMesh(const MeshPtr& mesh, Bone* rootBone)
{
    SkeletonPtr skeleton = mesh->getSkeleton();
    // Get root bone's binding position
    const Vector3 bindPos = rootBone->getInitialPosition();

    // Re-bind it at origin (preserving y)

    rootBone->setPosition(0.0f, bindPos.y, 0.0f);
    skeleton->setBindingPose();

    // Move all the vertices by the same amount
    // There's no shared vertices and only 1 submesh

    const VertexData* vertexData = mesh->getSubMeshes()[0]->vertexData;
    const VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
    HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
    DefaultHardwareBufferManagerBase bfrMgr;
    HardwareVertexBufferPtr shadowBuffer = bfrMgr.createVertexBuffer(vbuf->getVertexSize(), vbuf->getNumVertices(), Ogre::HBU_CPU_ONLY);
    shadowBuffer->copyData(*vbuf);

    HardwareBufferLockGuard vertexLock(shadowBuffer, HardwareBuffer::HBL_NORMAL);
    unsigned char* vertex = static_cast<unsigned char*>(vertexLock.pData);
    float* pFloat;

    for(size_t i = 0; i < vertexData->vertexCount; ++i)
    {
        posElem->baseVertexPointerToElement(vertex, &pFloat);
        pFloat[0] -= bindPos.x;
        pFloat[2] -= bindPos.z;
        vertex += shadowBuffer->getVertexSize();
    }

    vertexLock.unlock();

    vbuf->copyData(*shadowBuffer);
}

/*-----------------------------------------------------------------------------
| The jaiqua sneak animation doesn't loop properly. This method tweaks the
| animation to loop properly by altering the Spineroot bone track.
| We also move the Sneak animation to start closer to the origin.
-----------------------------------------------------------------------------*/
static void tweakSneakAnim(const SkeletonPtr& skel)
{
    // Move Sneak animation closer to origin
    Bone * rootBone = skel->getBone("Spineroot");
    Animation * animation = skel->getAnimation("Sneak");
    NodeAnimationTrack * rootTrack = animation->getNodeTrack(rootBone->getHandle());

    Vector3 start = rootTrack->getNodeKeyFrame(0)->getTranslate();
    start.y = 0.0f;

    for (size_t i = 0; i < rootTrack->getNumKeyFrames(); ++i)
    {
        TransformKeyFrame * kf = rootTrack->getNodeKeyFrame(i);
        kf->setTranslate(kf->getTranslate() - start);
    }

    // Tweak Sneak animation
    const float ANIM_CHOP = 8.0f;
    for (const auto& it : animation->_getNodeTrackList()) // for every node track...
    {
        NodeAnimationTrack* track = it.second;

        // get the keyframe at the chopping point
        TransformKeyFrame oldKf(0, 0);
        track->getInterpolatedKeyFrame(ANIM_CHOP, &oldKf);

        // drop all keyframes after the chopping point
        while (track->getKeyFrame(track->getNumKeyFrames()-1)->getTime() >= ANIM_CHOP - 0.3f)
            track->removeKeyFrame(track->getNumKeyFrames()-1);

        // create a new keyframe at chopping point, and get the first keyframe
        TransformKeyFrame* newKf = track->createNodeKeyFrame(ANIM_CHOP);
        TransformKeyFrame* startKf = track->getNodeKeyFrame(0);

        Bone* bone = skel->getBone(track->getHandle());

        if (bone->getName() == "Spineroot")   // adjust spine root relative to new location
        {
            newKf->setTranslate(oldKf.getTranslate());
            newKf->setRotation(oldKf.getRotation());
            newKf->setScale(oldKf.getScale());
        }
        else   // make all other bones loop back
        {
            newKf->setTranslate(startKf->getTranslate());
            newKf->setRotation(startKf->getRotation());
            newKf->setScale(startKf->getScale());
        }
    }

    animation->setLength(ANIM_CHOP);
}

static void generateBoundingBox(Entity * entity)
{
    // This is a hacky way to make a close-fitting bounding box
    // for the Sneak animation with the root movement suppressed.
    Skeleton * skeleton = entity->getSkeleton();
    Bone * rootBone = skeleton->getBone("Spineroot");
    Animation * animation = skeleton->getAnimation("Sneak");
    AnimationState * as = entity->getAnimationState("Sneak");
    NodeAnimationTrack * rootTrack = animation->getNodeTrack(rootBone->getHandle());

    // Suppress root-bone movement.

    if (!as->hasBlendMask())
    {
        as->createBlendMask(skeleton->getNumBones(), 1.0f);
    }
    as->setBlendMaskEntry(rootBone->getHandle(), 0.0f);
    as->setEnabled(true);

    // Generate bounding box from skeleton.

    entity->setUpdateBoundingBoxFromSkeleton(true);

    AxisAlignedBox sneakBounds = AxisAlignedBox::EXTENT_NULL;

    for (size_t i = 0; i < rootTrack->getNumKeyFrames(); ++i)
    {
        TransformKeyFrame * kf = rootTrack->getNodeKeyFrame(i);

        as->setTimePosition(kf->getTime());
        entity->_updateSkeleton();

        AxisAlignedBox bbox = entity->getBoundingBox();
        sneakBounds.merge(bbox);
    }

    entity->getMesh()->_setBounds(sneakBounds);
}

class _OgreSampleClassExport Sample_SkeletalAnimation : public SdkSample
{
    enum VisualiseBoundingBoxMode
    {
        kVisualiseNone,
        kVisualiseOne,
        kVisualiseAll
    };
public:
    Sample_SkeletalAnimation() : NUM_MODELS(6)
    {
        mInfo["Title"] = "Skeletal Animation";
        mInfo["Description"] = "Demonstrates advanced skeletal animation techniques including root motion and hardware skinning.";
        mInfo["Thumbnail"] = "thumb_skelanim.png";
        mInfo["Category"] = "Animation";
        mInfo["Help"] = "Controls:\n"
            "V toggle visualise bounding boxes.\n"
            "B toggle bone-based bounding boxes on/off.";
        mStatusPanel = NULL;
        mVisualiseBoundingBoxMode = kVisualiseNone;
        mBoundingBoxModelIndex = 0;
        mBoneBoundingBoxes = false;
        mBoneBoundingBoxesItemName = "Bone AABBs";
    }

    void setVisualiseBoundingBoxMode( VisualiseBoundingBoxMode mode )
    {
        mVisualiseBoundingBoxMode = mode;
        for (int i = 0; i < NUM_MODELS; i++)
        {
            switch (mVisualiseBoundingBoxMode)
            {
            case kVisualiseNone:
                mModelNodes[ i ]->setDisplaySceneNode( false );
                mModelNodes[ i ]->showBoundingBox( false );
                mEntities[ i ]->showBoundingSphere( false );
                break;
            case kVisualiseOne:
                mModelNodes[ i ]->setDisplaySceneNode( i == mBoundingBoxModelIndex );
                mModelNodes[ i ]->showBoundingBox( i == mBoundingBoxModelIndex );
                mEntities[ i ]->showBoundingSphere( i == mBoundingBoxModelIndex );
                break;
            case kVisualiseAll:
                mModelNodes[ i ]->setDisplaySceneNode( true );
                mModelNodes[ i ]->showBoundingBox( true );
                mEntities[ i ]->showBoundingSphere( true );
                break;
            }
        }
    }
    void enableBoneBoundingBoxMode( bool enable )
    {
        // update bone bounding box mode for all models
        mBoneBoundingBoxes = enable;
        for (int iModel = 0; iModel < NUM_MODELS; iModel++)
        {
            SceneNode* node = mModelNodes[iModel];
            for (unsigned int iObj = 0; iObj < node->numAttachedObjects(); ++iObj)
            {
                if (Entity* ent = dynamic_cast<Entity*>( node->getAttachedObject( iObj ) ))
                {
                    ent->setUpdateBoundingBoxFromSkeleton( mBoneBoundingBoxes );
                    Node::queueNeedUpdate( node );  // when turning off bone bounding boxes, need to force an update
                }
            }
        }
        // update status panel
        if ( mStatusPanel )
        {
            mStatusPanel->setParamValue(mBoneBoundingBoxesItemName, mBoneBoundingBoxes ? "On" : "Off");
        }
    }
    bool keyPressed(const KeyboardEvent& evt) override
    {
        if ( !mTrayMgr->isDialogVisible() )
        {
            // Handle keypresses.
            switch (evt.keysym.sym)
            {
            case 'v':
                // Toggle visualise bounding boxes.
                switch (mVisualiseBoundingBoxMode)
                {
                case kVisualiseNone:
                    setVisualiseBoundingBoxMode( kVisualiseOne );
                    break;
                case kVisualiseOne:
                    setVisualiseBoundingBoxMode( kVisualiseAll );
                    break;
                case kVisualiseAll:
                    setVisualiseBoundingBoxMode( kVisualiseNone );
                    break;
                }
                return true;
                break;

            case 'b':
                {
                    // Toggle bone based bounding boxes for all models.
                    enableBoneBoundingBoxMode( ! mBoneBoundingBoxes );
                    return true;
                }
                break;
            default:
                break;
            }
        }
        return SdkSample::keyPressed(evt);
    }

protected:

    void setupContent() override
    {

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        // Make this viewport work with shader generator scheme.
        mShaderGenerator->invalidateScheme(MSN_SHADERGEN);
        mViewport->setMaterialScheme(MSN_SHADERGEN);

        //Add the hardware skinning to the shader generator default render state
        auto srsHardwareSkinning = mShaderGenerator->createSubRenderState(RTShader::SRS_HARDWARE_SKINNING);
        Ogre::RTShader::RenderState* renderState = mShaderGenerator->getRenderState(MSN_SHADERGEN);
        renderState->addTemplateSubRenderState(srsHardwareSkinning);

        Ogre::MaterialPtr pCast1 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_1weight");
        Ogre::MaterialPtr pCast2 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_2weight");
        Ogre::MaterialPtr pCast3 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_3weight");
        Ogre::MaterialPtr pCast4 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_4weight");

        Ogre::RTShader::HardwareSkinningFactory::setCustomShadowCasterMaterials(Ogre::RTShader::ST_DUAL_QUATERNION,
                                                                                pCast1, pCast2, pCast3, pCast4);


        auto srs = mShaderGenerator->createSubRenderState(RTShader::SRS_SHADOW_MAPPING);
        srs->setParameter("light_count", "2");
        renderState->addTemplateSubRenderState(srs);

        // set shadow properties
        mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED);
        mSceneMgr->setShadowTextureCount(2);
        mSceneMgr->setShadowTextureSize(512);
#endif

        // add a little ambient lighting
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));

        SceneNode* lightsBbsNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        BillboardSet* bbs;

        // Create billboard set for lights .
        bbs = mSceneMgr->createBillboardSet();
        bbs->setMaterialName("Examples/Flare");
        lightsBbsNode->attachObject(bbs);


        // add a blue spotlight
        Light* l = mSceneMgr->createLight(Light::LT_SPOTLIGHT);
        Vector3 pos(-40, 180, -10);
        SceneNode* ln = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos);
        ln->attachObject(l);
        ln->setDirection(-pos);
        l->setDiffuseColour(0.0, 0.0, 0.5);
        bbs->createBillboard(pos)->setColour(l->getDiffuseColour());


        // add a green spotlight.
        l = mSceneMgr->createLight(Light::LT_SPOTLIGHT);
        pos = Vector3(0, 150, -100);
        ln = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos);
        ln->attachObject(l);
        ln->setDirection(-pos);
        l->setDiffuseColour(0.0, 0.5, 0.0);
        bbs->createBillboard(pos)->setColour(l->getDiffuseColour());

        // create a floor mesh resource
        MeshManager::getSingleton().createPlane("floor", RGN_DEFAULT,
            Plane(Vector3::UNIT_Y, -1), 250, 250, 25, 25, true, 1, 15, 15, Vector3::UNIT_Z);

        // add a floor to our scene using the floor mesh we created
        Entity* floor = mSceneMgr->createEntity("Floor", "floor");
        floor->setMaterialName("Examples/Rockwall");
        floor->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->attachObject(floor);

        // set camera initial transform and speed
        mCameraMan->setStyle(CS_ORBIT);
        mTrayMgr->showCursor();
        mCameraMan->setYawPitchDist(Degree(0), Degree(25), 100);
        mCameraMan->setTopSpeed(50);

        setupModels();
    }

    void setupModels()
    {
        // Load the mesh with shadow buffers so we can access vertex data
        MeshPtr mesh = MeshManager::getSingleton().load("jaiqua.mesh", RGN_DEFAULT, HBU_CPU_TO_GPU, HBU_CPU_TO_GPU, true, true);
        SkeletonPtr skeleton = mesh->getSkeleton();
        Bone * rootBone = skeleton->getBone("Spineroot");
        Animation * animation = skeleton->getAnimation("Sneak");
        NodeAnimationTrack * sneakRootTrack = animation->getNodeTrack(rootBone->getHandle());

        tweakJaiquaMesh(mesh, rootBone);
        tweakSneakAnim(skeleton);

        SceneNode* sn = NULL;
        Entity* ent = NULL;
        AnimationState* as = NULL;

        auto& controllerMgr = ControllerManager::getSingleton();

        for (int i = 0; i < NUM_MODELS; i++)
        {
            // create scene nodes for the models at regular angular intervals
            sn = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            sn->yaw(Radian(Math::TWO_PI * (float)i / (float)NUM_MODELS));
            sn->translate(0, 0, -20, Node::TS_LOCAL);
            mModelNodes.push_back(sn);

            // create and attach a jaiqua entity
            ent = mSceneMgr->createEntity("Jaiqua" + StringConverter::toString(i + 1), "jaiqua.mesh");
            ent->setMaterialName("jaiqua");
            mEntities.push_back(ent);
            sn->attachObject(ent);

            // enable the entity's sneaking animation at a random speed and loop it manually since translation is involved
            as = ent->getAnimationState("Sneak");
            as->setEnabled(true);
            as->setLoop(true);

            auto updater = std::make_shared<AnimationUpdater>(as);
            updater->setUseRootMotion(ent, sneakRootTrack);

            controllerMgr.createController(controllerMgr.getFrameTimeSource(),
                                           updater,
                                           ScaleControllerFunction::create(Math::RangeRandom(0.5, 1.5)));

            mAnimUpdaters.push_back(updater.get());
        }

        generateBoundingBox(mEntities[0]);

        // create name and value for skinning mode
        StringVector names;
        names.push_back("Help");
        names.push_back("Skinning");
        names.push_back(mBoneBoundingBoxesItemName);

        // create a params panel to display the help and skinning mode
        mStatusPanel = mTrayMgr->createParamsPanel(TL_TOPLEFT, "HelpMessage", 200, names);
        mStatusPanel->setParamValue("Help", "H / F1");
        String value = "Software";
        enableBoneBoundingBoxMode( false );  // update status panel entry

        // make sure we query the correct scheme
        MaterialManager::getSingleton().setActiveScheme(mViewport->getMaterialScheme());

        // change the value if hardware skinning is enabled
        MaterialPtr entityMaterial = ent->getSubEntity(0)->getMaterial();
        if(entityMaterial)
        {
            Technique* bestTechnique = entityMaterial->getBestTechnique();
            if(bestTechnique)
            {
                Pass* pass = bestTechnique->getPass(0);
                if (pass && pass->hasVertexProgram() && pass->getVertexProgram()->isSkeletalAnimationIncluded())
                {
                    value = "Hardware";
                }
            }
        }
        mStatusPanel->setParamValue("Skinning", value);
    }

    void cleanupContent() override
    {
        mModelNodes.clear();
        mEntities.clear();
        mAnimUpdaters.clear();
        MeshManager::getSingleton().remove("floor", RGN_DEFAULT);
    }

    const int NUM_MODELS;
    VisualiseBoundingBoxMode mVisualiseBoundingBoxMode;
    int mBoundingBoxModelIndex;  // which model to show the bounding box for
    bool mBoneBoundingBoxes;
    ParamsPanel* mStatusPanel;
    String mBoneBoundingBoxesItemName;

    std::vector<SceneNode*> mModelNodes;
    std::vector<Entity*> mEntities;
    std::vector<AnimationUpdater*> mAnimUpdaters;
};

#endif
