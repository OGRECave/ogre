#ifndef __SphereMapping_H__
#define __SphereMapping_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

/* NOTE: This sample simply displays an object with an sphere mapped material. The really relevant stuff
is all in the material script itself. You won't find anything even vaguely related to sphere mapping in
this source code. Check out the Examples/SphereMappedRustySteel material in Examples.material. */

class _OgreSampleClassExport Sample_SphereMapping : public SdkSample
{
public:

	Sample_SphereMapping()
	{
		mInfo["Title"] = "Sphere Mapping";
		mInfo["Description"] = "Shows the sphere mapping feature of materials. "
			"Sphere maps are not wrapped, and look the same from all directions.";
		mInfo["Thumbnail"] = "thumb_spheremap.png";
		mInfo["Category"] = "Unsorted";
		mBackgroundColour = ColourValue::White;
	}

protected:

	void setupContent()
	{     
		// setup some basic lighting for our scene
		mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
		SceneNode *lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		lightNode->setPosition(20, 80, 50);
		lightNode->attachObject( mSceneMgr->createLight() );

		// set our camera to orbit around the origin and show cursor
		mCameraMan->setStyle(CS_ORBIT);
		mTrayMgr->showCursor();

		// create our model, give it the environment mapped material, and place it at the origin
		Entity *ent = mSceneMgr->createEntity( "ogrehead.mesh",
												ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
												SCENE_STATIC );
		ent->setMaterialName("Examples/SphereMappedRustySteel");
		mSceneMgr->getRootSceneNode( SCENE_STATIC )->attachObject(ent);
	}
};

#endif
