#ifndef __SkeletalAnimation_H__
#define __SkeletalAnimation_H__

#include "SdkSample.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderExHardwareSkinning.h"
#endif
#include "OgreBillboard.h"

using namespace Ogre;
using namespace OgreBites;


class MovingAnimationStateControllerValue : public ControllerValue<float>
{
public:

    enum MoveMethod
    {
        kMovePeriodic,
        kMoveContinuous,
    };

    MovingAnimationStateControllerValue(AnimationState* animationState, Entity* entity, NodeAnimationTrack* track)
    : mAnimationState(animationState)
    , mEntity(entity)
    , mTrack(track)
    , mMethod(kMovePeriodic)
    , mAppliedTranslation(Ogre::Vector3::ZERO)
    , mAppliedRotation(Ogre::Quaternion::IDENTITY)
    {
        assert(animationState);
        assert(entity);
        assert(track);

        // Rotation is applied in the bone's space so we need its initial orientation.
        // Bone intial position will interpretted in our dynamic orientation so we need that initial position.

        Bone * rootBone = entity->getSkeleton()->getBone(track->getHandle());
        mInitialOrientation = rootBone->getInitialOrientation();
        mInitialPosition = rootBone->getInitialPosition();

        // Measure total transformation for root.

        TransformKeyFrame * tkfBeg = track->getNodeKeyFrame(0);
        TransformKeyFrame * tkfEnd = track->getNodeKeyFrame(track->getNumKeyFrames() - 1);
        mTranslation = tkfEnd->getTranslate() - tkfBeg->getTranslate();
        mTranslation.y = 0.0f;

        mRotation = mInitialOrientation * tkfEnd->getRotation() * tkfBeg->getRotation().Inverse() * mInitialOrientation.Inverse();
        // TODO: there's probably a smarter way to limit rotation to y-axis
        Matrix3 mat;
        mRotation.ToRotationMatrix(mat);
        Radian yAngle, zAngle, xAngle;
        mat.ToEulerAnglesYZX(yAngle, zAngle, xAngle);
        mat.FromEulerAnglesYZX(yAngle, Ogre::Radian(0.0f), Ogre::Radian(0.0f));
        mRotation.FromRotationMatrix(mat);

        mRotationInverse = mRotation.Inverse();
    }

    static ControllerValueRealPtr create(AnimationState* animationState, Entity* entity, NodeAnimationTrack* track)
    {
        return std::make_shared<MovingAnimationStateControllerValue>(animationState, entity, track);
    }

    void setMoveMethod(MoveMethod method)
    {
        if (mMethod == kMoveContinuous && method != kMoveContinuous)
        {
            SceneNode* sceneNode = mEntity->getParentSceneNode();
            sceneNode->rotate(mAppliedRotation.Inverse());
            sceneNode->translate(-mAppliedTranslation, Node::TS_LOCAL);

            mAppliedRotation = Quaternion::IDENTITY;
            mAppliedTranslation = Vector3::ZERO;
        }

        mMethod = method;

        // If we move the SceneNode continuously then disable moving the bone, otherwise enable.

        if ((mMethod == kMoveContinuous) && !mAnimationState->hasBlendMask())
        {
            mAnimationState->createBlendMask(mEntity->getSkeleton()->getNumBones(), 1.0f);
        }
        if (mAnimationState->hasBlendMask())
        {
            mAnimationState->setBlendMaskEntry(mTrack->getHandle(), (mMethod == kMoveContinuous) ? 0.0f : 1.0f);
        }
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
        bool looped = loops;

        // Apply Movement

        SceneNode* sceneNode = mEntity->getParentSceneNode();

        if (mMethod == kMoveContinuous)
        {
            // Unapply transform from last frame

            sceneNode->rotate(mAppliedRotation.Inverse());
            sceneNode->translate(-mAppliedTranslation, Node::TS_LOCAL);
        }

        // Apply periodic loop transforms

        while (loops < 0)
        {
            sceneNode->rotate(mRotationInverse);
            sceneNode->translate(-mTranslation, Node::TS_LOCAL);
            loops++;
        }
        while (loops > 0)
        {
            sceneNode->translate(mTranslation, Node::TS_LOCAL);
            sceneNode->rotate(mRotation);
            loops--;
        }

        if (mMethod == kMoveContinuous)
        {
            // Apply transform from this frame

            TransformKeyFrame tkf(0, 0);
            mTrack->getInterpolatedKeyFrame(thisTime, &tkf);

            // We need SnOri and SnPos such that
            // SnOri * BoneBindOri = BoneBindOri * BoneAnimOri
            // SnPos + SnOri * BoneBindPos = BoneBindPos + BoneAnimPos

            mAppliedRotation = mInitialOrientation * tkf.getRotation() * mInitialOrientation.Inverse();
            mAppliedTranslation = mInitialPosition + tkf.getTranslate() - mAppliedRotation * mInitialPosition;

            sceneNode->translate(mAppliedTranslation, Node::TS_LOCAL);
            sceneNode->rotate(mAppliedRotation);
        }

        if (looped && mEntity->getUpdateBoundingBoxFromSkeleton())
        {
            /* With mUpdateBoundingBoxFromSkeleton set, Entity::getBoundingBox() uses the
            skeleton from one frame earlier, which is close enough during smooth animation,
            but when we jump a moving animation back to the begininng we need to force it to
            update the bone positions to avoid culling glitches. TODO: can this be remedied? */
            mEntity->getSkeleton()->setAnimationState(*mAnimationState->getParent());
        }
    }

private:
    AnimationState* mAnimationState;
    Entity* mEntity;
    NodeAnimationTrack* mTrack;
    MoveMethod mMethod;
    Vector3 mInitialPosition;
    Quaternion mInitialOrientation;
    Vector3 mTranslation;
    Quaternion mRotation;
    Quaternion mRotationInverse;
    Vector3 mAppliedTranslation;
    Quaternion mAppliedRotation;
};


