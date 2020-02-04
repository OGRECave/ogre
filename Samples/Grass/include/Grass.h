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

 Sample_Grass() : GRASS_WIDTH(40), GRASS_HEIGHT(40)
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

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#       pragma pack(push, 1)
#endif
 struct GrassVertex
 {
     float x, y, z;
     float nx, ny, nz;
     float u, v;
 };
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#       pragma pack(pop)
#endif

 void setupContent()
 {
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
     // Make this viewport work with shader generator scheme.
     mViewport->setMaterialScheme(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
#endif
     mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");

     // create a mesh for our ground
     MeshManager::getSingleton().createPlane("ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                             Plane(Vector3::UNIT_Y, 0), 1000, 1000, 20, 20, true, 1, 6, 6, Vector3::UNIT_Z);

     // create a ground entity from our mesh and attach it to the origin
     Entity* ground = mSceneMgr->createEntity("Ground", "ground");
     ground->setMaterialName("Examples/GrassFloor");
     ground->setCastShadows(false);
     mSceneMgr->getRootSceneNode()->attachObject(ground);

     // create our grass mesh, and create a grass entity from it
     createGrassMesh();
     Entity* grass = mSceneMgr->createEntity("Grass", "grass");

     // create a static geometry field, which we will populate with grass
     mField = mSceneMgr->createStaticGeometry("Field");
     mField->setRegionDimensions(Vector3(140, 140, 140));
     mField->setOrigin(Vector3(70, 70, 70));

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

     mCameraMan->setStyle(CS_ORBIT);
     mTrayMgr->showCursor();
     mCameraMan->setYawPitchDist(Degree(0), Degree(25), 200);
 }

 void createGrassMesh()
 {
     MeshPtr mesh = MeshManager::getSingleton().createManual("grass", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

     // create a submesh with the grass material
     SubMesh* sm = mesh->createSubMesh();
     sm->setMaterialName("Examples/GrassBlades");
     sm->useSharedVertices = false;
     sm->vertexData = OGRE_NEW VertexData();
     sm->vertexData->vertexStart = 0;
     sm->vertexData->vertexCount = 12;
     sm->indexData->indexCount = 18;

#if defined(INCLUDE_RTSHADER_SYSTEM)
        MaterialPtr grassMat = MaterialManager::getSingleton().getByName("Examples/GrassBlades");
     grassMat->getTechnique(0)->setSchemeName(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
#endif

     // specify a vertex format declaration for our mesh: 3 floats for position, 3 floats for normal, 2 floats for UV
     VertexDeclaration* decl = sm->vertexData->vertexDeclaration;
     decl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
     decl->addElement(0, sizeof(float) * 3, VET_FLOAT3, VES_NORMAL);
     decl->addElement(0, sizeof(float) * 6, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

     // create a vertex buffer
     HardwareVertexBufferSharedPtr vb = HardwareBufferManager::getSingleton().createVertexBuffer
         (decl->getVertexSize(0), sm->vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

     GrassVertex* verts = (GrassVertex*)vb->lock(HardwareBuffer::HBL_DISCARD);  // start filling in vertex data

     for (unsigned int i = 0; i < 3; i++)  // each grass mesh consists of 3 planes
     {
         // planes intersect along the Y axis with 60 degrees between them
         Real x = Math::Cos(Degree(i * 60)) * GRASS_WIDTH / 2;
         Real z = Math::Sin(Degree(i * 60)) * GRASS_WIDTH / 2;

         for (unsigned int j = 0; j < 4; j++)  // each plane has 4 vertices
         {
             GrassVertex& vert = verts[i * 4 + j];

             vert.x = j < 2 ? -x : x;
             vert.y = j % 2 ? 0 : GRASS_HEIGHT;
             vert.z = j < 2 ? -z : z;

             // all normals point straight up
             vert.nx = 0;
             vert.ny = 1;
             vert.nz = 0;

             vert.u = j < 2 ? 0 : 1;
             vert.v = j % 2;
         }
     }

     vb->unlock();  // commit vertex changes

     sm->vertexData->vertexBufferBinding->setBinding(0, vb);  // bind vertex buffer to our submesh

     // create an index buffer
     sm->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer
         (HardwareIndexBuffer::IT_16BIT, sm->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

     // start filling in index data
     Ogre::uint16* indices = (Ogre::uint16*)sm->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD);

     for (unsigned int i = 0; i < 3; i++)  // each grass mesh consists of 3 planes
     {
         unsigned int off = i * 4;  // each plane consists of 2 triangles

         *indices++ = 0 + off;
         *indices++ = 3 + off;
         *indices++ = 1 + off;

         *indices++ = 0 + off;
         *indices++ = 2 + off;
         *indices++ = 3 + off;
     }

     sm->indexData->indexBuffer->unlock();  // commit index changes
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
         offset.x = Math::Sin(xpos) * 4;
         offset.z = Math::Sin(zpos) * 4;

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

 void cleanupContent()
 {
     ControllerManager::getSingleton().destroyController(mLightController);
     MeshManager::getSingleton().remove("ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
     MeshManager::getSingleton().remove("grass", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
 }

 const Real GRASS_WIDTH;
 const Real GRASS_HEIGHT;
 StaticGeometry* mField;
 AnimationState* mLightAnimState;
 Controller<Real>* mLightController;
};

#endif
