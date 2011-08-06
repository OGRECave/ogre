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

PlayPen_LightClipPlanes::PlayPen_LightClipPlanes()
{
	mInfo["Title"] = "PlayPen_LightClipPlanes";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_LightClipPlanes::setupContent()
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
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(150,0,100))->attachObject(pPlaneEnt);
	
	Real lightRange = 1000;
	Real spotWidth = 300;
	
	ManualObject* debugSphere = mSceneMgr->createManualObject("debugSphere");
	debugSphere->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_STRIP);
	for (int i = 0; i <= 20; ++i)
	{
		Vector3 basePos(spotWidth, 0, 0);
		Quaternion quat;
		quat.FromAngleAxis(Radian(((float)i/(float)20)*Math::TWO_PI), Vector3::UNIT_Y);
		basePos = quat * basePos;
		debugSphere->position(basePos);
	}
	debugSphere->end();
	
	Light* l = mSceneMgr->createLight("l1");
	l->setAttenuation(lightRange, 1, 0, 0);
	SceneNode* n = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100,0,0));
	n->attachObject(debugSphere);
	/* SPOT LIGHT
	*/
	// match spot width to groud
	Real spotHeight = lightRange * 0.5;
	n = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100,spotHeight,0));
	l->setType(Light::LT_SPOTLIGHT);
	Radian spotAngle = Math::ATan(spotWidth / spotHeight) * 2;
	l->setSpotlightOuterAngle(spotAngle); 
	l->setSpotlightInnerAngle(spotAngle * 0.75);
	Vector3 dir(0, -1, 0);
	dir.normalise();
	l->setDirection(dir);
	
	/* END SPOT LIGHT */
	n->attachObject(l);
	
	// Modify the plane material so that it clips to the light
	// Normally you'd only clip a secondary pass but this is engineered so you
	// can actually see the scissoring effect
	MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/GrassFloor");
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setLightClipPlanesEnabled(true);
	//if (scissortoo)
	//p->setLightScissoringEnabled(true);
	
	mCamera->setPosition(0, 200, 300);
	mCamera->lookAt(Vector3::ZERO);
	
	
	
}
