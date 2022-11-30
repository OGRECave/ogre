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


#ifndef _WATER_H_
#define _WATER_H_

#include "SdkSample.h"
#include "OgreBillboardParticleRenderer.h"
#include "WaterMesh.h"
#include "OgreParticleSystem.h"

#include <iostream>

using namespace Ogre;
using namespace OgreBites;

// Mesh stuff
#define MESH_NAME "WaterMesh"
#define ENTITY_NAME "WaterEntity"
#define MATERIAL_PREFIX "Examples/Water"
#define MATERIAL_NAME "Examples/Water0"
#define COMPLEXITY 64       // watch out - number of polys is 2*ACCURACY*ACCURACY !
#define PLANE_SIZE 3000.0f
#define CIRCLES_MATERIAL "Examples/Water/Circles"
#define CIRCLE_SIZE 500.0
#define CIRCLE_TIME 0.5f

static void prepareCircleMaterial()
{
    Image bmap(PF_L8, 256, 256);
    bmap.setTo(ColourValue(0.5));
    for(int b=0;b<16;b++) {
        int x0 = b % 4 ;
        int y0 = b / 4 ;
        float radius = 4.0f + 1.4 * (float) b ;
        for(int x=0;x<64;x++) {
            for(int y=0;y<64;y++) {
                Real dist = Math::Sqrt((x-32)*(x-32)+(y-32)*(y-32)); // 0..ca.45
                dist = fabs(dist -radius -2) / 2.0f ;
                dist = dist * 255.0f;
                if (dist>255)
                    dist=255 ;
                uchar colour = 255 - (uchar)dist;
                colour = (uchar)((15 - b) / 15.0f * colour);

                *bmap.getData(x+64*x0, y+64*y0) = colour;
            }
        }
    }

    auto tex = TextureManager::getSingleton().loadImage(CIRCLES_MATERIAL, RGN_DEFAULT, bmap);
    MaterialPtr material = MaterialManager::getSingleton().create(CIRCLES_MATERIAL, RGN_DEFAULT);
    auto texLayer = material->getTechnique(0)->getPass(0)->createTextureUnitState();
    texLayer->setTexture(tex);
    texLayer->setTextureAddressingMode( TextureUnitState::TAM_CLAMP );
    material->setSceneBlending( SBT_ADD );
    material->setLightingEnabled(false);
    material->setDepthWriteEnabled( false ) ;
}

class _OgreSampleClassExport Sample_Water : public SdkSample
{
public:
    Sample_Water(): waterMesh(0)
    {
        mInfo["Title"] = "Water";
        mInfo["Description"] = "A demo of a simple water effect.";
        mInfo["Thumbnail"] = "thumb_water.png";
        mInfo["Category"] = "Environment";
    }
    
protected:
    WaterMesh *waterMesh ;
    Entity *waterEntity ;
    AnimationState* mAnimState;
    SceneNode *headNode ;
    Overlay* waterOverlay ;
    ParticleSystem *particleSystem ;
    ParticleEmitter *particleEmitter ;
    SceneManager *sceneMgr ;

