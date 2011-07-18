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

PlayPen_BlendDiffuseColour::PlayPen_BlendDiffuseColour()
{
	mInfo["Title"] = "PlayPen_BlendDiffuseColour";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_BlendDiffuseColour::setupContent()
{
	MaterialPtr mat = MaterialManager::getSingleton().create(
	"testBlendDiffuseColour", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Pass* pass = mat->getTechnique(0)->getPass(0);
	// no lighting, it will mess up vertex colours
	pass->setLightingEnabled(false);
	// Make sure we pull in vertex colour as diffuse
	pass->setVertexColourTracking(TVC_DIFFUSE);
	// Base layer
	TextureUnitState* t = pass->createTextureUnitState("BeachStones.jpg");
	// don't want to bring in vertex diffuse on base layer
	t->setColourOperation(LBO_REPLACE); 
	// Second layer (lerp based on colour)
	t = pass->createTextureUnitState("terr_dirt-grass.jpg");
	t->setColourOperationEx(LBX_BLEND_DIFFUSE_COLOUR);
	// third layer (lerp based on alpha)
	ManualObject* man = mSceneMgr->createManualObject("quad");
	man->begin("testBlendDiffuseColour");
	man->position(-100, 100, 0);
	man->textureCoord(0,0);
	man->colour(0, 0, 0);
	man->position(-100, -100, 0);
	man->textureCoord(0,1);
	man->colour(0.5, 0.5, 0.5);
	man->position(100, -100, 0);
	man->textureCoord(1,1);
	man->colour(1, 1, 1);
	man->position(100, 100, 0);
	man->textureCoord(1,0);
	man->colour(0.5, 0.5, 0.5);
	man->quad(0, 1, 2, 3);
	man->end();
	
	mSceneMgr->getRootSceneNode()->attachObject(man);

	mCamera->setPosition(0,0,250);
	
}
