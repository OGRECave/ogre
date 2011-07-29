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

PlayPen_MaterialSchemesListener::PlayPen_MaterialSchemesListener()
{
	mInfo["Title"] = "PlayPen_MaterialSchemesListener";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

class TestMatMgrListener : public MaterialManager::Listener
{
public:
	TestMatMgrListener() : mTech(0) {}
	Technique* mTech;
	

	Technique* handleSchemeNotFound(unsigned short schemeIndex, 
		const String& schemeName, Material* originalMaterial, unsigned short lodIndex, 
		const Renderable* rend)
	{
		return mTech;
	}
};
TestMatMgrListener schemeListener;

void PlayPen_MaterialSchemesListener::setupContent()
{
	Entity *ent = mSceneMgr->createEntity("robot", "robot.mesh");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	mSceneMgr->setAmbientLight(ColourValue(0.8, 0.8, 0.8));
	
	// create a second viewport using alternate scheme
	// notice it's not defined in a technique
	Viewport* vp = mWindow->addViewport(mCamera, 1, 0.75, 0, 0.25, 0.25);
	vp->setMaterialScheme("newscheme");
	vp->setOverlaysEnabled(false);
	
	MaterialPtr mat = MaterialManager::getSingleton().create("schemetest", 
	TRANSIENT_RESOURCE_GROUP);
	// default scheme
	mat->getTechnique(0)->getPass(0)->createTextureUnitState("GreenSkin.jpg");
	
	schemeListener.mTech = mat->getTechnique(0);
	
	MaterialManager::getSingleton().addListener(&schemeListener);

	mCamera->setPosition(0,75,200);
}
