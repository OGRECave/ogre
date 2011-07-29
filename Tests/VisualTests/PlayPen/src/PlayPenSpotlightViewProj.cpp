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

PlayPen_SpotlightViewProj::PlayPen_SpotlightViewProj()
{
	mInfo["Title"] = "PlayPen_SpotlightViewProj";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_SpotlightViewProj::setupContent()
{
	SceneNode* mTestNode[10];
	bool worldViewProj = true;
	// Define programs that use spotlight projection
	
	String vpStr;
	vpStr = 
	"void vp(float4 position : POSITION,\n"
	"out float4 oPosition : POSITION,\n"
	"out float4 oUV : TEXCOORD0,\n";
	if (!worldViewProj)
	{
		vpStr += "uniform float4x4 world,\n"
		"uniform float4x4 spotlightViewProj,\n";
	}
	else
	{
		vpStr += "uniform float4x4 spotlightWorldViewProj,\n";
	}
	vpStr += "uniform float4x4 worldViewProj)\n"
	"{\n"
	"	oPosition = mul(worldViewProj, position);\n";
	if (worldViewProj)
	{
		vpStr += "	oUV = mul(spotlightWorldViewProj, position);\n";
	}
	else
	{
		vpStr += "	float4 worldPos = mul(world, position);\n"
		"	oUV = mul(spotlightViewProj, worldPos);\n";
	}
	vpStr += "}\n";
	
	String fpStr = 
	"void fp(\n"
	"float4 uv : TEXCOORD0,\n"
	"uniform sampler2D tex : register(s0),\n"
	"out float4 oColor : COLOR)\n"
	"{\n"
	"   uv = uv / uv.w;\n"
	"	oColor = tex2D(tex, uv.xy);\n"
	"}\n";
	
	HighLevelGpuProgramPtr vp = HighLevelGpuProgramManager::getSingleton()
	.createProgram("testvp", 
	TRANSIENT_RESOURCE_GROUP, 
	"cg", GPT_VERTEX_PROGRAM);
	vp->setSource(vpStr);
	vp->setParameter("profiles", "vs_1_1 arbvp1");
	vp->setParameter("entry_point", "vp");
	vp->load();
	
	HighLevelGpuProgramPtr fp = HighLevelGpuProgramManager::getSingleton()
	.createProgram("testfp", 
	TRANSIENT_RESOURCE_GROUP, 
	"cg", GPT_FRAGMENT_PROGRAM);
	fp->setSource(fpStr);
	fp->setParameter("profiles", "ps_2_0 arbfp1");
	fp->setParameter("entry_point", "fp");
	fp->load();
	
	MaterialPtr mat = MaterialManager::getSingleton().create("TestSpotlightProj", 
	TRANSIENT_RESOURCE_GROUP);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setVertexProgram("testvp");
	p->getVertexProgramParameters()->setNamedAutoConstant(
	"worldViewProj", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	
	if (worldViewProj)
	{
		p->getVertexProgramParameters()->setNamedAutoConstant(
		"spotlightWorldViewProj", GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX);
	}
	else
	{
		p->getVertexProgramParameters()->setNamedAutoConstant(
		"world", GpuProgramParameters::ACT_WORLD_MATRIX);
		p->getVertexProgramParameters()->setNamedAutoConstant(
		"spotlightViewProj", GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX);
	}
	p->setFragmentProgram("testfp");
	p->createTextureUnitState("ogrelogo.png");
	
	Entity* pEnt;
	
	// Define a plane mesh, use the above material
	Plane plane;
	plane.normal = Vector3::UNIT_Z;
	plane.d = 200;
	MeshManager::getSingleton().createPlane("WallPlane",
	TRANSIENT_RESOURCE_GROUP,
	plane,1500,1500,100,100,true,1,5,5,Vector3::UNIT_Y);
	pEnt = mSceneMgr->createEntity( "5", "WallPlane" );
	pEnt->setMaterialName(mat->getName());
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pEnt);
	
	
	mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	
	mTestNode[0]->translate(0, 0, 750);
	
	Light* spot = mSceneMgr->createLight("l1");
	spot->setType(Light::LT_SPOTLIGHT);
	spot->setDirection(Vector3::NEGATIVE_UNIT_Z);
	
	mTestNode[0]->attachObject(spot);
	
	
}
