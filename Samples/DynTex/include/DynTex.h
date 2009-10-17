#ifndef __DynTex_H__
#define __DynTex_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_DynTex : public SdkSample
{
public:

	Sample_DynTex() : TEXTURE_SIZE(128), SQR_BRUSH_RADIUS(Math::Sqr(12))
	{
		mInfo["Title"] = "Dynamic Texturing";
		mInfo["Description"] = "Demonstrates how to create and use dynamically changing textures.";
		mInfo["Thumbnail"] = "thumb_dyntex.png";
		mInfo["Category"] = "Unsorted";
		mInfo["Help"] = "Use the left mouse button to wipe away the frost. "
			"It's cold though, so the frost will return after a while.";
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		// shoot a ray from the cursor to the plane
		Ray ray = mTrayMgr->getCursorRay(mCamera);
		mCursorQuery->setRay(ray);
		RaySceneQueryResult& result = mCursorQuery->execute();

		if (!result.empty())
		{
			// using the point of intersection, find the corresponding texel on our texture
			Vector3 pt = ray.getPoint(result.back().distance);
			mBrushPos = (Vector2(pt.x, -pt.y) / mPlaneSize + Vector2(0.5, 0.5)) * TEXTURE_SIZE;
		}

		uint8 freezeAmount = 0;
		mTimeSinceLastFreeze += evt.timeSinceLastFrame;

		// find out how much to freeze the plane based on time passed
		while (mTimeSinceLastFreeze >= 0.1)
		{
			mTimeSinceLastFreeze -= 0.1;
			freezeAmount += 0x04;
		}

		updateTexture(freezeAmount);  // rebuild texture contents

		mPenguinAnimState->addTime(evt.timeSinceLastFrame);  // increment penguin idle animation time
		mPenguinNode->yaw(Radian(evt.timeSinceLastFrame));   // spin the penguin around

		return SdkSample::frameRenderingQueued(evt);  // don't forget the parent class updates!
	}
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	bool touchPressed(const OIS::MultiTouchEvent& evt)
	{
		if (mTrayMgr->injectMouseDown(evt)) return true;
		mWiping = true;  // wipe frost if user left clicks in the scene
		return true;
	}

	bool touchReleased(const OIS::MultiTouchEvent& evt)
	{
		if (mTrayMgr->injectMouseUp(evt)) return true;
		mWiping = false;  // stop wiping frost if user releases LMB
		return true;
	}
#else
	bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
	{
		if (mTrayMgr->injectMouseDown(evt, id)) return true;
		mWiping = true;  // wipe frost if user left clicks in the scene
		return true;
	}
    
	bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
	{
		if (mTrayMgr->injectMouseUp(evt, id)) return true;
		mWiping = false;  // stop wiping frost if user releases LMB
		return true;
	}
#endif
protected:

	void setupContent()
	{
		mSceneMgr->setSkyBox(true, "Examples/StormySkyBox");  // add a skybox

		// setup some basic lighting for our scene
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
        mSceneMgr->createLight()->setPosition(20, 80, 50);

		// set initial camera position
		mCameraMan->setStyle(CS_MANUAL);
		mCamera->setPosition(0, 0, 200);

		mTrayMgr->showCursor();

		// create our dynamic texture with 8-bit luminance texels
		TexturePtr tex = TextureManager::getSingleton().createManual("thaw", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			TEX_TYPE_2D, TEXTURE_SIZE, TEXTURE_SIZE, 0, PF_L8, TU_DYNAMIC_WRITE_ONLY);

		mTexBuf = tex->getBuffer();  // save off the texture buffer

		// initialise the texture to have full luminance
		mTexBuf->lock(HardwareBuffer::HBL_DISCARD);
		memset(mTexBuf->getCurrentLock().data, 0xff, mTexBuf->getSizeInBytes());
		mTexBuf->unlock();

		// create a penguin and attach him to our penguin node
		Entity* penguin = mSceneMgr->createEntity("Penguin", "penguin.mesh");
		mPenguinNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mPenguinNode->attachObject(penguin);

		// get and enable the penguin idle animation
		mPenguinAnimState = penguin->getAnimationState("amuse");
		mPenguinAnimState->setEnabled(true);

		// create a snowstorm over the scene, and fast forward it a little
        ParticleSystem* ps = mSceneMgr->createParticleSystem("Snow", "Examples/Snow");
        mSceneMgr->getRootSceneNode()->attachObject(ps);
        ps->fastForward(30);

		// create a frosted screen in front of the camera, using our dynamic texture to "thaw" certain areas
		Entity* ent = mSceneMgr->createEntity("Plane", SceneManager::PT_PLANE);
		ent->setMaterialName("Examples/Frost");
		SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		node->setPosition(0, 0, 50);
		node->attachObject(ent);

		mPlaneSize = ent->getBoundingBox().getSize().x;   // remember the size of the plane

		mCursorQuery = mSceneMgr->createRayQuery(Ray());  // create a ray scene query for the cursor

		mTimeSinceLastFreeze = 0;
		mWiping = false;
	}

	void updateTexture(uint8 freezeAmount)
	{
		mTexBuf->lock(HardwareBuffer::HBL_NORMAL);

		// get access to raw texel data
		uint8* data = (uint8*)mTexBuf->getCurrentLock().data;

		uint8 temperature;
		Real sqrDistToBrush;

		// go through every texel...
		for (unsigned int y = 0; y < TEXTURE_SIZE; y++)
		{
			for (unsigned int x = 0; x < TEXTURE_SIZE; x++)
			{
				if (freezeAmount != 0)
				{
					// gradually refreeze anything that isn't completely frozen
					temperature = 0xff - *data;
					if (temperature > freezeAmount) *data += freezeAmount;
					else *data = 0xff;
				}

				if (mWiping)
				{
					// wipe frost from under the cursor
					sqrDistToBrush = Math::Sqr(x - mBrushPos.x) + Math::Sqr(y - mBrushPos.y);
					if (sqrDistToBrush <= SQR_BRUSH_RADIUS)
						*data = std::min<uint8>(sqrDistToBrush / SQR_BRUSH_RADIUS * 0xff, *data);
				}

				data++;
			}
		}

		mTexBuf->unlock();
	}

	void cleanupContent()
	{
		TextureManager::getSingleton().remove("thaw");
	}

	const unsigned int TEXTURE_SIZE;
	const unsigned int SQR_BRUSH_RADIUS;
	HardwarePixelBufferSharedPtr mTexBuf;
	Real mPlaneSize;
	RaySceneQuery* mCursorQuery;
	Vector2 mBrushPos;
	Real mTimeSinceLastFreeze;
	bool mWiping;
	SceneNode* mPenguinNode;
	AnimationState* mPenguinAnimState;
};

#endif
