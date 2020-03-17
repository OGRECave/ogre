#ifndef __SkyBox_H__
#define __SkyBox_H__

#include "SdkSample.h"
#include "OgreParticleSystem.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_SkyBox : public SdkSample
{
public:

    Sample_SkyBox()
    {
        mInfo["Title"] = "Sky Box";
        mInfo["Description"] = "Shows how to use skyboxes (fixed-distance cubes used for backgrounds).";
        mInfo["Thumbnail"] = "thumb_skybox.png";
        mInfo["Category"] = "Environment";
    }

protected:

    void setupContent()
    {
        // setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
        mSceneMgr->getRootSceneNode()
            ->createChildSceneNode(Vector3(20, 80, 50))
            ->attachObject(mSceneMgr->createLight());
        
        mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 5000);  // set our skybox

        // create a spaceship model, and place it at the origin
        mSceneMgr->getRootSceneNode()->attachObject(mSceneMgr->createEntity("Razor", "razor.mesh"));

        // create a particle system with 200 quota, then set its material and dimensions
        ParticleSystem* thrusters = mSceneMgr->createParticleSystem(25);
        thrusters->setMaterialName("Examples/Flare");
        thrusters->setDefaultDimensions(25, 25);

        // create two emitters for our thruster particle system
        for (unsigned int i = 0; i < 2; i++)
        {
            ParticleEmitter* emitter = thrusters->addEmitter("Point");  // add a point emitter

            // set the emitter properties
            emitter->setAngle(Degree(3));
            emitter->setTimeToLive(0.5);
            emitter->setEmissionRate(25);
            emitter->setParticleVelocity(25);
            emitter->setDirection(Vector3::NEGATIVE_UNIT_Z);
            emitter->setColour(ColourValue::White, ColourValue::Red);
            emitter->setPosition(Vector3(i == 0 ? 5.7 : -18, 0, 0));
        }

        // attach our thruster particles to the rear of the ship
        mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 6.5, -67))->attachObject(thrusters);

        // set the camera's initial position and orientation
        mCameraNode->setPosition(0, 0, 150);
        mCameraNode->yaw(Degree(5));
    }
};

#endif
