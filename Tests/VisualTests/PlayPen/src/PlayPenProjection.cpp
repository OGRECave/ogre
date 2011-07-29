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

PlayPen_Projection::PlayPen_Projection()
{
	mInfo["Title"] = "PlayPen_Projection";
	mInfo["Description"] = "Tests projection.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_Projection::setupContent()
{
	SceneNode* mTestNode[3];
	
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	
	// Create a point light
	Light* l = mSceneMgr->createLight("MainLight");
	l->setType(Light::LT_DIRECTIONAL);
	l->setDirection(-Vector3::UNIT_Y);
	
	Entity* pEnt;
	//pEnt = mSceneMgr->createEntity( "1", "knot.mesh" );
	//mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-30,0,-50))->attachObject(pEnt);
	//pEnt->setMaterialName("Examples/OgreLogo");
	
	// Define a floor plane mesh
	Plane p;
	p.normal = Vector3::UNIT_Z;
	p.d = 200;
	MeshManager::getSingleton().createPlane("WallPlane",
	TRANSIENT_RESOURCE_GROUP,
	p,1500,1500,1,1,true,1,5,5,Vector3::UNIT_Y);
	pEnt = mSceneMgr->createEntity( "5", "WallPlane" );
	pEnt->setMaterialName("Examples/OgreLogo");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pEnt);
	
	
	mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	
	//pEnt = mSceneMgr->createEntity( "2", "ogrehead.mesh" );
	//mTestNode[0]->attachObject( pEnt );
	mTestNode[0]->translate(0, 0, 750);
	
	Ogre::Frustum* frustum = new Frustum();
	frustum->setVisible(true);
	frustum->setFarClipDistance(5000);
	frustum->setNearClipDistance(200);
	frustum->setAspectRatio(1);
	frustum->setProjectionType(PT_ORTHOGRAPHIC);
	mTestNode[0]->attachObject(frustum);
	
	// Hook the frustum up to the material
	MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/OgreLogo");
	TextureUnitState *t = mat->getTechnique(0)->getPass(0)->getTextureUnitState(0);
	t->setProjectiveTexturing(true, frustum);
	//t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
	
}
