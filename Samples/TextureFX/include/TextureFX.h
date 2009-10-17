#ifndef __TextureFX_H__
#define __TextureFX_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_TextureFX : public SdkSample
{
public:

	Sample_TextureFX()
	{
		mInfo["Title"] = "Texture Effects";
		mInfo["Description"] = "Demonstrates OGRE's many different animated texture effects.";
		mInfo["Thumbnail"] = "thumb_texfx.png";
		mInfo["Category"] = "Unsorted";
		mInfo["Help"] = "Top Left: Multi-frame\nTop Right: Scrolling\nBottom Left: Rotation\nBottom Right: Scaling";
	}

protected:

	void setupContent()
	{
		mSceneMgr->setSkyBox(true, "Examples/TrippySkyBox");

		// set our camera to orbit around the origin and show cursor
		mCameraMan->setStyle(CS_ORBIT);
		mTrayMgr->showCursor();

		// the names of the four materials we will use
		String matNames[] = {"Examples/OgreDance", "Examples/OgreParade", "Examples/OgreSpin", "Examples/OgreWobble"};

		for (unsigned int i = 0; i < 4; i++)
		{
			// create a standard plane entity
			Entity* ent = mSceneMgr->createEntity("Plane" + StringConverter::toString(i + 1), SceneManager::PT_PLANE);

			// attach it to a node, scale it, and position appropriately
			SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			node->setPosition(i % 2 ? 25 : -25, i / 2 ? -25 : 25, 0);
			node->setScale(0.25, 0.25, 0.25);
			node->attachObject(ent);

			ent->setMaterialName(matNames[i]);  // give it the material we prepared
		}
	}
};

#endif
