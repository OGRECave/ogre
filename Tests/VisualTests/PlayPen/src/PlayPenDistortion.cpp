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

Entity* pPlaneEnt = 0;// yucky global...
Camera* theCam = 0;

class RefractionTextureListener : public RenderTargetListener
{
public:
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        pPlaneEnt->setVisible(false);

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        pPlaneEnt->setVisible(true);
    }

};

class ReflectionTextureListener : public RenderTargetListener
{
public:
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        static Plane reflectPlane(Vector3::UNIT_Y, -100);
        pPlaneEnt->setVisible(false);
        theCam->enableReflection(reflectPlane);

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        pPlaneEnt->setVisible(true);
        theCam->disableReflection();
    }

};

PlayPen_Distortion::PlayPen_Distortion()
{
	mInfo["Title"] = "PlayPen_Distortion";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(15);

	mRefractionListener = new RefractionTextureListener();
	mReflectionListener = new ReflectionTextureListener();
}
//----------------------------------------------------------------------------

PlayPen_Distortion::~PlayPen_Distortion()
{
	delete mRefractionListener;
	delete mReflectionListener;
}
//----------------------------------------------------------------------------

void PlayPen_Distortion::cleanupContent()
{
	TexturePtr rttTex = TextureManager::getSingleton().getByName("Refraction");
	rttTex->getBuffer()->getRenderTarget()->removeAllListeners();
	TextureManager::getSingleton().unload(rttTex->getHandle());
	rttTex = TextureManager::getSingleton().getByName("Reflection");
	rttTex->getBuffer()->getRenderTarget()->removeAllListeners();
	TextureManager::getSingleton().unload(rttTex->getHandle());
}
//----------------------------------------------------------------------------

void PlayPen_Distortion::setupContent()
{
	SceneNode* mTestNode[5];
	theCam = mCamera;
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	
	// Create a point light
	Light* l = mSceneMgr->createLight("MainLight");
	l->setType(Light::LT_DIRECTIONAL);
	l->setDirection(-Vector3::UNIT_Y);
	
	Entity* pEnt;
	
	TexturePtr rttTex = TextureManager::getSingleton().createManual("Refraction", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
	512, 512, 1, 0, PF_R8G8B8, TU_RENDERTARGET);
	{
		Viewport *v = rttTex->getBuffer()->getRenderTarget()->addViewport( mCamera );
		MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
		mat->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName("Refraction");
		v->setOverlaysEnabled(false);
		rttTex->getBuffer()->getRenderTarget()->addListener(mRefractionListener);
	}
	
	rttTex = TextureManager::getSingleton().createManual("Reflection", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
	512, 512, 1, 0, PF_R8G8B8, TU_RENDERTARGET);
	{
		Viewport *v = rttTex->getBuffer()->getRenderTarget()->addViewport( mCamera );
		MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
		mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName("Reflection");
		v->setOverlaysEnabled(false);
		rttTex->getBuffer()->getRenderTarget()->addListener(mReflectionListener);
	}
	// Define a floor plane mesh
	Plane p;
	p.normal = Vector3::UNIT_Y;
	p.d = 100;
	MeshManager::getSingleton().createPlane("WallPlane",
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
	p,1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
	pPlaneEnt = mSceneMgr->createEntity( "5", "WallPlane" );
	pPlaneEnt->setMaterialName("Examples/FresnelReflectionRefraction");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	
	mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");
	
	mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	int i;
	for (i = 0; i < 10; ++i)
	{
		pEnt = mSceneMgr->createEntity( "ogre" + StringConverter::toString(i), "ogrehead.mesh" );
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(i*100 - 500, -75, 0))->attachObject(pEnt);
		pEnt = mSceneMgr->createEntity( "knot" + StringConverter::toString(i), "knot.mesh" );
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(i*100 - 500, 140, 0))->attachObject(pEnt);
	}
	
	mCamera->setPosition(100,200,300);
	mCamera->lookAt(0,0,0);
	
}