class _OgreSampleClassExport Sample_SkeletalAnimation : public SdkSample
{
    enum VisualiseBoundingBoxMode
    {
        kVisualiseNone,
        kVisualiseOne,
        kVisualiseAll
    };
public:
    Sample_SkeletalAnimation() : NUM_MODELS(6), ANIM_CHOP(8)
    {
        mInfo["Title"] = "Skeletal Animation";
        mInfo["Description"] = "A demo of the skeletal animation feature, including spline animation.";
        mInfo["Thumbnail"] = "thumb_skelanim.png";
        mInfo["Category"] = "Animation";
        mInfo["Help"] = "Controls:\n"
            "WASD to move the camera.  Mouse to look around.\n"
            "V toggle visualise bounding boxes.\n"
            "B toggle bone-based bounding boxes on/off.\n"
            "M toggle move-method periodic/continuous.";
        mStatusPanel = NULL;
        mVisualiseBoundingBoxMode = kVisualiseNone;
        mBoundingBoxModelIndex = 0;
        mBoneBoundingBoxes = false;
        mMoveMethod = MovingAnimationStateControllerValue::kMovePeriodic;
        mBoneBoundingBoxesItemName = "Bone AABBs";
        mMoveMethodItemName = "Move Method";
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
                mEntities[ i ]->setDisplayBoundingSphere( false );
                break;
            case kVisualiseOne:
                mModelNodes[ i ]->setDisplaySceneNode( i == mBoundingBoxModelIndex );
                mModelNodes[ i ]->showBoundingBox( i == mBoundingBoxModelIndex );
                mEntities[ i ]->setDisplayBoundingSphere( i == mBoundingBoxModelIndex );
                break;
            case kVisualiseAll:
                mModelNodes[ i ]->setDisplaySceneNode( true );
                mModelNodes[ i ]->showBoundingBox( true );
                mEntities[ i ]->setDisplayBoundingSphere( true );
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
    void setMoveMethod( MovingAnimationStateControllerValue::MoveMethod method )
    {
        mMoveMethod = method;

        if (method == MovingAnimationStateControllerValue::kMovePeriodic)
        {
            mEntities[0]->getMesh()->_setBounds(mSneakBoundsMoving);
        }
        else if (method == MovingAnimationStateControllerValue::kMoveContinuous)
        {
            mEntities[0]->getMesh()->_setBounds(mSneakBoundsStationary);
        }

        // update move method mode for all models
        for (int iModel = 0; iModel < NUM_MODELS; iModel++)
        {
            mControllers[iModel]->setMoveMethod(method);
        }
        // update status panel
        if ( mStatusPanel )
        {
            switch (method)
            {
            case MovingAnimationStateControllerValue::kMovePeriodic:
                mStatusPanel->setParamValue(mMoveMethodItemName, "Periodic");
                break;
            case MovingAnimationStateControllerValue::kMoveContinuous:
                mStatusPanel->setParamValue(mMoveMethodItemName, "Continuous");
                break;
            }

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


            case 'm':
                {
                    // Toggle move method for all models.
                    switch (mMoveMethod)
                    {
                    case MovingAnimationStateControllerValue::kMovePeriodic:
                        setMoveMethod(MovingAnimationStateControllerValue::kMoveContinuous);
                        break;
                    case MovingAnimationStateControllerValue::kMoveContinuous:
                        setMoveMethod(MovingAnimationStateControllerValue::kMovePeriodic);
                        break;
                    }
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
        // TODO: figure out why enabling shadows causes debug SceneNode drawing to block camera
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
        MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
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
        tweakJaiquaMesh();
        tweakSneakAnim();

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

            auto controller = MovingAnimationStateControllerValue::create(as, ent, mSneakSpinerootTrack);

            controllerMgr.createController(controllerMgr.getFrameTimeSource(),
                                           controller,
                                           ScaleControllerFunction::create(Math::RangeRandom(0.5, 1.5)));

            mControllers.push_back((MovingAnimationStateControllerValue*)controller.get());
        }

        generateBoundingBoxes();

        // create name and value for skinning mode
        StringVector names;
        names.push_back("Help");
        names.push_back("Skinning");
        names.push_back(mBoneBoundingBoxesItemName);
        names.push_back(mMoveMethodItemName);
        
        // create a params panel to display the help and skinning mode
        mStatusPanel = mTrayMgr->createParamsPanel(TL_TOPLEFT, "HelpMessage", 240, names);
        mStatusPanel->setParamValue("Help", "H / F1");
        String value = "Software";
        enableBoneBoundingBoxMode( false );  // update status panel entry
        setMoveMethod( MovingAnimationStateControllerValue::kMoveContinuous );  // update status panel entry

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

    /*-----------------------------------------------------------------------------
    | The jaiqua mesh has the vertices baked quite a distance from local origin.
    | This moves the mesh to the origin and moves the skeleton's Spineroot bone.
    -----------------------------------------------------------------------------*/
    void tweakJaiquaMesh()
    {
        // make sure we can get the buffers for bbox calculations
        // TODO: figure out why buffers don't get shadow buffers even though we ask for them...
        MeshPtr mesh = MeshManager::getSingleton().load("jaiqua.mesh",
                                                        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                        HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
                                                        HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, true, true);

        // Get root bone's binding position

        Skeleton * skeleton = mesh->getSkeleton().get();
        Bone * rootBone = skeleton->getBone("Spineroot");
        const Vector3 bindPos = rootBone->getInitialPosition();
        rootBone->setPosition(0.0f, bindPos.y, 0.0f);
        skeleton->setBindingPose();

        // Move all the vertices
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
    void tweakSneakAnim()
    {
        // get the skeleton, animation, and the node track iterator
        SkeletonPtr skel = static_pointer_cast<Skeleton>(SkeletonManager::getSingleton().load("jaiqua.skeleton",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME));

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
                mSneakSpinerootTrack = track;

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

    void generateBoundingBoxes()
    {
        // This is a hacky way to generate bounding boxes
        // for the Sneak animation with movement and without.

        mSneakBoundsMoving.setNull();
        mSneakBoundsStationary.setNull();

        Entity * entity = mEntities[0];
        Skeleton * skeleton = entity->getSkeleton();
        Bone * rootBone = skeleton->getBone("Spineroot");
        Animation * animation = skeleton->getAnimation("Sneak");
        AnimationState * as = entity->getAnimationState("Sneak");

        entity->setUpdateBoundingBoxFromSkeleton(true);

        // Use root track's keyframes for our resolution.

        NodeAnimationTrack * rootTrack = animation->getNodeTrack(rootBone->getHandle());

        for (size_t i = 0; i < rootTrack->getNumKeyFrames(); ++i)
        {
            TransformKeyFrame * kf = rootTrack->getNodeKeyFrame(i);
            const Vector3 rootPos = kf->getTranslate();

            as->setTimePosition(kf->getTime());
            skeleton->setAnimationState(*as->getParent());
            AxisAlignedBox bbox = entity->getBoundingBox();

            mSneakBoundsMoving.merge(bbox);

            bbox.setExtents(bbox.getMinimum() - rootPos, bbox.getMaximum() - rootPos);
            mSneakBoundsStationary.merge(bbox);
        }
    }

    void cleanupContent() override
    {
        mModelNodes.clear();
        mEntities.clear();
        mControllers.clear();
        MeshManager::getSingleton().remove("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }

    const int NUM_MODELS;
    const Real ANIM_CHOP;
    VisualiseBoundingBoxMode mVisualiseBoundingBoxMode;
    int mBoundingBoxModelIndex;  // which model to show the bounding box for
    bool mBoneBoundingBoxes;
    MovingAnimationStateControllerValue::MoveMethod mMoveMethod;
    ParamsPanel* mStatusPanel;
    String mBoneBoundingBoxesItemName;
    String mMoveMethodItemName;

    std::vector<SceneNode*> mModelNodes;
    std::vector<Entity*> mEntities;
    std::vector<MovingAnimationStateControllerValue*> mControllers;

    AxisAlignedBox mSneakBoundsMoving;
    AxisAlignedBox mSneakBoundsStationary;
    Vector3 mSneakTranslate;
    Quaternion mSneakRotation;
    NodeAnimationTrack* mSneakSpinerootTrack;
};

#endif
