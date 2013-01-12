#ifndef __AtomicCounters_H__
#define __AtomicCounters_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_AtomicCounters : public SdkSample
{
public:

	Sample_AtomicCounters()
	{
		mInfo["Title"] = "Atomic Counters";
		mInfo["Description"] = "An example of using atomic counters to visualise GPU rasterization order";
		mInfo["Thumbnail"] = "thumb_camtrack.png";
		mInfo["Category"] = "Unsorted";
	}

    void testCapabilities(const RenderSystemCapabilities* caps)
	{
		if (!caps->hasCapability(RSC_ATOMIC_COUNTERS))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support atomic counters"
                        ", so you cannot run this sample. Sorry!", "Sample_AtomicCounters::testCapabilities");
        }
	}

protected:

	void setupContent()
	{
		// Setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
        mSceneMgr->createLight()->setPosition(20, 80, 50);
        
		mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox");

		// Create a dragon entity and attach it to a node
        Entity *ent = mSceneMgr->createEntity("Dragon", "dragon.mesh");
        SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        node->rotate(Vector3(0,1,0), Ogre::Angle(180));
		node->attachObject(ent);

		mCamera->setPosition(75, 0, 150);
		mCamera->lookAt(0, 0, 0);
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        setDragLook(true);
#endif

        HardwareCounterBufferSharedPtr cBuf = HardwareBufferManager::getSingleton().createCounterBuffer(sizeof(uint32), HardwareBuffer::HBU_WRITE_ONLY, false);

		mTrayMgr->showCursor();
    }
};

#endif
