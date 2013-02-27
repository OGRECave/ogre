#ifndef __PNTrianglesTesselation_H__
#define __PNTrianglesTesselation_H__

#include "SdkSample.h"
#include "OgreImage.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_PNTrianglesTessellation : public SdkSample
{
public:

	Sample_PNTrianglesTessellation()
	{
		mInfo["Title"] = "PNTrianglesTesselation";
		mInfo["Description"] = "Sample for parametric PN-Triangles tessellation algorithm";
		mInfo["Thumbnail"] = "thumb_tesselation.png";
		mInfo["Category"] = "Unsorted";
		mInfo["Help"] = "Top Left: Multi-frame\nTop Right: Scrolling\nBottom Left: Rotation\nBottom Right: Scaling";
	}

	void testCapabilities(const RenderSystemCapabilities* caps)
	{
		if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
		{
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex and fragment"
				" programs, so you cannot run this sample. Sorry!", "Sample_PNTrianglesTessellation::testCapabilities");
		}
		if (!caps->hasCapability(RSC_TESSELATION_HULL_PROGRAM) || !caps->hasCapability(RSC_TESSELATION_DOMAIN_PROGRAM))
		{
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Your graphics card does not support tesselation shaders. Sorry!",
				"Sample_PNTrianglesTessellation:testCapabilities");
		}
		if (!GpuProgramManager::getSingleton().isSyntaxSupported("vs_5_0") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("hs_5_0") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("ds_5_0") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("ps_5_0") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
		{
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support the shader model 5.0 needed for this sample, "
				"so you cannot run this sample. Sorry!", "Sample_PNTrianglesTessellation::testCapabilities");
		}
	}

protected:

	void setupContent()
	{
		// set our camera
		mTrayMgr->showCursor();
		mCamera->setFOVy(Ogre::Degree(50.0));
		mCamera->setFOVy(Ogre::Degree(50.0));
		mCamera->setNearClipDistance(0.01f);
		mCamera->lookAt(Ogre::Vector3(0,0,0));
		mCamera->setPolygonMode(PM_WIREFRAME);

		mCameraMan->setStyle(OgreBites::CS_FREELOOK);
		mCameraMan->setTopSpeed(10);

		MaterialPtr lmaterialPtr1 = MaterialManager::getSingleton().createOrRetrieve(
			"Knot", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).first;
		lmaterialPtr1->compile();

		Pass *lpass1 = lmaterialPtr1->getBestTechnique()->getPass(0);
		Entity *tentity1 = mSceneMgr->createEntity("knot.mesh");

		unsigned short src, dest;
		if( tentity1->getMesh()->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest) )
			tentity1->getMesh()->buildTangentVectors(Ogre::VES_TANGENT, src, dest);

		tentity1->setMaterialName(lmaterialPtr1->getName());
		Ogre::SceneNode* tnode1 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		tnode1->setScale(0.5f, 0.5f, 0.5f);
		tnode1->setPosition(-100.0f, 0.0f, 0.0f);
		tnode1->attachObject(tentity1);

		MaterialPtr lmaterialPtr2 = MaterialManager::getSingleton().createOrRetrieve(
			"SimpleTessellation", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).first;

		lmaterialPtr2->compile();

		Pass *lpass2 = lmaterialPtr2->getBestTechnique()->getPass(0);
		Entity *tentity2 = mSceneMgr->createEntity("knot.mesh");

		if( tentity2->getMesh()->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest) )
			tentity2->getMesh()->buildTangentVectors(Ogre::VES_TANGENT, src, dest);

		tentity2->setMaterialName(lmaterialPtr2->getName());
		Ogre::SceneNode* tnode2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		tnode2->setScale(0.5f, 0.5f, 0.5f);
		tnode2->setPosition(100.0f, 0.0f, 0.0f);
		tnode2->attachObject(tentity2);

		Ogre::Light *lpointLight1 = mSceneMgr->createLight();
		lpointLight1->setType(Ogre::Light::LT_POINT);
		lpointLight1->setDiffuseColour(1.0, 1.0, 1.0);
		lpointLight1->setSpecularColour(1.0, 1.0, 1.0);
		lpointLight1->setPosition(Ogre::Vector3(-50.0f, 0.0f, -50.0f));

		Ogre::Light *lpointLight2 = mSceneMgr->createLight();
		lpointLight2->setType(Ogre::Light::LT_POINT);
		lpointLight2->setDiffuseColour(1.0, 1.0, 1.0);
		lpointLight2->setSpecularColour(1.0, 1.0, 1.0);
		lpointLight2->setPosition(Ogre::Vector3(50.0f, 0.0f, -50.0f));

		Ogre::SceneNode* tlightsNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("lightsNode");
		tlightsNode->attachObject(lpointLight1);
		tlightsNode->attachObject(lpointLight2);

		mTrayMgr->showCursor();

		// make room for the controls
		mTrayMgr->showLogo(TL_TOPRIGHT);
		mTrayMgr->showFrameStats(TL_TOPRIGHT);
		mTrayMgr->toggleAdvancedFrameStats();

		// create a menu to choose the model displayed
		mMeshMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "Mesh", "Mesh", 370, 290, 10);
		mMeshMenu->addItem("KNOT");

		// create a menu to choose the material used by the model
		mMaterialMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "Material", "Material", 370, 290, 10);

		// create checkboxes to toggle lights
		mTrayMgr->createCheckBox(TL_TOPLEFT, "Light1", "Light A")->setChecked(true, false);
		mTrayMgr->createCheckBox(TL_TOPLEFT, "Light2", "Light B")->setChecked(true, false);
		mTrayMgr->createCheckBox(TL_TOPLEFT, "MoveLights", "Move Lights")->setChecked(true, false);

		// a friendly reminder
		StringVector names;
		names.push_back("Help");
		mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");

		mMeshMenu->selectItem(0);  // select first mesh
	}

	SelectMenu* mMeshMenu;
	SelectMenu* mMaterialMenu;
};

#endif
