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

PlayPen_ManualIlluminationStage::PlayPen_ManualIlluminationStage()
{
	mInfo["Title"] = "PlayPen_ManualIlluminationStage";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_ManualIlluminationStage::setupContent()
{
	mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
	mSceneMgr->setShadowDirectionalLightExtrusionDistance(1000);
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(5);
	
	// Set ambient light
	mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));
	
	Light* mLight = mSceneMgr->createLight("MainLight");
	mLight->setPosition(-400,400,-300);
	mLight->setDiffuseColour(0.9, 0.9, 1);
	mLight->setSpecularColour(0.9, 0.9, 1);
	mLight->setAttenuation(6000,1,0.001,0);
	
	
	mLight = mSceneMgr->createLight("Light2");
	mLight->setPosition(300,200,100);
	mLight->setDiffuseColour(1, 0.6, 0.5);
	mLight->setSpecularColour(0.9, 0.9, 1);
	mLight->setAttenuation(6000,1,0.001,0);
	
	
	
	MeshPtr msh = MeshManager::getSingleton().load("knot.mesh", TRANSIENT_RESOURCE_GROUP);
	msh->buildTangentVectors();
	Entity* pEnt = mSceneMgr->createEntity( "3.5", "knot.mesh" );
	pEnt->setMaterialName("Examples/OffsetMapping/Specular");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject( pEnt );
	
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	MeshPtr planeMesh = MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, plane,
	1500,1500,100,100,true,1,15,15,Vector3::UNIT_Z);
	planeMesh->buildTangentVectors();
	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("Examples/OffsetMapping/Specular");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	mCamera->setPosition(180, 34, 223);
	mCamera->lookAt(0,50,0);
}
