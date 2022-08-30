#ifndef __ShaderSystemMultiLight_H__
#define __ShaderSystemMultiLight_H__

#include "SdkSample.h"
#include "SegmentedDynamicLightManager.h"
#include "RTShaderSRSSegmentedLights.h"
#include "OgreControllerManager.h"
#include "OgreBillboard.h"

/*
Part of the original guidelines under which the RTSS was created was to emulate the fixed pipeline mechanism as close as possible.  
Due to this fact and how it was interpreted using multiple lights in RTSS with the default implementation is problematic. Every light  
requires it's own line in the shader. Every time an object receives a different amount of lights the shader for is invalidated and  lights 
recompiled. Amount of is also limited by the amount of const registers a shader supports. 

The following example shows a different approach to rendering lights in RTSS. A few points on this system
    - Only one directional light is supported.
    - Point lights and spot lights are handled through the same code.
    - Light attenuation is only controlled by range. all other parameters are ignored (to produce more efficient shader programs)
    - point light specular effect is not calculated (to produce more faster shader programs). If any one wants to add it feel free.
    - Large amount of lights can be supported. Limited currently by the size of the texture used to send the light information to the 
        shader (currently set to a 9x9 grid. each grid cell can contain 32 lights).
    - No need to recompile the shader when the number of lights on an object changes
    - Sample requires shader model 3 or higher to run in order
    - The world is divided into a grid of 9x9 cells (can be easily increased). Each cell receives it's own list of lights appropriate 
        only for it. This can be increased depending on your situation.
    - The information of the lights in the grid is transferred onto a texture. Which is sent to the shader.
    - The list of lights is iterated over in the shader through a dynamic loop.


Note:
This code was somewhat inspired by Kojack's "Tons of street lights" (http://www.ogre3d.org/forums/viewtopic.php?t=48412) idea. One of 
the more innovative ideas I've seen of late.

*/

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_ShaderSystemMultiLight : public SdkSample
{
    static const uint8 cPriorityMain = 50;
    static const uint8 cPriorityQuery = 51;
    static const uint8 cPriorityLights = 55;
    static const uint32 cInitialLightCount = 3;

    static const String DEBUG_MODE_CHECKBOX;
    static const String NUM_OF_LIGHTS_SLIDER;
    static const String TWIRL_LIGHTS_CHECKBOX;
public:

    Sample_ShaderSystemMultiLight() :
        mTwirlLights(false),
        mSRSSegLightFactory(NULL),
        mPathNameGen("RTPath")
    {
        mInfo["Title"] = "ShaderSystem - Multi Light";
        mInfo["Description"] = "Shows a possible way to support a large varying amount of spot lights in the RTSS using a relatively simple system."
            "Note in debug mode green and red lines show the light grid. Blue shows the amount of lights processed per grid position.";
        mInfo["Thumbnail"] = "thumb_shadersystemmultilight.png";
        mInfo["Category"] = "Lighting";

    }

    ~Sample_ShaderSystemMultiLight()
    {
        
    }

    void _shutdown() override
    {
        delete SegmentedDynamicLightManager::getSingletonPtr();

        RTShader::RenderState* pMainRenderState = 
            RTShader::ShaderGenerator::getSingleton().createOrRetrieveRenderState(MSN_SHADERGEN).first;
        pMainRenderState->reset();
        
        if (mSRSSegLightFactory)
        {
            RTShader::ShaderGenerator::getSingleton().removeAllShaderBasedTechniques();
            RTShader::ShaderGenerator::getSingleton().removeSubRenderStateFactory(mSRSSegLightFactory);
            delete mSRSSegLightFactory;
            mSRSSegLightFactory = NULL;
        }

        while (mLights.size())
        {
            removeSpotLight();
        }

        SdkSample::_shutdown();
    }

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        // Move the lights along their paths
        for(size_t i = 0 ; i < mLights.size() ; ++i)
        {
            mLights[i].animState->addTime(evt.timeSinceLastFrame);
            if (mTwirlLights)
            {
                mLights[i].dirnode->setDirection(
                    Quaternion(Degree(ControllerManager::getSingleton().getElapsedTime() * 150 + 360 * i / (float)mLights.size()), Vector3::UNIT_Y) *
                    Vector3(0,-1,-1).normalisedCopy(), Node::TS_WORLD);
            }
            else
            {
                mLights[i].dirnode->setDirection(Vector3::NEGATIVE_UNIT_Y, Node::TS_WORLD);
            }
        }
        
                
        return SdkSample::frameRenderingQueued(evt);   // don't forget the parent class updates!
    }

