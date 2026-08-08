#ifndef __ShaderSystemMultiLight_H__
#define __ShaderSystemMultiLight_H__

#include "SdkSample.h"
#include "OgreControllerManager.h"
#include "OgreBillboard.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_ShaderSystemMultiLight : public SdkSample
{
    static const uint8 cPriorityMain = 50;
    static const uint8 cPriorityQuery = 51;
    static const uint8 cPriorityLights = 55;
    static const uint32 cInitialLightCount = 3;

    static constexpr const char* DEBUG_MODE_CHECKBOX = "DebugModeCheckbox";
    static constexpr const char* NUM_OF_LIGHTS_SLIDER = "NumOfLightsSlider";
    static constexpr const char* CLUSTERED_CULLING_CHECKBOX = "ClusteredCullingCheckbox";
public:

    Sample_ShaderSystemMultiLight() :
        mPathNameGen("RTPath")
    {
        mInfo["Title"] = "ShaderSystem - Multi Light";
        mInfo["Description"] = "Shows a possible way to support a large varying amount of spot lights";
        mInfo["Thumbnail"] = "thumb_shadersystemmultilight.png";
        mInfo["Category"] = "Lighting";

    }

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        // Move the lights along their paths
        for(size_t i = 0 ; i < mLights.size() ; ++i)
        {
            mLights[i].animState->addTime(evt.timeSinceLastFrame);
        }

        return SdkSample::frameRenderingQueued(evt);   // don't forget the parent class updates!
    }

