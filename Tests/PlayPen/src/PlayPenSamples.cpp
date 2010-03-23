/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2010 Torus Knot Software Ltd

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
#include "PlayPenSamples.h"


//---------------------------------------------------------------------
PlayPen_testManualBlend::PlayPen_testManualBlend()
{
	mInfo["Title"] = "PlayPen: Manual Blend";
	mInfo["Description"] = "Manual blending";

}
void PlayPen_testManualBlend::setupContent()
{
	// create material
	MaterialPtr mat = MaterialManager::getSingleton().create("TestMat", 
		TRANSIENT_RESOURCE_GROUP);
	Pass * p = mat->getTechnique(0)->getPass(0);
	p->setLightingEnabled(false);
	p->createTextureUnitState("Dirt.jpg");
	TextureUnitState* t = p->createTextureUnitState("ogrelogo.png");
	t->setColourOperationEx(LBX_BLEND_MANUAL, LBS_TEXTURE, LBS_CURRENT, 
		ColourValue::White, ColourValue::White, 0.75);

	Entity *planeEnt = mSceneMgr->createEntity("Plane", SceneManager::PT_PLANE);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(planeEnt);
	planeEnt->setMaterialName("TestMat");

	mCamera->setPosition(0,0,600);
	mCamera->lookAt(Vector3::ZERO);
}
//---------------------------------------------------------------------
PlayPen_testProjectSphere::PlayPen_testProjectSphere()
{
	mInfo["Title"] = "PlayPen: Project Sphere";
	mInfo["Description"] = "Projecting a sphere's bounds onto the camera";

}
void PlayPen_testProjectSphere::setupContent()
{
	mSceneMgr->setAmbientLight(ColourValue::White);


	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 0;
	MeshManager::getSingleton().createPlane("Myplane",
		TRANSIENT_RESOURCE_GROUP, plane,
		4500,4500,10,10,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("Examples/GrassFloor");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

	mProjectionSphere = new Sphere(Vector3(0, 2000, 0), 1500.0);

	ManualObject* debugSphere = mSceneMgr->createManualObject("debugSphere");
	debugSphere->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_STRIP);
	for (int i = 0; i <= 20; ++i)
	{
		Vector3 basePos(mProjectionSphere->getRadius(), 0, 0);
		Quaternion quat;
		quat.FromAngleAxis(Radian(((float)i/(float)20)*Math::TWO_PI), Vector3::UNIT_Y);
		basePos = quat * basePos;
		debugSphere->position(basePos);
	}
	for (int i = 0; i <= 20; ++i)
	{
		Vector3 basePos(mProjectionSphere->getRadius(), 0, 0);
		Quaternion quat;
		quat.FromAngleAxis(Radian(((float)i/(float)20)*Math::TWO_PI), Vector3::UNIT_Z);
		basePos = quat * basePos;
		debugSphere->position(basePos);
	}
	debugSphere->end();

	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,2000,0))->attachObject(debugSphere);

	MaterialPtr mat = MaterialManager::getSingleton().create("scissormat", 
		TRANSIENT_RESOURCE_GROUP);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setDepthWriteEnabled(false);
	p->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	TextureUnitState* t = p->createTextureUnitState();
	t->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 
		ColourValue::Red);
	t->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 0.5f);


	mScissorRect = mSceneMgr->createManualObject("mScissorRect");
	mScissorRect->setUseIdentityProjection(true);
	mScissorRect->setUseIdentityView(true);
	AxisAlignedBox aabb;
	aabb.setInfinite();
	mScissorRect->setBoundingBox(aabb);
	mScissorRect->begin(mat->getName());
	mScissorRect->position(Vector3::ZERO);
	mScissorRect->position(Vector3::ZERO);
	mScissorRect->position(Vector3::ZERO);
	mScissorRect->quad(0, 1, 2, 3);
	mScissorRect->end();
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mScissorRect);

	mCamera->setPosition(0,3000,5000);
	mCamera->lookAt(mProjectionSphere->getCenter());


}
bool PlayPen_testProjectSphere::frameStarted(const Ogre::FrameEvent& evt)
{
	Real left, top, right, bottom;
	mCamera->projectSphere(*mProjectionSphere, &left, &top, &right, &bottom);

	mScissorRect->beginUpdate(0);
	mScissorRect->position(left, top, 0);
	mScissorRect->position(left, bottom, 0);
	mScissorRect->position(right, bottom, 0);
	mScissorRect->position(right, top, 0);
	mScissorRect->quad(0,1,2,3);
	mScissorRect->end();

	return PlayPenBase::frameStarted(evt);

}

