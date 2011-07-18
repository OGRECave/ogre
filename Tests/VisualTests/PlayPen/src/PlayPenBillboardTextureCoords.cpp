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

PlayPen_BillboardTextureCoords::PlayPen_BillboardTextureCoords()
{
	mInfo["Title"] = "PlayPen_BillboardTextureCoords";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_BillboardTextureCoords::setupContent()
{
	mSceneMgr->setAmbientLight(ColourValue::White);
	
	BillboardSet* bbs = mSceneMgr->createBillboardSet("test");
	BillboardSet* bbs2 = mSceneMgr->createBillboardSet("test2");
	float xsegs = 3;
	float ysegs = 3;
	float width = 300;
	float height = 300;
	float gap = 20;
	
	// set up texture coords
	bbs->setTextureStacksAndSlices(ysegs, xsegs);
	bbs->setDefaultDimensions(width/xsegs, height/xsegs);
	bbs2->setDefaultDimensions(width/xsegs, height/xsegs);
	
	for (float y = 0; y < ysegs; ++y)
	{
		for (float x = 0; x < xsegs; ++x)
		{
			Vector3 midPoint;
			midPoint.x = (x * width / xsegs) + ((x-1) * gap);
			midPoint.y = (y * height / ysegs) + ((y-1) * gap);
			midPoint.z = 0;
			Billboard* bb = bbs->createBillboard(midPoint);
			bb->setTexcoordIndex((ysegs - y - 1)*xsegs + x);
			Billboard* bb2 = bbs2->createBillboard(midPoint);
			bb2->setTexcoordRect(
			FloatRect((x + 0) / xsegs, (ysegs - y - 1) / ysegs,
			(x + 1) / xsegs, (ysegs - y - 0) / ysegs));
		}
	}
	
	bbs->setMaterialName("Examples/OgreLogo");
	bbs2->setMaterialName("Examples/OgreLogo");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(bbs);
	mSceneMgr->getRootSceneNode()
	->createChildSceneNode(Vector3(- (width + xsegs * gap), 0, 0))
	->attachObject(bbs2);
	
	mCamera->setPosition(-100,150,900);
}
