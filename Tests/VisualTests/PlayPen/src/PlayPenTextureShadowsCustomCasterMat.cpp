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

PlayPen_TextureShadowsCustomCasterMat::PlayPen_TextureShadowsCustomCasterMat()
{
	mInfo["Title"] = "PlayPen_TextureShadowsCustomCasterMat";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_TextureShadowsCustomCasterMat::setupContent()
{
	PlayPen_TextureShadows::setupContent();
	
	String customCasterMatVp = 
	"void customCasterVp(float4 position : POSITION,\n"
	"out float4 oPosition : POSITION,\n"
	"uniform float4x4 worldViewProj)\n"
	"{\n"
	"	oPosition = mul(worldViewProj, position);\n"
	"}\n";
	String customCasterMatFp = 
	"void customCasterFp(\n"
	"out float4 oColor : COLOR)\n"
	"{\n"
	"	oColor = float4(1,1,0,1); // just a test\n"
	"}\n";
	
	HighLevelGpuProgramPtr vp = HighLevelGpuProgramManager::getSingleton()
	.createProgram("CustomShadowCasterVp", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
	"cg", GPT_VERTEX_PROGRAM);
	vp->setSource(customCasterMatVp);
	vp->setParameter("profiles", "vs_1_1 arbvp1");
	vp->setParameter("entry_point", "customCasterVp");
	vp->load();
	
	HighLevelGpuProgramPtr fp = HighLevelGpuProgramManager::getSingleton()
	.createProgram("CustomShadowCasterFp", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
	"cg", GPT_FRAGMENT_PROGRAM);
	fp->setSource(customCasterMatFp);
	fp->setParameter("profiles", "ps_1_1 arbfp1");
	fp->setParameter("entry_point", "customCasterFp");
	fp->load();
	
	MaterialPtr mat = MaterialManager::getSingleton().create("CustomShadowCaster", 
	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setVertexProgram("CustomShadowCasterVp");
	p->getVertexProgramParameters()->setNamedAutoConstant(
	"worldViewProj", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	p->setFragmentProgram("CustomShadowCasterFp");
	
	mSceneMgr->setShadowTextureCasterMaterial("CustomShadowCaster");
	
	
	
	
	
}
