#ifndef __Character_H__
#define __Character_H__

#include "SdkSample.h"
#include "SinbadCharacterController.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Character : public SdkSample
{
public:

    Sample_Character()
    {
        mInfo["Title"] = "Character";
        mInfo["Description"] = "A demo showing 3rd-person character control and use of TagPoints.";
        mInfo["Thumbnail"] = "thumb_char.png";
        mInfo["Category"] = "Animation";
        mInfo["Help"] = "Use the WASD keys to move Sinbad, and the space bar to jump. "
            "Use mouse to look around and mouse wheel to zoom. Press Q to take out or put back "
            "Sinbad's swords. With the swords equipped, you can left click to slice vertically or "
            "right click to slice horizontally. When the swords are not equipped, press E to "
            "start/stop a silly dance routine. "
            "On touch screens, hold to run, drag to look around, tap to jump or slice and "
            "double tap to take out or put back the swords.";
    }

protected:

    void setupContent() override
    {
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        // integrated depth shadows with soft edges
        auto& rtShaderGen = RTShader::ShaderGenerator::getSingleton();
        auto shadows = rtShaderGen.createSubRenderState(RTShader::SRS_SHADOW_MAPPING);
        shadows->setParameter("filter", "pcf4");
        rtShaderGen.getRenderState(MSN_SHADERGEN)->addTemplateSubRenderState(shadows);

        // Make this viewport work with shader generator scheme.
        mViewport->setMaterialScheme(MSN_SHADERGEN);
        // update scheme for FFP supporting rendersystems
        MaterialManager::getSingleton().setActiveScheme(mViewport->getMaterialScheme());
#endif
        // the sky box is drawn first without depth writes, so its distance only has to be
        // inside the fog start
        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox", 30);
        // fade the floor into the hazy horizon of the sky box before the far clip distance
        mSceneMgr->setFog(FOG_LINEAR, ColourValue(0.78, 0.81, 0.80), 0, 50, 450);

        // set shadow properties
        mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
        mSceneMgr->setShadowTexturePixelFormat(PF_DEPTH16);
        mSceneMgr->setShadowTextureSize(1024);
        mSceneMgr->setShadowTextureCount(1);
        mSceneMgr->setShadowTextureSelfShadow(true);
        mSceneMgr->setShadowCasterRenderBackFaces(true);
        mSceneMgr->setShadowFarDistance(50);
        mSceneMgr->setShadowCameraSetup(FocusedShadowCameraSetup::create());

        // disable default camera control so the character can do its own
        mCameraMan->setStyle(CS_MANUAL);

        // the sky reflections of the SinbadPBR materials light the shadows, so little ambient light
        // is needed. Without tone mapping the sun must stay dim enough not to clip.
        mSceneMgr->setAmbientLight(ColourValue(0.05, 0.05, 0.06));

        // a warm afternoon sun
        Light* sun = mSceneMgr->createLight(Light::LT_DIRECTIONAL);
        sun->setDiffuseColour(ColourValue(1.6, 1.52, 1.37));
        sun->setSpecularColour(ColourValue(1.0, 0.95, 0.85));
        SceneNode* sunNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        sunNode->setDirection(Vector3(0.45, -0.7, -0.55).normalisedCopy());
        sunNode->attachObject(sun);

        // create a floor mesh resource, large enough for its edge to vanish in the fog
        MeshManager::getSingleton().createPlane("floor", RGN_DEFAULT,
            Plane(Vector3::UNIT_Y, 0), 1200, 1200, 60, 60, true, 1, 120, 120, Vector3::UNIT_Z);

        // create a floor entity, give it a material, and place it at the origin
        Entity* floor = mSceneMgr->createEntity("Floor", "floor");
        floor->setMaterialName("SinbadPBR/Floor");
        floor->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->attachObject(floor);

//      LogManager::getSingleton().logMessage("creating sinbad");
        // create our character controller
        mChara = std::make_unique<SinbadCharacterController>(mCamera);
        // see the floor up to the end of the fog
        mCamera->setFarClipDistance(600);

        // use the variants of Sinbad's materials for physically based lighting
        for (const auto& mo : mSceneMgr->getMovableObjects(MOT_ENTITY))
        {
            for (auto sub : static_cast<Entity*>(mo.second)->getSubEntities())
            {
                const String& name = sub->getMaterialName();
                if (StringUtil::startsWith(name, "Sinbad/", false))
                    sub->setMaterialName("SinbadPBR/" + name.substr(7));
            }
        }

        mInputListenerChain = TouchAgnosticInputListenerChain(mWindow, {mTrayMgr.get(), this, mChara.get()});

//      LogManager::getSingleton().logMessage("toggling stats");
        mTrayMgr->toggleAdvancedFrameStats();

//      LogManager::getSingleton().logMessage("creating panel");
        StringVector items;
        items.push_back("Help");
        ParamsPanel* help = mTrayMgr->createParamsPanel(TL_TOPLEFT, "HelpMessage", 100, items);
        help->setParamValue("Help", "H / F1");
        
//      LogManager::getSingleton().logMessage("all done");
    }

    void cleanupContent() override
    {
        MeshManager::getSingleton().remove("floor", RGN_DEFAULT);
    }

    std::unique_ptr<SinbadCharacterController> mChara;
};

#endif
