#ifndef __SkyPlane_H__
#define __SkyPlane_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_SkyPlane : public SdkSample
{
public:

	Sample_SkyPlane()
	{
		mInfo["Title"] = "Sky Plane";
		mInfo["Description"] = "Shows how to use skyplanes (fixed-distance planes used for backgrounds).";
		mInfo["Thumbnail"] = "thumb_skyplane.png";
		mInfo["Category"] = "Unsorted";
	}

protected:

	void setupContent()
	{     
		// setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
        mSceneMgr->createLight()->setPosition(20, 80, 50);
        
		// create a skyplane 5000 units away, facing down, 10000 square units large, with 3x texture tiling
        mSceneMgr->setSkyPlane(true, Plane(0, -1, 0, 5000), "Examples/SpaceSkyPlane", 10000, 3);

        // and finally... omg it's a DRAGON!
        mSceneMgr->getRootSceneNode()->attachObject(mSceneMgr->createEntity("Dragon", "dragon.mesh"));

		// turn around and look at the DRAGON!
		mCamera->yaw(Degree(210));
		mCamera->pitch(Degree(-10));
	}
};

#endif