protected:

    void setupContent() override
    {
        mTrayMgr->createThickSlider(TL_BOTTOM, NUM_OF_LIGHTS_SLIDER, "Num of lights", 240, 80, 0, 64, 65)->setValue(cInitialLightCount, false);
        mTrayMgr->createCheckBox(TL_BOTTOM, TWIRL_LIGHTS_CHECKBOX, "Twirl Lights", 240)->setChecked(false, false);
        mTrayMgr->createCheckBox(TL_BOTTOM, DEBUG_MODE_CHECKBOX, "Show Grid", 240)->setChecked(false, false);

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

        setupShaderGenerator();
    
        setupLights();
    }
        
    void cleanupContent() override
    {
        MeshManager::getSingleton().remove("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }

    void setupShaderGenerator()
    {
        new SegmentedDynamicLightManager;

        SegmentedDynamicLightManager::getSingleton().setSceneManager(mSceneMgr);

        RTShader::ShaderGenerator* mGen = RTShader::ShaderGenerator::getSingletonPtr();

        RTShader::RenderState* pMainRenderState = 
            mGen->createOrRetrieveRenderState(MSN_SHADERGEN).first;
        pMainRenderState->reset();

        // If we are using segmented lighting, no auto light update required. (prevent constant invalidation)
        pMainRenderState->setLightCountAutoUpdate(false);

        mSRSSegLightFactory = new RTShaderSRSSegmentedLightsFactory;
        mGen->addSubRenderStateFactory(mSRSSegLightFactory);
        pMainRenderState->addTemplateSubRenderState(
            mGen->createSubRenderState<RTShaderSRSSegmentedLights>());

        mGen->invalidateScheme(Ogre::MSN_SHADERGEN);

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
        state.node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(50, 30, 0));

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
            Vector3 framePos(rand01() * 900 - 500, 10 + rand01() * 100, rand01() * 900 - 500);
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
        state.light->setAttenuation(200,0,0,0);
        state.light->setDiffuseColour(lightColor);
        state.dirnode = state.node->createChildSceneNode();
        state.dirnode->setDirection(Vector3::NEGATIVE_UNIT_Y, Node::TS_WORLD);
        state.dirnode->attachObject(state.light);

        // Attach a flare with the same colour to the light node
        state.bbs = mSceneMgr->createBillboardSet(1);
        Billboard* bb = state.bbs->createBillboard(Vector3::ZERO, lightColor);
        bb->setColour(lightColor);
        state.bbs->setMaterialName("Examples/Flare");
        state.bbs->setRenderQueueGroup(cPriorityLights);
        state.node->attachObject(state.bbs);

        mLights.push_back(state);
    }

    float rand01()
    {
        return (abs(rand()) % 1000) / 1000.0f;
    }

    void setDebugModeState(bool state)
    {
        bool needInvalidate = SegmentedDynamicLightManager::getSingleton().setDebugMode(state);
        if (needInvalidate)
        {
            RTShader::ShaderGenerator::getSingleton().invalidateScheme(MSN_SHADERGEN);
        }
    }

    //--------------------------------------------------------------------------
    void sliderMoved(Slider* slider) override
    {
        if (slider->getName() == NUM_OF_LIGHTS_SLIDER)
        {
            size_t numOfLights = (size_t)slider->getValue();

            while (mLights.size() < numOfLights)
            {
                addSpotLight();
            }

            while (numOfLights < mLights.size())
            {
                removeSpotLight();
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
            setDebugModeState(box->isChecked());
        }
        if (cbName == TWIRL_LIGHTS_CHECKBOX)
        {
            mTwirlLights = box->isChecked();
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

    typedef std::vector<LightState> VecLights;
    VecLights mLights;  
    bool mTwirlLights;

    RTShaderSRSSegmentedLightsFactory* mSRSSegLightFactory;

    NameGenerator mPathNameGen;
};

#endif
