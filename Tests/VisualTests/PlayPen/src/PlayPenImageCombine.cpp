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

PlayPen_ImageCombine::PlayPen_ImageCombine()
{
	mInfo["Title"] = "PlayPen_ImageCombine";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_ImageCombine::setupContent()
{
	Image combined;
	
	// pick 2 files that are the same size, alpha texture will be made greyscale
	combined.loadTwoImagesAsRGBA("rockwall.tga", "flare.png", 
	TRANSIENT_RESOURCE_GROUP, PF_BYTE_RGBA);
	
	TexturePtr tex = TextureManager::getSingleton().createManual("1", TRANSIENT_RESOURCE_GROUP, TEX_TYPE_2D, 256, 256, 1, 0, PF_BYTE_RGBA);
	tex->loadImage(combined);
	
	MaterialManager& mmgr = MaterialManager::getSingleton();
	MaterialPtr mat = mmgr.create("m1", TRANSIENT_RESOURCE_GROUP);
	Pass* pass = mat->getTechnique(0)->getPass(0);
	pass->setLightingEnabled(false);
	pass->setCullingMode(CULL_NONE);
	pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	pass->setDepthWriteEnabled(false);
	pass->createTextureUnitState(tex->getName());
	
	Entity *e = mSceneMgr->createEntity("test", SceneManager::PT_PLANE);
	e->setMaterialName(mat->getName());
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
	
	mCamera->setPosition(0, 0, 200);
	mCamera->lookAt(Vector3::ZERO);
	
	mWindow->getViewport(0)->setBackgroundColour(ColourValue::Blue);
	
}
