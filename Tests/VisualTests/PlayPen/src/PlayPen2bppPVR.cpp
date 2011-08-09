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
#include "OgreCodec.h"

PlayPen_2bppPVR::PlayPen_2bppPVR()
{
	mInfo["Title"] = "PlayPen_2bppPVR";
	mInfo["Description"] = "Tests 2 bpp pvr.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_2bppPVR::setupContent()
{
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Tests");
	
	MaterialPtr mat = MaterialManager::getSingleton().create("testpvr", 
	TRANSIENT_RESOURCE_GROUP);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setLightingEnabled(false);
	p->setCullingMode(CULL_NONE);
	p->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	TextureUnitState* t = p->createTextureUnitState("ogreborderUp_pvr2.pvr");
	Entity *e = mSceneMgr->createEntity("Plane", SceneManager::PT_PLANE);
	e->setMaterialName(mat->getName());
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
	mWindow->getViewport(0)->setBackgroundColour(ColourValue::Red);
	
	mCamera->setPosition(0,0,300);
	mCamera->lookAt(Vector3::ZERO);
}
//----------------------------------------------------------------------------

void PlayPen_2bppPVR::testCapabilities(const Ogre::RenderSystemCapabilities* caps)
{
	Codec* codec = Codec::getCodec("pvr");
	if (!codec)
		throw Ogre::Exception(999, "No support for PVR textures.", "testCapabilities");
}
//----------------------------------------------------------------------------