    // Just override the mandatory create scene method
    void setupContent(void) override
    {
        sceneMgr = mSceneMgr ;
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.75, 0.75, 0.75));
        
        // Create a light
        // Accept default settings: point light, white diffuse, just set position
        Light* l = mSceneMgr->createLight("MainLight");
        
        // Create water mesh and entity
        waterMesh = new WaterMesh(MESH_NAME, PLANE_SIZE, COMPLEXITY);
        waterEntity = mSceneMgr->createEntity(ENTITY_NAME,
                                              MESH_NAME);
        //~ waterEntity->setMaterialName(MATERIAL_NAME);
        SceneNode *waterNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        waterNode->attachObject(waterEntity);
        
        // Add a head, give it it's own node
        headNode = waterNode->createChildSceneNode();
        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        headNode->attachObject(ent);
        
        // Make sure the camera track this node
        //~ mCamera->setAutoTracking(true, headNode);
        
        // Create the camera node, set its position & attach camera
        mCameraNode->translate(0, 500, PLANE_SIZE);
        mCameraNode->yaw(Degree(-45));
        
        // Create light node
        SceneNode* lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        lightNode->setPosition(200,300,100);
        lightNode->attachObject(l);
        
        // set up spline animation of light node
        Animation* anim = mSceneMgr->createAnimation("WaterLight", 20);
        NodeAnimationTrack *track ;
        TransformKeyFrame *key ;
        // create a random spline for light
        track = anim->createNodeTrack(0, lightNode);
        track->createNodeKeyFrame(0);
        for(int ff=1;ff<=19;ff++) {
            key = track->createNodeKeyFrame(ff);
            Vector3 lpos (
                          rand()%(int)PLANE_SIZE , //- PLANE_SIZE/2,
                          rand()%300+100,
                          rand()%(int)PLANE_SIZE //- PLANE_SIZE/2
                          );
            key->setTranslate(lpos);
        }
        track->createNodeKeyFrame(20);
        
        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("WaterLight");
        mAnimState->setEnabled(true);
        
        // Put in a bit of fog for the hell of it
        //mSceneMgr->setFog(FOG_EXP, ColourValue::White, 0.0002);
        
        // Let there be rain
        particleSystem = mSceneMgr->createParticleSystem("rain",
                                                         "Examples/Water/Rain");
        particleEmitter = particleSystem->getEmitter(0);
        particleEmitter->setEmissionRate(0);
        SceneNode* rNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        rNode->translate(PLANE_SIZE/2.0f, 3000, PLANE_SIZE/2.0f);
        rNode->attachObject(particleSystem);
        // Fast-forward the rain so it looks more natural
        particleSystem->fastForward(20);
        
        prepareCircleMaterial();
        
        billboardSet = mSceneMgr->createBillboardSet("circles", 100);
        billboardSet->setMaterialName(CIRCLES_MATERIAL);
        billboardSet->setDefaultDimensions(CIRCLE_SIZE, CIRCLE_SIZE);
        billboardSet->setBillboardType(BBT_PERPENDICULAR_COMMON);
        billboardSet->setCommonDirection(Vector3::UNIT_Y);
        billboardSet->setCommonUpVector(Vector3::UNIT_Z);

        billboardSet->setTextureStacksAndSlices(4, 4);

        waterNode->attachObject(billboardSet);

        setupControls();
        setDragLook(true);
        
        timeoutDelay = 0.0f;
    }
    
#define PANEL_WIDTH 200
    void setupControls()
    {
        mTrayMgr->createLabel(TL_TOPLEFT, "GeneralLabel", "General", PANEL_WIDTH);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "FakeNormalsCB", "Fake normals", PANEL_WIDTH);
        mTrayMgr->createCheckBox(TL_TOPLEFT, "SkyboxCB", "Skybox", PANEL_WIDTH);
        mTrayMgr->createThickSlider(TL_TOPLEFT, "HeadDepthSlider", "Head Depth", PANEL_WIDTH, 80, 1, 3, 50)->setValue(2.0f);
        SelectMenu* waterMaterial = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, "WaterMaterialMenu", "Water material", PANEL_WIDTH, 9);
        for (size_t i = 0; i < 9; i++)
        {
            waterMaterial->addItem(MATERIAL_PREFIX + StringConverter::toString(i));
        }
        waterMaterial->selectItem(8);
        mTrayMgr->createLabel(TL_TOPLEFT, "RainLabel", "Rain : [Space]", PANEL_WIDTH);
        
        mTrayMgr->createLabel(TL_TOPRIGHT, "AdvancedLabel", "Advanced", PANEL_WIDTH);
        mTrayMgr->createThickSlider(TL_TOPRIGHT, "RippleSpeedSlider", "Ripple Speed", PANEL_WIDTH, 80, 0, 2, 50)->setValue(0.3, false);
        mTrayMgr->createThickSlider(TL_TOPRIGHT, "DistanceSlider", "Distance", PANEL_WIDTH, 80, 0.1, 5.0, 50)->setValue(0.4, false);
        mTrayMgr->createThickSlider(TL_TOPRIGHT, "ViscositySlider", "Viscosity", PANEL_WIDTH, 80, 0, 1, 50)->setValue(0.05, false);
        mTrayMgr->createThickSlider(TL_TOPRIGHT, "FrameTimeSlider", "FrameTime", PANEL_WIDTH, 80, 0, 1, 61)->setValue(0.13, false);
        
        mTrayMgr->showCursor();
    }
    
    void cleanupContent() override
    {
        delete waterMesh;
        waterMesh = 0;
    }
    
protected:
    Real timeoutDelay ;
    