//---------------------------------------------------------------------
PlayPen_testCameraSetDirection::PlayPen_testCameraSetDirection()
: mUseParentNode(false)
, mUseFixedYaw(true)
, mFocus(100,200,-300)
{
	mInfo["Title"] = "PlayPen: Camera Set Direction";
	mInfo["Description"] = "Testing various settings for Camera::setDirection";

}
void PlayPen_testCameraSetDirection::setupContent()
{
	mSceneMgr->setAmbientLight(ColourValue::White);

	Entity* e = mSceneMgr->createEntity("1", "knot.mesh");
	mSceneMgr->getRootSceneNode()->createChildSceneNode(mFocus)->attachObject(e);


	mCamera->setPosition(200,1000,1000);
	mCamera->lookAt(mFocus);

	mTrayMgr->createButton(OgreBites::TL_BOTTOM, "Look At", "Look At");
	mTrayMgr->createCheckBox(OgreBites::TL_BOTTOM, "tglParent", "Use Parent Node");
	OgreBites::CheckBox* chk = mTrayMgr->createCheckBox(OgreBites::TL_BOTTOM, "tglFixedYaw", "Use Fixed Yaw");
	chk->setChecked(true, false);
	mTrayMgr->showCursor();
	setDragLook(true);

	mParentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(1000, 2000, -1000));

}
void PlayPen_testCameraSetDirection::buttonHit(OgreBites::Button* button)
{
	mCamera->lookAt(mFocus);
}

