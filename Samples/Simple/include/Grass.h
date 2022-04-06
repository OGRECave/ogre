/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

  Copyright (c) 2000-2014 Torus Knot Software Ltd
  Also see acknowledgements in Readme.html

  You may use this sample code for anything you like, it is not covered by the
  same license as the rest of the engine.
  -----------------------------------------------------------------------------
*/
#ifndef __Grass_H__
#define __Grass_H__

#include "SdkSample.h"
#include "OgreBillboard.h"
#include "OgrePredefinedControllers.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Grass : public SdkSample
{
 public:

 Sample_Grass()
    {
        mInfo["Title"] = "Grass";
        mInfo["Description"] = "Demonstrates how to use the StaticGeometry class to create 'baked' "
            "instances of many meshes, to create effects like grass efficiently.";
        mInfo["Thumbnail"] = "thumb_grass.png";
        mInfo["Category"] = "Environment";
        mInfo["Help"] = "Press B to toggle bounding boxes.";
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        mLightAnimState->addTime(evt.timeSinceLastFrame);   // move the light around
        waveGrass(evt.timeSinceLastFrame);                  // wave the grass around slowly to simulate wind
        return SdkSample::frameRenderingQueued(evt);        // don't forget the parent class updates!
    }

    bool keyPressed(const KeyboardEvent& evt)
    {
        Keycode key = evt.keysym.sym;
        // toggle bounding boxes with B key unless the help dialog is visible
        if (key == 'b' && !mTrayMgr->isDialogVisible())
            mSceneMgr->showBoundingBoxes(!mSceneMgr->getShowBoundingBoxes());
        return SdkSample::keyPressed(evt);
    }

 protected:

    /*=============================================================================
    // This class will be used to pulsate the light and billboard.
    =============================================================================*/
    class LightPulse : public ControllerValue<Real>
 {
 public:

     LightPulse(Light* light, Billboard* billboard, const ColourValue& maxColour, Real maxSize)
     {
         mLight = light;
         mBillboard = billboard;
         mMaxColour = maxColour;
         mMaxSize = maxSize;
     }

     Real getValue () const
     {
         return mIntensity;
     }

     void setValue (Real value)
     {
         mIntensity = value;

         // calculate new colour and apply it to the light and billboard
         ColourValue newColour = mMaxColour * mIntensity;
         mLight->setDiffuseColour(newColour);
         mBillboard->setColour(newColour);

         // calculate new billboard size and apply it
         Real newSize = mMaxSize * mIntensity;
         mBillboard->setDimensions(newSize, newSize);
     }

 protected:

     Light* mLight;
     Billboard* mBillboard;
     ColourValue mMaxColour;
     Real mMaxSize;
     Real mIntensity;
 };

 void setupContent()
 {
     // Make this viewport work with shader generator
     mViewport->setMaterialScheme(MSN_SHADERGEN);
     mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");

     // create a mesh for our ground
     MeshManager::getSingleton().createPlane("ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                             Plane(Vector3::UNIT_Y, 0), 1000, 1000, 20, 20, true, 1, 6, 6, Vector3::UNIT_Z);

     // create a ground entity from our mesh and attach it to the origin
     Entity* ground = mSceneMgr->createEntity("Ground", "ground");
     ground->setMaterialName("Examples/GrassFloor");
     ground->setCastShadows(false);
     mSceneMgr->getRootSceneNode()->attachObject(ground);

     //! [static_geom]
     // create our grass mesh, and create a grass entity from it
     createGrassMesh();
     Entity* grass = mSceneMgr->createEntity("Grass", "grass");

     // create a static geometry field, which we will populate with grass
     mField = mSceneMgr->createStaticGeometry("Field");
     mField->setRegionDimensions(Vector3(140, 140, 140));
     mField->setOrigin(Vector3(70, 70, 70));
     //! [static_geom]

     //! [grass_field]
     // add grass uniformly throughout the field, with some random variations
     for (int x = -280; x < 280; x += 20)
     {
         for (int z = -280; z < 280; z += 20)
         {
             Vector3 pos(x + Math::RangeRandom(-7, 7), 0, z + Math::RangeRandom(-7, 7));
             Quaternion ori(Degree(Math::RangeRandom(0, 359)), Vector3::UNIT_Y);
             Vector3 scale(1, Math::RangeRandom(0.85, 1.15), 1);

             mField->addEntity(grass, pos, ori, scale);
         }
     }

     mField->build();  // build our static geometry (bake the grass into it)
     //! [grass_field]

     // build tangent vectors for the ogre head mesh
     MeshPtr headMesh = MeshManager::getSingleton().load("ogrehead.mesh", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
     unsigned short src, dest;
     if (!headMesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
         headMesh->buildTangentVectors(VES_TANGENT, src, dest);

     // put an ogre head in the middle of the field
     Entity* head = mSceneMgr->createEntity("Head", "ogrehead.mesh");
     head->setMaterialName("RTSS/OffsetMapping");
     mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 30, 0))->attachObject(head);

     setupLighting();

     mTrayMgr->createCheckBox(TL_TOPLEFT, "wind", "Wind")->setChecked(false, false);
     mTrayMgr->showCursor();

     mCameraMan->setStyle(CS_ORBIT);
     mCameraMan->setYawPitchDist(Degree(0), Degree(25), 200);
 }

 void createGrassMesh()
 {
     const float width = 40;
     const float height = 40;

     //! [mo]
     ManualObject obj("GrassObject");
     obj.begin("Examples/GrassBlades");
     //! [mo]
     // to apply wind in vertex shader:
     // obj.begin("Examples/GrassBladesWaver");

     for (unsigned int i = 0; i < 3; i++)  // each grass mesh consists of 3 planes
     {
         //! [grass_base]
         // planes intersect along the Y axis with 60 degrees between them
         Vector3 vec = Quaternion(Degree(i * 60), Vector3::UNIT_Y) * Vector3(width / 2, 0, 0);
         //! [grass_base]

         //! [mo_quad]
         for (unsigned int j = 0; j < 4; j++) // each plane has 4 vertices
         {
             vec.y = j % 2 ? 0 : height;
             obj.position(j < 2 ? Vector3(-1, 1, -1) * vec : vec);
             obj.textureCoord(j < 2 ? 0 : 1, j % 2);

             // all normals point straight up
             obj.normal(0, 1, 0);
         }
         //! [mo_quad]
         //! [mo_index]
         unsigned int off = i * 4;
         // each plane consists of 2 triangles
         obj.triangle(off + 0, off + 3, off + 1);
         obj.triangle(off + 0, off + 2, off + 3);
         //! [mo_index]
     }

     //! [finish]
     obj.end();
     obj.convertToMesh("grass");
     //! [finish]
 }

 void setupLighting()
 {
     mSceneMgr->setAmbientLight(ColourValue::Black);  // turn off ambient light

     ColourValue lightColour(1, 1, 0.3);

     // create a light
     Light* light = mSceneMgr->createLight();
     light->setDiffuseColour(lightColour);
     light->setSpecularColour(1, 1, 0.3);
     light->setAttenuation(1500, 1, 0.0005, 0);

     // create a flare
     BillboardSet* bbs = mSceneMgr->createBillboardSet(1);
     bbs->setMaterialName("Examples/Flare");
     Billboard* bb = bbs->createBillboard(0, 0, 0, lightColour);

     // create a controller for the light intensity, using our LightPulsator class
     ControllerFunctionRealPtr func(OGRE_NEW WaveformControllerFunction(Ogre::WFT_SINE, 0.5, 0.5, 0, 0.5));
     ControllerValueRealPtr dest(OGRE_NEW LightPulse(light, bb, lightColour, 15));
     ControllerManager& cm = ControllerManager::getSingleton();
     mLightController = cm.createController(cm.getFrameTimeSource(), dest, func);

     // create a light node and attach the light and flare to it
     SceneNode* lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
     lightNode->attachObject(light);
     lightNode->attachObject(bbs);

     // set up a 20 second animation for our light, using spline interpolation for nice curves
     Animation* anim = mSceneMgr->createAnimation("LightTrack", 20);
     anim->setInterpolationMode(Animation::IM_SPLINE);

     // create a track to animate the camera's node
     NodeAnimationTrack* track = anim->createNodeTrack(0, lightNode);

     // create keyframes for our track
     track->createNodeKeyFrame(0)->setTranslate(Vector3(42, 77, -42));
     track->createNodeKeyFrame(2)->setTranslate(Vector3(21, 84, -35));
     track->createNodeKeyFrame(4)->setTranslate(Vector3(-21, 91, -14));
     track->createNodeKeyFrame(6)->setTranslate(Vector3(-56, 70, -28));
     track->createNodeKeyFrame(8)->setTranslate(Vector3(-28, 70, -56));
     track->createNodeKeyFrame(10)->setTranslate(Vector3(-14, 63, -28));
     track->createNodeKeyFrame(12)->setTranslate(Vector3(-14, 56, 28));
     track->createNodeKeyFrame(14)->setTranslate(Vector3(0, 35, 84));
     track->createNodeKeyFrame(16)->setTranslate(Vector3(14, 35, 14));
     track->createNodeKeyFrame(18)->setTranslate(Vector3(35, 84, 0));
     track->createNodeKeyFrame(20)->setTranslate(Vector3(42, 77, -42));

     lightNode->setPosition(track->getNodeKeyFrame(0)->getTranslate());

     // create a new animation state to track this
     mLightAnimState = mSceneMgr->createAnimationState("LightTrack");
     mLightAnimState->setEnabled(true);
 }

 void waveGrass(Real timeElapsed)
 {
     static Real xinc = Math::PI * 0.3;
     static Real zinc = Math::PI * 0.44;
     static Real xpos = Math::RangeRandom(-Math::PI, Math::PI);
     static Real zpos = Math::RangeRandom(-Math::PI, Math::PI);
     static Vector4 offset(0, 0, 0, 0);

     xpos += xinc * timeElapsed;
     zpos += zinc * timeElapsed;

     // update vertex program parameters by binding a value to each renderable
     for (const auto& reg : mField->getRegions())
     {
         // a little randomness
         xpos += reg.second->getCentre().x * 0.001;
         zpos += reg.second->getCentre().z * 0.001;
         offset.x = std::sin(xpos) * 4;
         offset.z = std::sin(zpos) * 4;

         for (auto lod : reg.second->getLODBuckets())
         {
             for (const auto& mb : lod->getMaterialBuckets())
             {
                 for (auto geom : mb.second->getGeometryList())
                     geom->setCustomParameter(999, offset);
             }
         }
     }
 }

 void checkBoxToggled(CheckBox* box)
 {
     auto mat = MaterialManager::getSingleton().getByName(box->isChecked() ? "Examples/GrassBladesWaver"
                                                                           : "Examples/GrassBlades");
     for (const auto& reg : mField->getRegions())
     {
         for (auto lod : reg.second->getLODBuckets())
         {
             for (const auto& mb : lod->getMaterialBuckets())
             {
                 mb.second->_setMaterial(mat);
             }
         }
     }
 }

 void cleanupContent()
 {
     ControllerManager::getSingleton().destroyController(mLightController);
     MeshManager::getSingleton().remove("ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
     MeshManager::getSingleton().remove("grass", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
 }

 StaticGeometry* mField;
 AnimationState* mLightAnimState;
 Controller<Real>* mLightController;
};

#endif