#define RAIN_HEIGHT_RANDOM 5
#define RAIN_HEIGHT_CONSTANT 5

    struct WaterCircle
    {
        Billboard* bb;
        float time;
    };
    std::list<WaterCircle> circles;
    BillboardSet* billboardSet;
    
    void processCircles(Real timeSinceLastFrame)
    {
        for (auto it = circles.begin(); it != circles.end(); )
        {
            it->time += timeSinceLastFrame;
            if(it->time >= CIRCLE_TIME)
            {
                billboardSet->removeBillboard(it->bb);
                it = circles.erase(it);
                continue;
            }

            it->bb->setTexcoordIndex(it->time / CIRCLE_TIME * 16);
            it++;
        }
    }
    
    void processParticles()
    {
        for (auto particle : particleSystem->_getActiveParticles())
        {
            Vector3 ppos = particle->mPosition;
            if (ppos.y<=0 && particle->mTimeToLive>0) { // hits the water!
                // delete particle
                particle->mTimeToLive = 0.0f;
                // push the water
                float x = ppos.x / PLANE_SIZE * COMPLEXITY ;
                float y = ppos.z / PLANE_SIZE * COMPLEXITY ;
                float h = rand() % RAIN_HEIGHT_RANDOM + RAIN_HEIGHT_CONSTANT ;
                if (x<1) x=1 ;
                if (x>COMPLEXITY-1) x=COMPLEXITY-1;
                if (y<1) y=1 ;
                if (y>COMPLEXITY-1) y=COMPLEXITY-1;
                waterMesh->push(x,y,-h) ;
                auto bb = billboardSet->createBillboard(x*(PLANE_SIZE/COMPLEXITY), 10, y*(PLANE_SIZE/COMPLEXITY));
                circles.push_back({bb, 0});
            }
        }
    }
    
    void sliderMoved(Slider* slider) override
    {
        if (slider->getName() == "HeadDepthSlider")
        {
            headDepth = slider->getValue();
        }
        else if (slider->getName() == "RippleSpeedSlider")
        {
            waterMesh->PARAM_C = slider->getValue();
        }
        else if (slider->getName() == "DistanceSlider")
        {
            waterMesh->PARAM_D = slider->getValue();
        }
        else if (slider->getName() == "ViscositySlider")
        {
            waterMesh->PARAM_U = slider->getValue();
        }
        else if (slider->getName() == "FrameTimeSlider")
        {
            waterMesh->PARAM_T = slider->getValue();
        }
    }
    
    void checkBoxToggled(CheckBox* checkBox) override
    {
        if (checkBox->getName() == "FakeNormalsCB")
        {
            waterMesh->useFakeNormals = checkBox->isChecked();
        }
        else if (checkBox->getName() == "SkyboxCB")
        {
            sceneMgr->setSkyBox(checkBox->isChecked(), "Examples/SceneSkyBox2");
        }
    }
    
    void itemSelected(SelectMenu* menu) override
    {
        //Only one menu in this demo
        const String& materialName = menu->getSelectedItem();
        MaterialPtr material = MaterialManager::getSingleton().getByName(materialName);
        if (!material)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Material "+materialName+"doesn't exist!",
                        "WaterListener::updateMaterial");
        }
        waterEntity->setMaterialName(materialName);
    }
    
    /** Head animation */
    Real headDepth ;
    void animateHead(Real timeSinceLastFrame)
    {
        // sine track? :)
        static double sines[4] = {0,100,200,300};
        static const double adds[4] = {0.3,-1.6,1.1,0.5};
        static Vector3 oldPos = Vector3::UNIT_Z;
        for(int i=0;i<4;i++) {
            sines[i]+=adds[i]*timeSinceLastFrame;
        }
        Real tx = ((sin(sines[0]) + sin(sines[1])) / 4 + 0.5 ) * (float)(COMPLEXITY-2) + 1 ;
        Real ty = ((sin(sines[2]) + sin(sines[3])) / 4 + 0.5 ) * (float)(COMPLEXITY-2) + 1 ;
        waterMesh->push(tx,ty, -headDepth);
        Real step = PLANE_SIZE / COMPLEXITY ;
        headNode->resetToInitialState();
        headNode->scale(3,3,3);
        Vector3 newPos = Vector3(step*tx, headDepth, step*ty);
        Vector3 diffPos = newPos - oldPos ;
        Quaternion headRotation = Vector3::UNIT_Z.getRotationTo(diffPos);
        oldPos = newPos ;
        headNode->translate(newPos);
        headNode->rotate(headRotation);
    }
    
public:
    
    bool keyPressed(const KeyboardEvent& evt) override
    {
        static bool rain = false;

        if(evt.keysym.sym == SDLK_SPACE) {
            rain = !rain;
            particleEmitter->setEmissionRate(rain ? 20.0f : 0.0f);
        }

        return SdkSample::keyPressed(evt);
    }
    
    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        if( SdkSample::frameRenderingQueued(evt) == false )
            return false;
        
        mAnimState->addTime(evt.timeSinceLastFrame);
        
        // rain
        processCircles(evt.timeSinceLastFrame);
        processParticles();
        
        timeoutDelay-=evt.timeSinceLastFrame ;
        if (timeoutDelay<=0)
            timeoutDelay = 0;
        
        animateHead(evt.timeSinceLastFrame);
        
        waterMesh->updateMesh(evt.timeSinceLastFrame);
        
        return true;
    }
};

#endif
