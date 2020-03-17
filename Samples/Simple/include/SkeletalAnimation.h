#ifndef __SkeletalAnimation_H__
#define __SkeletalAnimation_H__

#include "SdkSample.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderExHardwareSkinning.h"
#endif
#include "OgreBillboard.h"

using namespace Ogre;
using namespace OgreBites;

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
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
        , mSrsHardwareSkinning(0)
#endif
    {
        mInfo["Title"] = "Skeletal Animation";
        mInfo["Description"] = "A demo of the skeletal animation feature, including spline animation.";
        mInfo["Thumbnail"] = "thumb_skelanim.png";
        mInfo["Category"] = "Animation";
        mInfo["Help"] = "Controls:\n"
            "WASD to move the camera.  Mouse to look around.\n"
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
                mModelNodes[ i ]->showBoundingBox( false );
                break;
            case kVisualiseOne:
                mModelNodes[ i ]->showBoundingBox( i == mBoundingBoxModelIndex );
                break;
            case kVisualiseAll:
                mModelNodes[ i ]->showBoundingBox( true );
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
    bool keyPressed(const KeyboardEvent& evt)
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

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        for (int i = 0; i < NUM_MODELS; i++)
        {
            if (mAnimStates[i]->getTimePosition() >= ANIM_CHOP)   // when it's time to loop...
            {
                /* We need reposition the scene node origin, since the animation includes translation.
                Position is calculated from an offset to the end position, and rotation is calculated
                from how much the animation turns the character. */

                Quaternion rot(Degree(-60), Vector3::UNIT_Y);   // how much the animation turns the character

                // find current end position and the offset
                Vector3 currEnd = mModelNodes[i]->getOrientation() * mSneakEndPos + mModelNodes[i]->getPosition();
                Vector3 offset = rot * mModelNodes[i]->getOrientation() * -mSneakStartPos;

                mModelNodes[i]->setPosition(currEnd + offset);
                mModelNodes[i]->rotate(rot);

                mAnimStates[i]->setTimePosition(0);   // reset animation time
            }
        }

        return SdkSample::frameRenderingQueued(evt);
    }


protected:

    void setupContent()
    {

#if defined(INCLUDE_RTSHADER_SYSTEM) && defined(RTSHADER_SYSTEM_BUILD_EXT_SHADERS)
        // Make this viewport work with shader generator scheme.
        mShaderGenerator->invalidateScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
        mViewport->setMaterialScheme(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

        //Add the hardware skinning to the shader generator default render state
        mSrsHardwareSkinning = mShaderGenerator->createSubRenderState<RTShader::HardwareSkinning>();
        Ogre::RTShader::RenderState* renderState = mShaderGenerator->getRenderState(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
        renderState->addTemplateSubRenderState(mSrsHardwareSkinning);

        Ogre::MaterialPtr pCast1 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_1weight");
        Ogre::MaterialPtr pCast2 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_2weight");
        Ogre::MaterialPtr pCast3 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_3weight");
        Ogre::MaterialPtr pCast4 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_4weight");

        Ogre::RTShader::HardwareSkinningFactory::getSingleton().setCustomShadowCasterMaterials(
            Ogre::RTShader::ST_DUAL_QUATERNION, pCast1, pCast2, pCast3, pCast4);
#endif
        // set shadow properties
        mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
        mSceneMgr->setShadowTextureSize(512);
        mSceneMgr->setShadowColour(ColourValue(0.6, 0.6, 0.6));
        mSceneMgr->setShadowTextureCount(2);

        // add a little ambient lighting
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        SceneNode* lightsBbsNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        BillboardSet* bbs;

        // Create billboard set for lights .
        bbs = mSceneMgr->createBillboardSet();
        bbs->setMaterialName("Examples/Flare");
        lightsBbsNode->attachObject(bbs);


        // add a blue spotlight
        Light* l = mSceneMgr->createLight();
        Vector3 pos(-40, 180, -10);
        SceneNode* ln = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos);
        ln->attachObject(l);
        l->setType(Light::LT_SPOTLIGHT);
        ln->setDirection(-pos);
        l->setDiffuseColour(0.0, 0.0, 0.5);
        bbs->createBillboard(pos)->setColour(l->getDiffuseColour());
        

        // add a green spotlight.
        l = mSceneMgr->createLight();
        l->setType(Light::LT_SPOTLIGHT);
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
        tweakSneakAnim();

        SceneNode* sn = NULL;
        Entity* ent = NULL;
        AnimationState* as = NULL;

        // make sure we can get the buffers for bbox calculations
        MeshManager::getSingleton().load("jaiqua.mesh",
                                         ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                         HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
                                         HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, true, true);

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
            sn->attachObject(ent);
        
            // enable the entity's sneaking animation at a random speed and loop it manually since translation is involved
            as = ent->getAnimationState("Sneak");
            as->setEnabled(true);
            as->setLoop(false);

            controllerMgr.createController(controllerMgr.getFrameTimeSource(),
                                           AnimationStateControllerValue::create(as, true),
                                           ScaleControllerFunction::create(Math::RangeRandom(0.5, 1.5)));
            mAnimStates.push_back(as);
        }

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
    | The jaiqua sneak animation doesn't loop properly. This method tweaks the
    | animation to loop properly by altering the Spineroot bone track.
    -----------------------------------------------------------------------------*/
    void tweakSneakAnim()
    {
        // get the skeleton, animation, and the node track iterator
        SkeletonPtr skel = static_pointer_cast<Skeleton>(SkeletonManager::getSingleton().load("jaiqua.skeleton",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME));
        Animation* anim = skel->getAnimation("Sneak");
        Animation::NodeTrackIterator tracks = anim->getNodeTrackIterator();

        while (tracks.hasMoreElements())   // for every node track...
        {
            NodeAnimationTrack* track = tracks.getNext();

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
                mSneakStartPos = startKf->getTranslate() + bone->getInitialPosition();
                mSneakEndPos = oldKf.getTranslate() + bone->getInitialPosition();
                mSneakStartPos.y = mSneakEndPos.y;

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
    }

    void cleanupContent()
    {
        mModelNodes.clear();
        mAnimStates.clear();
        MeshManager::getSingleton().remove("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        mSceneMgr->destroyEntity("Jaiqua");

#if defined(INCLUDE_RTSHADER_SYSTEM) && defined(RTSHADER_SYSTEM_BUILD_EXT_SHADERS)
        Ogre::RTShader::RenderState* renderState = mShaderGenerator->getRenderState(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
        renderState->removeTemplateSubRenderState(mSrsHardwareSkinning);
#endif
    }

    const int NUM_MODELS;
    const Real ANIM_CHOP;
    VisualiseBoundingBoxMode mVisualiseBoundingBoxMode;
    int mBoundingBoxModelIndex;  // which model to show the bounding box for
    bool mBoneBoundingBoxes;
    ParamsPanel* mStatusPanel;
    String mBoneBoundingBoxesItemName;

    std::vector<SceneNode*> mModelNodes;
    std::vector<AnimationState*> mAnimStates;

    Vector3 mSneakStartPos;
    Vector3 mSneakEndPos;

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
    RTShader::SubRenderState* mSrsHardwareSkinning;
#endif
};

#endif
