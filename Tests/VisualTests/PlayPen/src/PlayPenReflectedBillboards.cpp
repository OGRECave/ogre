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

PlayPen_ReflectedBillboards::PlayPen_ReflectedBillboards()
{
	mInfo["Title"] = "PlayPen_ReflectedBillboards";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(400);
}
//----------------------------------------------------------------------------

void PlayPen_ReflectedBillboards::setupContent()
{
	Camera* reflectCam = 0;
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
	
	// Create a light
	Light* l = mSceneMgr->createLight("MainLight");
	l->setType(Light::LT_DIRECTIONAL);
	Vector3 dir(0.5, -1, 0);
	dir.normalise();
	l->setDirection(dir);
	l->setDiffuseColour(1.0f, 1.0f, 0.8f);
	l->setSpecularColour(1.0f, 1.0f, 1.0f);
	
	
	// Create a prefab plane
	Plane plane;
	plane.d = 0;
	plane.normal = Vector3::UNIT_Y;
	MeshManager::getSingleton().createPlane("ReflectionPlane", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
	plane, 2000, 2000, 
	1, 1, true, 1, 1, 1, Vector3::UNIT_Z);
	Entity* planeEnt = mSceneMgr->createEntity( "Plane", "ReflectionPlane" );
	
	// Attach the rtt entity to the root of the scene
	SceneNode* rootNode = mSceneMgr->getRootSceneNode();
	SceneNode* planeNode = rootNode->createChildSceneNode();
	
	// Attach both the plane entity, and the plane definition
	planeNode->attachObject(planeEnt);

	mCamera->setPosition(-50, 100, 500);
	mCamera->lookAt(0,0,0);
	
	TexturePtr rttTex = TextureManager::getSingleton().createManual("RttTex", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
	512, 512, 1, 0, PF_R8G8B8, TU_RENDERTARGET);
	{
		reflectCam = mSceneMgr->createCamera("ReflectCam");
		reflectCam->setNearClipDistance(mCamera->getNearClipDistance());
		reflectCam->setFarClipDistance(mCamera->getFarClipDistance());
		reflectCam->setAspectRatio(
		(Real)mWindow->getViewport(0)->getActualWidth() / 
		(Real)mWindow->getViewport(0)->getActualHeight());

		reflectCam->setPosition(mCamera->getPosition());
		reflectCam->setOrientation(mCamera->getOrientation());
		
		Viewport *v = rttTex->getBuffer()->getRenderTarget()->addViewport( reflectCam );
		v->setClearEveryFrame( true );
		v->setBackgroundColour( ColourValue::Black );
		
		MaterialPtr mat = MaterialManager::getSingleton().create("RttMat",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		TextureUnitState* t = mat->getTechnique(0)->getPass(0)->createTextureUnitState("RustedMetal.jpg");
		t = mat->getTechnique(0)->getPass(0)->createTextureUnitState("RttTex");
		// Blend with base texture
		t->setColourOperationEx(LBX_BLEND_MANUAL, LBS_TEXTURE, LBS_CURRENT, ColourValue::White, 
		ColourValue::White, 0.25);
		t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
		t->setProjectiveTexturing(true, reflectCam);
		
		// set up linked reflection
		reflectCam->enableReflection(plane);
		// Also clip
		reflectCam->enableCustomNearClipPlane(plane);
	}
	
	// Give the plane a texture
	planeEnt->setMaterialName("RttMat");
	
	
	// point billboards
	ParticleSystem* pSys2 = mSceneMgr->createParticleSystem("fountain1", 
	"Examples/Smoke");
	// Point the fountain at an angle
	SceneNode* fNode = static_cast<SceneNode*>(rootNode->createChild());
	fNode->attachObject(pSys2);
	
	// oriented_self billboards
	ParticleSystem* pSys3 = mSceneMgr->createParticleSystem("fountain2", 
	"Examples/PurpleFountain");
	// Point the fountain at an angle
	fNode = rootNode->createChildSceneNode();
	fNode->translate(-200,-100,0);
	fNode->rotate(Vector3::UNIT_Z, Degree(-20));
	fNode->attachObject(pSys3);
	
	
	
	// oriented_common billboards
	ParticleSystem* pSys4 = mSceneMgr->createParticleSystem("rain", 
	"Examples/Rain");
	SceneNode* rNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	rNode->translate(0,1000,0);
	rNode->attachObject(pSys4);
	// Fast-forward the rain so it looks more natural
	pSys4->fastForward(5);
	
	
}
