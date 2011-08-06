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

PlayPen_16Textures::PlayPen_16Textures()
{
	mInfo["Title"] = "PlayPen_16Textures";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_16Textures::setupContent()
{
	
	HighLevelGpuProgramPtr frag;
	if (StringUtil::match(Root::getSingleton().getRenderSystem()->getName(), "*GL*"))
	{
		frag = HighLevelGpuProgramManager::getSingleton().createProgram("frag16", TRANSIENT_RESOURCE_GROUP,
		"glsl", GPT_FRAGMENT_PROGRAM);
		frag->setSource(" \
		uniform sampler2D tex0; \
		uniform sampler2D tex1; \
		uniform sampler2D tex2; \
		uniform sampler2D tex3; \
		uniform sampler2D tex4; \
		uniform sampler2D tex5; \
		uniform sampler2D tex6; \
		uniform sampler2D tex7; \
		uniform sampler2D tex8; \
		uniform sampler2D tex9; \
		uniform sampler2D tex10; \
		uniform sampler2D tex11; \
		uniform sampler2D tex12; \
		uniform sampler2D tex13; \
		uniform sampler2D tex14; \
		uniform sampler2D tex15; \
		void main() \
		{ \
			gl_FragColor = texture2D(tex15, gl_TexCoord[0].xy); \
		} \
		");
		
	}
	else
	{
		// DirectX
		frag = HighLevelGpuProgramManager::getSingleton().createProgram("frag16", TRANSIENT_RESOURCE_GROUP,
		"hlsl", GPT_FRAGMENT_PROGRAM);
		frag->setParameter("target", "ps_2_0");
		frag->setParameter("entry_point", "main");
		frag->setSource(" \
		float4 main( \
		float2 uv : TEXCOORD0, \
		uniform sampler2D tex0 : register(s0), \
		uniform sampler2D tex1 : register(s1), \
		uniform sampler2D tex2 : register(s2), \
		uniform sampler2D tex3 : register(s3), \
		uniform sampler2D tex4 : register(s4), \
		uniform sampler2D tex5 : register(s5), \
		uniform sampler2D tex6 : register(s6), \
		uniform sampler2D tex7 : register(s7), \
		uniform sampler2D tex8 : register(s8), \
		uniform sampler2D tex9 : register(s9), \
		uniform sampler2D tex10 : register(s10), \
		uniform sampler2D tex11 : register(s11), \
		uniform sampler2D tex12 : register(s12), \
		uniform sampler2D tex13 : register(s13), \
		uniform sampler2D tex14 : register(s14), \
		uniform sampler2D tex15 : register(s15) \
		) : COLOR \
		{ \
			return tex2D(tex15, uv); \
		} \
		");
	}
	frag->load();
	
	MaterialPtr mat = MaterialManager::getSingleton().create("test16", TRANSIENT_RESOURCE_GROUP);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setVertexProgram("Ogre/BasicVertexPrograms/AmbientOneTextureUnified");
	p->setFragmentProgram(frag->getName());
	// create 15 textures the same
	for (int i = 0; i < 15; ++i)
	{
		p->createTextureUnitState("Dirt.jpg");
	}
	// create 16th texture differently
	p->createTextureUnitState("ogrelogo.png");
	if (StringUtil::match(Root::getSingleton().getRenderSystem()->getName(), "*GL*"))
	{
		// map samplers
		GpuProgramParametersSharedPtr params = p->getFragmentProgramParameters();
		for (int i = 0; i < 16; ++i)
		{
			params->setNamedConstant(String("tex") + StringConverter::toString(i), i);
		}
		
	}
	
	mat->load();
	
	Entity* e = mSceneMgr->createEntity("1", "knot.mesh");
	e->setMaterialName(mat->getName());
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
	
	mCamera->setPosition(0,0,250);
	mCamera->lookAt(0,0,0);
	
	
}