protected:

    void setupContent() override
    {
        mTrayMgr->createThickSlider(TL_BOTTOM, NUM_OF_LIGHTS_SLIDER, "Num of lights", 240, 80, 0, 128, 129)->setValue(cInitialLightCount, false);
        mTrayMgr->createCheckBox(TL_BOTTOM, CLUSTERED_CULLING_CHECKBOX, "Clustered Light Culling", 240)->setChecked(true, false);
        mTrayMgr->createCheckBox(TL_BOTTOM, DEBUG_MODE_CHECKBOX, "Show Occupancy", 240)->setChecked(false, false);

        mCamera->setNearClipDistance(30);

        // Set our camera to orbit around the origin at a suitable distance
        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Degree(0), Degree(25), 600);

        mTrayMgr->showCursor();

        // create a floor mesh resource
        MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Plane(Vector3::UNIT_Y, -30), 1000, 1000, 10, 10, true, 1, 8, 8, Vector3::UNIT_Z);

        // create a floor entity, give it a material, and place it at the origin
        Entity* floor = mSceneMgr->createEntity("Floor", "floor");
        floor->setMaterialName("Examples/BumpyMetal");
        mSceneMgr->getRootSceneNode()->attachObject(floor);

        // Create an ogre head and place it at the origin
        Entity* head = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        head->setRenderQueueGroup(cPriorityMain);
        mSceneMgr->getRootSceneNode()->attachObject(head);
   
        setupLights();
        setupShaderGenerator();
    }
        
    void cleanupContent() override
    {
        MeshManager::getSingleton().remove("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }

    void setupShaderGenerator()
    {
        RTShader::RenderState* pMainRenderState = mShaderGenerator->getRenderState(MSN_SHADERGEN);
        pMainRenderState->resetToBuiltinSubRenderStates();
        pMainRenderState->setLightCountAutoUpdate(false);
        pMainRenderState->setLightCount(mLights.size() + 1); // +1 for the directional light
        pMainRenderState->addTemplateSubRenderState(
            mShaderGenerator->createSubRenderState(RTShader::SRS_COOK_TORRANCE_LIGHTING));

        if(mClusteredLightCullingEnabled)
        {
            mClusteredLightCullingSRS = mShaderGenerator->createSubRenderState(RTShader::SRS_CLUSTERED_LIGHT_CULLING);
            pMainRenderState->addTemplateSubRenderState(mClusteredLightCullingSRS);
        }

        mShaderGenerator->invalidateScheme(Ogre::MSN_SHADERGEN);

        // Make this viewport work with shader generator scheme.
        mViewport->setMaterialScheme(MSN_SHADERGEN);
    }


    void setupLights()
    {
        mSceneMgr->setAmbientLight(ColourValue(0.1, 0.1, 0.1));
        // set the single directional light
        Light* light = mSceneMgr->createLight();
        light->setType(Light::LT_DIRECTIONAL);
        light->setDiffuseColour(ColourValue(0.1, 0.1, 0.1));
        light->setCastShadows(false);
        
        auto ln = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ln->setDirection(Vector3(-1,-1,0).normalisedCopy());
        ln->attachObject(light);

        for(unsigned int i = 0 ; i < cInitialLightCount ; ++i)
        {
            addSpotLight();
        }
    }

    void addSpotLight()
    {
        LightState state;
        
        // Create a light node
        state.node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(50, 10, 0));

        String animName = mPathNameGen.generate();
        // Create a 14 second animation with spline interpolation

        const int animPoints = 5;
        const int animTimeBetweenPoints = 4;
        state.anim = mSceneMgr->createAnimation(animName, animPoints * animTimeBetweenPoints);
        state.anim->setInterpolationMode(Animation::IM_SPLINE);

        state.track = state.anim->createNodeTrack(1, state.node);  // Create a node track for our animation

        // Enter keyframes for our track to define a path for the light to follow
        Vector3 firstFramePos;
        for(int i = 0 ; i <= animPoints ; ++i)
        {
            Vector3 framePos(rand01() * 900 - 500, rand01() * 20, rand01() * 900 - 500);
            if (i == 0)
            {
                firstFramePos = framePos;
            }
            if (i == animPoints)
            {
                framePos = firstFramePos;
            }
            state.track->createNodeKeyFrame(i * animTimeBetweenPoints)->setTranslate(framePos);
        }


        ColourValue lightColor(rand01(), rand01(), rand01());
        float complement = 1 - std::max<float>(std::max<float>(lightColor.r, lightColor.g), lightColor.b);
        lightColor.r += complement;
        lightColor.g += complement;
        lightColor.b += complement;
        
        // Create an animation state from the animation and enable it
        state.animState = mSceneMgr->createAnimationState(animName);
        state.animState->setEnabled(true);

        // Attach a light with the same colour to the light node
        state.light = mSceneMgr->createLight();
        state.light->setCastShadows(false);
        state.light->setType(mLights.size() % 10 ? Light::LT_SPOTLIGHT : Light::LT_POINT);
        state.light->setAttenuation(50,1,0,0);
        state.light->setDiffuseColour(lightColor);
        state.dirnode = state.node->createChildSceneNode();
        state.dirnode->setDirection(Vector3::NEGATIVE_UNIT_Y, Node::TS_WORLD);
        state.dirnode->attachObject(state.light);

        // Attach a flare with the same colour to the light node
        state.bbs = mSceneMgr->createBillboardSet(1);
        Billboard* bb = state.bbs->createBillboard(Vector3::ZERO, lightColor);
        bb->setDimensions(25, 25);
        bb->setColour(lightColor);
        state.bbs->setMaterialName("Examples/Flare");
        state.bbs->setRenderQueueGroup(cPriorityLights);
        state.node->attachObject(state.bbs);

        mLights.push_back(state);
    }

    float rand01()
    {
        return Math::UnitRandom();
    }

    //--------------------------------------------------------------------------
    void sliderMoved(Slider* slider) override
    {
        if (slider->getName() == NUM_OF_LIGHTS_SLIDER)
        {
            auto numOfLights = slider->getValue();

            while (mLights.size() < numOfLights)
            {
                addSpotLight();
            }

            while (numOfLights < mLights.size())
            {
                removeSpotLight();
            }

            auto rs = mShaderGenerator->getRenderState(MSN_SHADERGEN);
            if(numOfLights > rs->getLightCount())
            {
                rs->setLightCount(numOfLights + 1);
                mShaderGenerator->invalidateScheme(MSN_SHADERGEN);
            }
        }   
    }

    void removeSpotLight()
    {
        if (!mLights.empty())
        {
            LightState& state = mLights[mLights.size() - 1];
        
            // Delete the nodes
            mSceneMgr->destroyBillboardSet(state.bbs);
            mSceneMgr->destroyLight(state.light);
            mSceneMgr->destroyAnimationState(state.anim->getName());
            mSceneMgr->destroyAnimation(state.anim->getName());
            mSceneMgr->destroySceneNode(state.node);
            

            mLights.resize(mLights.size() - 1);
        }
    }

    
    void checkBoxToggled(CheckBox* box) override
    {
        const String& cbName = box->getName();

        if (cbName == DEBUG_MODE_CHECKBOX)
        {
            if(!mClusteredLightCullingEnabled)
            {
                box->setChecked(false, false);
                return;
            }
            mClusteredLightCullingSRS->setParameter("debug", box->isChecked() ? "true" : "false");
            mShaderGenerator->invalidateScheme(MSN_SHADERGEN);
        }
        if (cbName == CLUSTERED_CULLING_CHECKBOX)
        {
            mClusteredLightCullingEnabled = box->isChecked();
            setupShaderGenerator();
            if(!mClusteredLightCullingEnabled)
            {
                mClusteredLightCullingSRS = NULL;
            }
        }
    }
private:

    struct LightState
    {
        SceneNode* node;
        SceneNode* dirnode;
        Animation* anim;
        NodeAnimationTrack* track;
        AnimationState* animState;
        Light* light;
        BillboardSet* bbs;
    };

    std::vector<LightState> mLights;
    bool mClusteredLightCullingEnabled = true;

    RTShader::SubRenderState* mClusteredLightCullingSRS = NULL;

    NameGenerator mPathNameGen;
};

#endif