void PlayPen_testCameraSetDirection::checkBoxToggled(OgreBites::CheckBox* box)
{
	if (box->getName() == "tglParent")
	{
		mUseParentNode = !mUseParentNode;

		if (mUseParentNode)
			mParentNode->attachObject(mCamera);
		else
			mParentNode->detachAllObjects();
	}
	else if (box->getName() == "tglFixedYaw")
	{
		mUseFixedYaw = !mUseFixedYaw;
		if (mUseFixedYaw)
			mCamera->setFixedYawAxis(true);
		else
			mCamera->setFixedYawAxis(false);

	}
}
//---------------------------------------------------------------------
PlayPen_testManualLOD::PlayPen_testManualLOD()
{
	mInfo["Title"] = "PlayPen: Test Manual LOD";
	mInfo["Description"] = "Testing meshes with manual LODs assigned";
}
//---------------------------------------------------------------------
String PlayPen_testManualLOD::getLODMesh()
{
	MeshPtr msh1 = (MeshPtr)MeshManager::getSingleton().load("robot.mesh", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	msh1->createManualLodLevel(200, "razor.mesh");
	msh1->createManualLodLevel(500, "sphere.mesh");

	return msh1->getName();

}
//---------------------------------------------------------------------
void PlayPen_testManualLOD::setupContent()
{
	String meshName = getLODMesh();

	Entity *ent;
	for (int i = 0; i < 5; ++i)
	{
		ent = mSceneMgr->createEntity("robot" + StringConverter::toString(i), meshName);
		// Add entity to the scene node
		mSceneMgr->getRootSceneNode()->createChildSceneNode(
			Vector3(0,0,(i*50)-(5*50/2)))->attachObject(ent);
	}
	mAnimState = ent->getAnimationState("Walk");
	mAnimState->setEnabled(true);



	// Give it a little ambience with lights
	Light* l;
	l = mSceneMgr->createLight("BlueLight");
	l->setPosition(-200,-80,-100);
	l->setDiffuseColour(0.5, 0.5, 1.0);

	l = mSceneMgr->createLight("GreenLight");
	l->setPosition(0,0,-100);
	l->setDiffuseColour(0.5, 1.0, 0.5);

	// Position the camera
	mCamera->setPosition(100,50,100);
	mCamera->lookAt(-50,50,0);

	mSceneMgr->setAmbientLight(ColourValue::White);

}
//---------------------------------------------------------------------
bool PlayPen_testManualLOD::frameStarted(const Ogre::FrameEvent& evt)
{
	mAnimState->addTime(evt.timeSinceLastFrame);

	return PlayPenBase::frameStarted(evt);
}
//---------------------------------------------------------------------
PlayPen_testManualLODFromFile::PlayPen_testManualLODFromFile()
{
	mInfo["Title"] = "PlayPen: Test Manual LOD (file)";
	mInfo["Description"] = "Testing meshes with manual LODs assigned, loaded from a file";
}
//---------------------------------------------------------------------
String PlayPen_testManualLODFromFile::getLODMesh()
{
	MeshPtr msh1 = (MeshPtr)MeshManager::getSingleton().load("robot.mesh", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	msh1->createManualLodLevel(200, "razor.mesh");
	msh1->createManualLodLevel(500, "sphere.mesh");

	// this time, we save this data to a file and re-load it

	MeshSerializer ser;
	const ResourceGroupManager::LocationList& ll = 
		ResourceGroupManager::getSingleton().getResourceLocationList(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	String prefix;
	for (ResourceGroupManager::LocationList::const_iterator i = ll.begin(); i != ll.end(); ++i)
	{
		if (StringUtil::endsWith((*i)->archive->getName(), "media"))
		{
			prefix = (*i)->archive->getName();
		}
	}
	ser.exportMesh(msh1.get(), prefix + "/testlod.mesh");

	MeshManager::getSingleton().removeAll();

	return "testlod.mesh";

}
//---------------------------------------------------------------------
PlayPen_testFullScreenSwitch::PlayPen_testFullScreenSwitch()
{
	mInfo["Title"] = "PlayPen: Test full screen";
	mInfo["Description"] = "Testing switching full screen modes without re-initialisation";

}
//---------------------------------------------------------------------
void PlayPen_testFullScreenSwitch::setupContent()
{
	m640x480w = mTrayMgr->createButton(TL_CENTER, "m640x480w", "640 x 480 (windowed)", 300);
	m640x480fs = mTrayMgr->createButton(TL_CENTER, "m640x480fs", "640 x 480 (fullscreen)", 300);
	m800x600w = mTrayMgr->createButton(TL_CENTER, "m800x600w", "800 x 600 (windowed)", 300);
	m800x600fs = mTrayMgr->createButton(TL_CENTER, "m800x600fs", "800 x 600 (fullscreen)", 300);
	m1024x768w = mTrayMgr->createButton(TL_CENTER, "m1024x768w", "1024 x 768 (windowed)", 300);
	m1024x768fs = mTrayMgr->createButton(TL_CENTER, "m1024x768fs", "1024 x 768 (fullscreen)", 300);

	mTrayMgr->showCursor();

}
//---------------------------------------------------------------------
void PlayPen_testFullScreenSwitch::buttonHit(OgreBites::Button* button)
{
	if (button == m640x480w)
		mWindow->setFullscreen(false, 640, 480);
	else if (button == m640x480fs)
		mWindow->setFullscreen(true, 640, 480);
	else if (button == m800x600w)
		mWindow->setFullscreen(false, 800, 600);
	else if (button == m800x600fs)
		mWindow->setFullscreen(true, 800, 600);
	else if (button == m1024x768w)
		mWindow->setFullscreen(false, 1024, 768);
	else if (button == m1024x768fs)
		mWindow->setFullscreen(true, 1024, 768);
}



