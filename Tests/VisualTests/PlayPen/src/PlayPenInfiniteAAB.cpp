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

PlayPen_InfiniteAAB::PlayPen_InfiniteAAB()
{
	mInfo["Title"] = "PlayPen_InfiniteAAB";
	mInfo["Description"] = "Tests finite/infinite AABBs.";
	addScreenshotFrame(15);
}
//----------------------------------------------------------------------------

void PlayPen_InfiniteAAB::setupContent()
{
	// When using the BspSceneManager
	mSceneMgr->setWorldGeometry("ogretestmap.bsp");
	
	// When using the TerrainSceneManager
	//mSceneMgr->setWorldGeometry("terrain.cfg");
	
	AxisAlignedBox b1; // null
	assert( b1.isNull() );
	
	AxisAlignedBox b2(Vector3::ZERO, 5.0 * Vector3::UNIT_SCALE); // finite
	assert( b2.isFinite() );
	
	AxisAlignedBox b3;
	b3.setInfinite();
	assert( b3.isInfinite() );
	
	{
		// Create background material
		MaterialPtr material = MaterialManager::getSingleton().create("Background", "General");
		material->getTechnique(0)->getPass(0)->createTextureUnitState("rockwall.tga");
		material->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
		material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
		material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		
		// Create left background rectangle
		// NOTE: Uses finite aab
		Rectangle2D* rect1 = new Rectangle2D(true);
		rect1->setCorners(-0.5, 0.1, -0.1, -0.1);
		// Hacky, set small bounding box, to show problem
		rect1->setBoundingBox(AxisAlignedBox(-10.0*Vector3::UNIT_SCALE, 10.0*Vector3::UNIT_SCALE));
		rect1->setMaterial("Background");
		rect1->setRenderQueueGroup(RENDER_QUEUE_OVERLAY - 1);
		SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode("Background1");
		node->attachObject(rect1);
		
		// Create right background rectangle
		// NOTE: Uses infinite aab
		Rectangle2D* rect2 = new Rectangle2D(true);
		rect2->setCorners(0.1, 0.1, 0.5, -0.1);
		AxisAlignedBox aabInf; aabInf.setInfinite();
		rect2->setBoundingBox(aabInf);
		rect2->setMaterial("Background");
		rect2->setRenderQueueGroup(RENDER_QUEUE_OVERLAY - 1);
		node = mSceneMgr->getRootSceneNode()->createChildSceneNode("Background2");
		node->attachObject(rect2);
		
		// Create a manual object for 2D
		ManualObject* manual = mSceneMgr->createManualObject("manual");
		manual->setUseIdentityProjection(true);
		manual->setUseIdentityView(true);
		manual->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_STRIP);
		manual->position(-0.2, -0.2, 0.0);
		manual->position( 0.2, -0.2, 0.0);
		manual->position( 0.2,  0.2, 0.0);
		manual->position(-0.2,  0.2, 0.0);
		manual->index(0);
		manual->index(1);
		manual->index(2);
		manual->index(3);
		manual->index(0);
		manual->end();
		manual->setBoundingBox(aabInf); // Use infinite aab to always stay visible
		rect2->setRenderQueueGroup(RENDER_QUEUE_OVERLAY - 1);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(manual);
	}
	
	mSceneMgr->showBoundingBoxes(true);
	
	Entity* ent = mSceneMgr->createEntity("test", "ogrehead.mesh");
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
	"test", 50.0 * Vector3::UNIT_X)->attachObject(ent);

	mCamera->setPosition(100,50,350);
	mCamera->lookAt(0,0,0);
}
