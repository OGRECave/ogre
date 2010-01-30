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




