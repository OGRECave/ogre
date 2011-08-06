/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "PlayPenTests.h"

PlayPen_TextureShadowsTransparentCaster::PlayPen_TextureShadowsTransparentCaster()
{
	mInfo["Title"] = "PlayPen_TextureShadowsTransparentCaster";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_TextureShadowsTransparentCaster::setupContent()
{
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
	
	
	LiSPSMShadowCameraSetup *mLiSPSMSetup = new LiSPSMShadowCameraSetup();
	//mLiSPSMSetup->setUseAggressiveFocusRegion(false);
	ShadowCameraSetupPtr mCurrentShadowCameraSetup = ShadowCameraSetupPtr(mLiSPSMSetup);
	//ShadowCameraSetupPtr mCurrentShadowCameraSetup = ShadowCameraSetupPtr(new PlaneOptimalShadowCameraSetup(mPlane));					
	mSceneMgr->setShadowCameraSetup(mCurrentShadowCameraSetup);
	
	PixelFormat pxFmt = PF_L8;
	if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_TEXTURE_FLOAT))
	{
		if (Root::getSingleton().getRenderSystem()->getName().find("GL") != String::npos)
		{
			// GL performs much better if you pick half-float format
			pxFmt = PF_FLOAT16_R;
		}
		else
		{
			// D3D is the opposite - if you ask for PF_FLOAT16_R you
			// get an integer format instead! You can ask for PF_FLOAT16_GR
			// but the precision doesn't work well
			pxFmt = PF_FLOAT32_R;
		}
	}
	mSceneMgr->setShadowTextureSettings(1024, 1, pxFmt);
	
	// New depth shadow mapping
	String CUSTOM_ROCKWALL_MATERIAL("Ogre/DepthShadowmap/Receiver/RockWall");
	String CUSTOM_CASTER_MATERIAL("Ogre/DepthShadowmap/Caster/Float");
	String CUSTOM_RECEIVER_MATERIAL("Ogre/DepthShadowmap/Receiver/Float");
	
	mSceneMgr->setShadowTextureCasterMaterial(CUSTOM_CASTER_MATERIAL);
	mSceneMgr->setShadowTextureReceiverMaterial(CUSTOM_RECEIVER_MATERIAL);
	mSceneMgr->setShadowTextureSelfShadow(true);
	
	mSceneMgr->setShadowTextureFadeStart(1.0);
	mSceneMgr->setShadowTextureFadeEnd(1.0);
	
	mSceneMgr->setShadowTextureSelfShadow(true);
	
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(5);
	
	mSceneMgr->setShadowDirLightTextureOffset(0.2);
	mSceneMgr->setShadowFarDistance(150);
	//mSceneMgr->setShadowCasterRenderBackFaces(false);
	
	// Create a basic plane to have something in the scene to look at
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshPtr msh = MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, plane,
	4500,4500,100,100,true,1,40,40,Vector3::UNIT_Z);
	msh->buildTangentVectors(VES_TANGENT);
	Entity* pPlaneEnt;
	pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	
	pPlaneEnt->setMaterialName(CUSTOM_ROCKWALL_MATERIAL);
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	
	
	// Reorient the plane and create a plane mesh for the test planes
	plane.normal = Vector3::UNIT_Z;
	MeshManager::getSingleton().createPlane(
	"Test_Plane", TRANSIENT_RESOURCE_GROUP, 
	plane, 50.0, 50.0, 1, 1, true);
	
	
	const String GRASSMAT("Examples/GrassBladesAdditiveFloatTransparent");
	//const String GRASSMAT("Examples/DepthShadowmap/CasterReceiver/GrassBlades");
	//const String GRASSMAT("tree4324");//"tree1.tga");
	
	
	// Add test plane entities to the scene
	Entity* entity = mSceneMgr->createEntity("GrassBlades0", "Test_Plane");
	entity->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(0.0, -100.0+25.0, 0.0))->attachObject(entity);
	
	
	entity = mSceneMgr->createEntity("GrassBlades1", "Test_Plane");
	entity->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(0.0, -100.0+25.0, -20.0))->attachObject(entity);
	
	entity = mSceneMgr->createEntity("GrassBlades2", "Test_Plane");
	entity->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(0.0, -100.0+25.0, -40.0))->attachObject(entity);
	
	// Add test plane entities to the scene, shadowed partially by athene mesh
	entity = mSceneMgr->createEntity("GrassBlades3", "Test_Plane");
	entity->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(-80.0, -100.0+25.0, 0.0))->attachObject(entity);
	
	entity = mSceneMgr->createEntity("GrassBlades4", "Test_Plane");
	entity->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(-130.0, -100.0+25.0, -20.0))->attachObject(entity);
	
	entity = mSceneMgr->createEntity("GrassBlades5", "Test_Plane");
	entity->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(-180.0, -100.0+25.0, -40.0))->attachObject(entity);
	
	
	
	Entity* ent = mSceneMgr->createEntity("athene", "athene.mesh");
	ent->setMaterialName(CUSTOM_ROCKWALL_MATERIAL);
	//ent->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,-20,-20))->attachObject(ent);
	
	// Add test plane entities to the scene, one after another
	entity = mSceneMgr->createEntity("GrassBlades6", "Test_Plane");
	entity->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(-260.0, -100.0+25.0, 0.0))->attachObject(entity);
	
	entity = mSceneMgr->createEntity("GrassBlades7", "Test_Plane");
	entity->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(-260.0, -100.0+25.0, -10.0))->attachObject(entity);
	
	entity = mSceneMgr->createEntity("GrassBlades8", "Test_Plane");
	entity->setMaterialName(GRASSMAT);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(-260.0, -100.0+25.0, -20.0))->attachObject(entity);
	
	// Add test plane entities to the scene, alone with other material
	
	const String GRASSMAT_CUSTOM_DEFAULT_CUSTOM("Examples/GrassBladesAdditiveFloat");
	const String GRASSMAT_CUSTOM_NOSPECIAL_CUSTOM("Examples/GrassBladesAdditive");		
	const String GRASSMAT_ORIG("Examples/GrassBlades");
	
	entity = mSceneMgr->createEntity("GrassBlades9", "Test_Plane");
	entity->setMaterialName(GRASSMAT_CUSTOM_DEFAULT_CUSTOM);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(-80.0, -100.0+25.0, -80.0))->attachObject(entity);
	
	entity = mSceneMgr->createEntity("GrassBlades10", "Test_Plane");
	entity->setMaterialName(GRASSMAT_CUSTOM_NOSPECIAL_CUSTOM);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(-130.0, -100.0+25.0, -90.0))->attachObject(entity);
	
	entity = mSceneMgr->createEntity("GrassBlades11", "Test_Plane");
	entity->setMaterialName(GRASSMAT_ORIG);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	Vector3(-180.0, -100.0+25.0, -90.0))->attachObject(entity);
	
	// Position and orient the camera
	//mCamera->setPosition(-55.0, 40.0, 100.0);
	//mCamera->lookAt(-10.0, 20.0, -35.0);
	//mCamera->setPosition(-75.0, 30.0, 150.0);
	//mCamera->lookAt(0.0, 20.0, -35.0);
	mCamera->setPosition(100,50,150);
	mCamera->lookAt(0,0,0);
	
	//mSceneMgr->setAmbientLight(ColourValue::Black);
	Light* l;
	
	l = mSceneMgr->createLight("Dir1");
	l->setType(Light::LT_DIRECTIONAL);
	//l->setAttenuation(5000,1,0,0);
	Vector3 dir1(0.0, -0.7, -0.5);
	dir1.normalise();
	l->setDirection(dir1);
	l->setCastShadows(true);
	
	l->setDiffuseColour(ColourValue(1.0, 1.0, 1.0));
	
	
	mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.2));
	
	// 		l = mSceneMgr->createLight("Spot2");
	// 		l->setAttenuation(5000,1,0,0);
	// 		/* // spotlight */
	// 		l->setType(Light::LT_SPOTLIGHT);
	// 		l->setSpotlightRange(Degree(30),Degree(45),1.0f);
	// 		
	// 		
	// 		SceneNode* lightNode2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	// 		lightNode2->attachObject(l);
	// 		lightNode2->setPosition(-500, 200, 500);
	// 		lightNode2->lookAt(Vector3(0,-200,0), Node::TS_WORLD);
	// 		l->setDirection(Vector3::NEGATIVE_UNIT_Z);
	//lightNode2->setPosition(-75.0, 30.0, 150.0);
	//lightNode2->lookAt(Vector3(.0, 20.0, -35.0), Node::TS_WORLD);
	
	
	//addTextureShadowDebugOverlay(1, mSceneMgr);
	
	// not completely necessary, and can't guarantee determinism easily
	//Root::getSingleton().addFrameListener(new GrassListener(mSceneMgr));
	
}
