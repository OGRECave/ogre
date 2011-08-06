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

PlayPen_VertexTexture::PlayPen_VertexTexture()
{
	mInfo["Title"] = "PlayPen_VertexTexture";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_VertexTexture::setupContent()
{
	
	// NOTE: DirectX only right now
	
	Light* l = mSceneMgr->createLight("MainLight");
	l->setType(Light::LT_POINT);
	l->setPosition(0, 200, 0);
	
	
	// Create single-channel floating point texture, no mips
	TexturePtr tex = TextureManager::getSingleton().createManual(
	"vertexTexture", TRANSIENT_RESOURCE_GROUP, TEX_TYPE_2D, 
	128, 128, 0, PF_FLOAT32_R);
	float* pData = static_cast<float*>(
	tex->getBuffer()->lock(HardwareBuffer::HBL_DISCARD));
	// write concentric circles into the texture
	for (int y  = -64; y < 64; ++y)
	{
		for (int x = -64; x < 64; ++x)
		{
			
			float val = Math::Sqrt(x*x + y*y);
			// repeat every 20 pixels
			val = val * Math::TWO_PI / 20.0f;
			*pData++ = Math::Sin(val);
		}
	}
	tex->getBuffer()->unlock();
	
	String progSource = 
	"void main(\n"
	"float4 pos : POSITION,\n"
	"float2 uv1 : TEXCOORD0,\n"
	"uniform float4x4 world, \n"
	"uniform float4x4 viewProj,\n"
	"uniform float heightscale,\n"
	"uniform sampler2D heightmap,\n"
	"out float4 oPos : POSITION,\n"
	"out float2 oUv1 : TEXCOORD1,\n"
	"out float4 col : COLOR)\n"
	"{\n"
	"oPos = mul(world, pos);\n"
	"// tex2Dlod since no mip\n"
	"float4 t = float4(0,0,0,0);\n"
	"t.xy = uv1.xy;\n"
	"float height = tex2Dlod(heightmap, t);\n"
	"oPos.y = oPos.y + (height * heightscale);\n"
	"oPos = mul(viewProj, oPos);\n"
	"oUv1 = uv1;\n"
	"col = float4(1,1,1,1);\n"
	"}\n";
	HighLevelGpuProgramPtr prog = HighLevelGpuProgramManager::getSingleton().createProgram(
	"TestVertexTextureFetch", TRANSIENT_RESOURCE_GROUP, 
	"hlsl", GPT_VERTEX_PROGRAM);
	prog->setSource(progSource);
	prog->setParameter("target", "vs_3_0");
	prog->setVertexTextureFetchRequired(true);
	prog->setParameter("entry_point", "main");
	prog->load();
	
	
	MaterialPtr mat = MaterialManager::getSingleton().create("TestVertexTexture", 
	TRANSIENT_RESOURCE_GROUP);
	Pass* pass = mat->getTechnique(0)->getPass(0);
	pass->setLightingEnabled(false);
	pass->setVertexProgram("TestVertexTextureFetch");
	GpuProgramParametersSharedPtr vp = pass->getVertexProgramParameters();
	vp->setNamedAutoConstant("world", GpuProgramParameters::ACT_WORLD_MATRIX);
	vp->setNamedAutoConstant("viewProj", GpuProgramParameters::ACT_VIEWPROJ_MATRIX);
	vp->setNamedConstant("heightscale", 30.0f);
	// vertex texture
	TextureUnitState* t = pass->createTextureUnitState("vertexTexture");
	t->setBindingType(TextureUnitState::BT_VERTEX);
	// regular texture
	pass->createTextureUnitState("BumpyMetal.jpg");
	
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 100;
	// 128 x 128 segment plane
	MeshManager::getSingleton().createPlane("Myplane",
	TRANSIENT_RESOURCE_GROUP, plane,
	1500,1500,128,128,true,1,1,1,Vector3::UNIT_Z);
	Entity* pPlaneEnt;
	pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("TestVertexTexture");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
	
	
}
