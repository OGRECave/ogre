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

PlayPen_LightClipPlanesMoreLights::PlayPen_LightClipPlanesMoreLights()
{
	mInfo["Title"] = "PlayPen_LightClipPlanesMoreLights";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_LightClipPlanesMoreLights::setupContent()
{
	mSceneMgr->setAmbientLight(ColourValue(0.3, 0.25, 0.2, 0));
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
	mSceneMgr->setShadowTextureCount(3);
	
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 0;
	MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, plane,
	4500,4500,100,100,true,1,5,5,Vector3::UNIT_Z);
	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("Examples/GrassFloor");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	Real lightRange = 1000;
	Real spotWidth = 300;
	
	int numLights = 8;
	Real xoff = -numLights * spotWidth * 0.5;
	
	
	for (int i = 0; i < numLights; ++i)
	{
		Light* l = mSceneMgr->createLight("l" + StringConverter::toString(i));
		l->setAttenuation(lightRange, 1, 0, 0);
		/* SPOT LIGHT
		*/
		// match spot width to groud
		Real spotHeight = lightRange * 0.5;
		SceneNode* n = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3((spotWidth+50)*i + xoff,spotHeight,0));
		l->setType(Light::LT_SPOTLIGHT);
		Radian spotAngle = Math::ATan(spotWidth / spotHeight) * 2;
		l->setSpotlightOuterAngle(spotAngle); 
		l->setSpotlightInnerAngle(spotAngle * 0.75);
		Vector3 dir(0, -1, 0);
		dir.normalise();
		l->setDirection(dir);
		
		/* END SPOT LIGHT */
		n->attachObject(l);
		
		Entity* e = mSceneMgr->createEntity("e" + StringConverter::toString(i), "robot.mesh");
		SceneNode* en = n->createChildSceneNode(Vector3(0, -200, 0));
		en->attachObject(e);
		
		
	}
	
	// Modify the plane material so that it clips to the light on the second pass, post ambient
	
	MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/GrassFloor");
	Pass* p = mat->getTechnique(0)->getPass(0);
	String texname = p->getTextureUnitState(0)->getTextureName();
	p->removeAllTextureUnitStates();
	p->setIlluminationStage(IS_AMBIENT);
	p->setDiffuse(ColourValue::Black);
	p = mat->getTechnique(0)->createPass();
	p->setIlluminationStage(IS_PER_LIGHT);
	p->setIteratePerLight(true, false);
	p->setAmbient(ColourValue::Black);
	p->setLightClipPlanesEnabled(true);
	p->setSceneBlending(SBT_ADD);
	//if (scissortoo)
	p->setLightScissoringEnabled(true);
	p = mat->getTechnique(0)->createPass();
	p->setIlluminationStage(IS_DECAL);
	p->createTextureUnitState(texname);
	p->setLightingEnabled(false);
	p->setSceneBlending(SBT_MODULATE);
	
	
	mCamera->setPosition(0, 200, 300);
	mCamera->lookAt(Vector3::ZERO);
	
	
	
}
