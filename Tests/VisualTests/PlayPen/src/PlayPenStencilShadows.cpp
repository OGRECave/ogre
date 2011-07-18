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

PlayPen_StencilShadows::PlayPen_StencilShadows()
{
	mInfo["Title"] = "PlayPen_StencilShadows";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_StencilShadows::setupContent()
{
	SceneNode* mTestNode[10];
	Light* mLight = 0;

	mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
	//mSceneMgr->setShowDebugShadows(true);
	mSceneMgr->setShadowDirectionalLightExtrusionDistance(1000);
	//mSceneMgr->setShadowColour(ColourValue(0.4, 0.25, 0.25));
	
	//mSceneMgr->setShadowFarDistance(800);
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));
	
	// Point light
	//if(pointLight)
	//{
		mLight = mSceneMgr->createLight("MainLight");
		mLight->setPosition(-400,400,-300);
		mLight->setDiffuseColour(0.9, 0.9, 1);
		mLight->setSpecularColour(0.9, 0.9, 1);
		mLight->setAttenuation(6000,1,0.001,0);
	//}
	// Directional light
	//if (directionalLight)
	//{
		mLight = mSceneMgr->createLight("Light2");
		Vector3 dir(-1,-1,0);
		dir.normalise();
		mLight->setType(Light::LT_DIRECTIONAL);
		mLight->setDirection(dir);
		mLight->setDiffuseColour(1, 1, 0.8);
		mLight->setSpecularColour(1, 1, 1);
	//}
	
	mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	
	// Hardware skin
	Entity* pEnt;
	pEnt = mSceneMgr->createEntity( "1", "robot.mesh" );
	AnimationState* anim = pEnt->getAnimationState("Walk");
	anim->setEnabled(true);
	mAnimStateList.push_back(anim);
	mTestNode[0]->attachObject( pEnt );
	
	// Software skin
	pEnt = mSceneMgr->createEntity( "12", "robot.mesh" );
	anim = pEnt->getAnimationState("Walk");
	anim->setEnabled(true);
	mAnimStateList.push_back(anim);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 0))->attachObject(pEnt);
	pEnt->setMaterialName("Examples/Rocky");
	
	// test object
	//pEnt = mSceneMgr->createEntity("tst", "building.mesh");
	//mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(300, 0, 0))->attachObject(pEnt);
	
	
	// Does not receive shadows
	pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
	pEnt->setMaterialName("Examples/EnvMappedRustySteel");
	MaterialPtr mat2 = MaterialManager::getSingleton().getByName("Examples/SphereMappedRustySteel");
	mat2->setReceiveShadows(false);
	mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
	mTestNode[2]->attachObject( pEnt );
	
	// Transparent object 
	pEnt = mSceneMgr->createEntity( "3.5", "knot.mesh" );
	pEnt->setMaterialName("Examples/TransparentTest");
	MaterialPtr mat3 = MaterialManager::getSingleton().getByName("Examples/TransparentTest");
	pEnt->setCastShadows(false);
	mTestNode[3] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(350, 0, -200));
	mTestNode[3]->attachObject( pEnt );
	
	// User test
	/*
	pEnt = mSceneMgr->createEntity( "3.6", "ogre_male_endCaps.mesh" );
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 100))->attachObject( pEnt );
	*/
	
	MeshPtr msh = MeshManager::getSingleton().load("knot.mesh", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	unsigned short src, dest;
	if (!msh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
	{
		msh->buildTangentVectors(VES_TANGENT, src, dest);
	}
	pEnt = mSceneMgr->createEntity( "4", "knot.mesh" );
	pEnt->setMaterialName("Examples/BumpMapping/MultiLightSpecular");
	mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 200));
	mTestNode[2]->attachObject( pEnt );
	
	// controller based material
	pEnt = mSceneMgr->createEntity( "432", "knot.mesh" );
	pEnt->setMaterialName("Examples/TextureEffect2");
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 200, 200))->attachObject( pEnt );
	
	ParticleSystem* pSys2 = mSceneMgr->createParticleSystem("smoke", 
	"Examples/Smoke");
	mTestNode[4] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-300, -100, 200));
	mTestNode[4]->attachObject(pSys2);
	
	
	mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");
	
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshManager::getSingleton().createPlane("Myplane",
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
	1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("2 - Default");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	mCamera->setPosition(180, 34, 223);
	mCamera->setOrientation(Quaternion(0.7265, -0.2064, 0.6304, 0.1791));
	
	// Create a render texture
/*	TexturePtr rtt = TextureManager::getSingleton().createManual("rtt0", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
	TEX_TYPE_2D, 512, 512, 0, PF_R8G8B8, TU_RENDERTARGET);
	rtt->getBuffer()->getRenderTarget()->addViewport(mCamera);
	// Create an overlay showing the rtt
	MaterialPtr debugMat = MaterialManager::getSingleton().create(
	"DebugRTT", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState("rtt0");
	t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
	OverlayContainer* debugPanel = (OverlayContainer*)
	(OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugShadowPanel"));
	debugPanel->_setPosition(0.6, 0);
	debugPanel->_setDimensions(0.4, 0.6);
	debugPanel->setMaterialName("DebugRTT");
	Overlay* debugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
	debugOverlay->add2D(debugPanel);*/
	
	
}
