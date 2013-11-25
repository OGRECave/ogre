#ifndef __Tessellation_H__
#define __Tessellation_H__

#include "SdkSample.h"
#include "OgreImage.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Tessellation : public SdkSample
{
public:

	Sample_Tessellation()
	{
		mInfo["Title"] = "Tessellation";
		mInfo["Description"] = "Sample for tessellation support (Hull, Domain shaders)";
		mInfo["Thumbnail"] = "thumb_tessellation.png";
		mInfo["Category"] = "Unsorted";
		mInfo["Help"] = "Top Left: Multi-frame\nTop Right: Scrolling\nBottom Left: Rotation\nBottom Right: Scaling";
	}

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex and fragment"
                        " programs, so you cannot run this sample. Sorry!", "Sample_Tessellation::testCapabilities");
        }
		if (!caps->hasCapability(RSC_TESSELLATION_HULL_PROGRAM) || !caps->hasCapability(RSC_TESSELLATION_DOMAIN_PROGRAM))
		{
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Your graphics card does not support tessellation shaders. Sorry!",
				"Sample_Tessellation:testCapabilities");
		}
		if (!GpuProgramManager::getSingleton().isSyntaxSupported("vs_5_0") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("hs_5_0") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("ds_5_0") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("ps_5_0") &&
   			!GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
		{
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support the shader model 5.0 needed for this sample, "
						"so you cannot run this sample. Sorry!", "Sample_Tessellation::testCapabilities");
		}
    }

protected:

	void setupContent()
	{
		// set our camera
		mTrayMgr->showCursor();
		mCameraMan->setStyle(CS_ORBIT);
		Camera* cam = mCameraMan->getCamera();
		cam->setPosition(0, 5, -30);
		cam->setPolygonMode(PM_WIREFRAME);

		// create material and set the texture unit to our texture
        MaterialPtr tMat = MaterialManager::getSingleton().createOrRetrieve("Ogre/TessellationExample", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).first.staticCast<Material>();
        tMat->compile();
        tMat->getBestTechnique()->getPass(0);

		// create a plain with float3 tex cord
	    ManualObject* tObject = mSceneMgr->createManualObject("TesselatedObject");
        
        // create a triangle that uses our material 
	    tObject->begin(tMat->getName(), RenderOperation::OT_TRIANGLE_LIST);
	    tObject->position(0, 0, 0);
	    tObject->position(0, 10, 0);
	    tObject->position(10, 10, 0);
	    tObject->end();

		// attach it to a node and position appropriately
		SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		node->attachObject(tObject);

		printf("Cam position: %.2f, %.2f, %.2f", cam->getPosition().x, cam->getPosition().y, cam->getPosition().z);
	}
};

#endif
