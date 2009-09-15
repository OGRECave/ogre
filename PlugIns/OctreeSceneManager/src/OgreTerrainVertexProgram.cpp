
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

#include "OgreTerrainVertexProgram.h"

namespace Ogre {

    String TerrainVertexProgram::mNoFogArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 1, 1, 1, 1 };\n"
        "#var float4x4 worldViewProj :  : c[0], 4 : 8 : 1\n"
        "#var float morphFactor :  : c[4] : 9 : 1\n"
        "TEMP R0;\n"
        "ATTRIB v17 = vertex.attrib[1];\n"
        "ATTRIB v25 = vertex.texcoord[1];\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "	MOV result.texcoord[0], v24;\n"
        "	MOV result.texcoord[1], v25;\n"
        "	MOV R0, v16;\n"
        "	MAD R0.y, v17.x, c4.x, R0.y;\n"
        "	DP4 result.position.x, c0[0], R0;\n"
        "	DP4 result.position.y, c0[1], R0;\n"
        "	DP4 result.position.z, c0[2], R0;\n"
        "	DP4 result.position.w, c0[3], R0;\n"
        "	MOV result.color.front.primary, c5.x;\n"
        "END\n";

	String TerrainVertexProgram::mShadowReceiverArbvp1 = 
		"!!ARBvp1.0\n"
		"PARAM c[14] = { program.local[0..12], { 1 } };\n"
		"TEMP R0;\n"
		"TEMP R1;\n"
		"TEMP R2;\n"
		"MOV result.color, c[13].x;\n"
		"MUL R0.x, vertex.attrib[1], c[12];\n"
		"ADD R1.y, R0.x, vertex.position;\n"
		"MOV R1.xzw, vertex.position;\n"
		"DP4 result.position.w, R1, c[3];\n"
		"DP4 result.position.z, R1, c[2];\n"
		"DP4 R0.w, R1, c[7];\n"
		"DP4 R0.z, R1, c[6];\n"
		"DP4 R0.y, R1, c[5];\n"
		"DP4 R0.x, R1, c[4];\n"
		"DP4 result.position.y, R1, c[1];\n"
		"DP4 R2.y, R0, c[9];\n"
		"DP4 R2.z, R0, c[11];\n"
		"DP4 R2.x, R0, c[8];\n"
		"RCP R0.x, R2.z;\n"
		"DP4 result.position.x, R1, c[0];\n"
		"MUL result.texcoord[0].xy, R2, R0.x;\n"
		"END\n";
    String TerrainVertexProgram::mLinearFogArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 1, 1, 1, 1 };\n"
        "#var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "#var float morphFactor :  : c[4] : 10 : 1\n"
        "TEMP R0, R1;\n"
        "ATTRIB v17 = vertex.attrib[1];\n"
        "ATTRIB v25 = vertex.texcoord[1];\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "	MOV result.texcoord[0], v24;\n"
        "	MOV result.texcoord[1], v25;\n"
        "	MOV R1, v16;\n"
        "	MAD R1.y, v17.x, c4.x, R1.y;\n"
        "	DP4 R0.x, c0[0], R1;\n"
        "	DP4 R0.y, c0[1], R1;\n"
        "	DP4 R0.z, c0[2], R1;\n"
        "	DP4 R0.w, c0[3], R1;\n"
        "	MOV result.fogcoord.x, R0.z;\n"
        "	MOV result.position, R0;\n"
        "	MOV result.color.front.primary, c5.x;\n"
        "END\n";
    String TerrainVertexProgram::mExpFogArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 1, 1, 1 };\n"
        "PARAM c7 = { 2.71828, 0, 0, 0 };\n"
        "#var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "#var float morphFactor :  : c[4] : 10 : 1\n"
        "#var float fogDensity :  : c[5] : 11 : 1\n"
        "TEMP R0, R1;\n"
        "ATTRIB v17 = vertex.attrib[1];\n"
        "ATTRIB v25 = vertex.texcoord[1];\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c5 = program.local[5];\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "	MOV result.texcoord[0], v24;\n"
        "	MOV result.texcoord[1], v25;\n"
        "	MOV R1, v16;\n"
        "	MAD R1.y, v17.x, c4.x, R1.y;\n"
        "	DP4 R0.x, c0[0], R1;\n"
        "	DP4 R0.y, c0[1], R1;\n"
        "	DP4 R0.z, c0[2], R1;\n"
        "	DP4 R0.w, c0[3], R1;\n"
        "	MOV result.position, R0;\n"
        "	MOV result.color.front.primary, c6.x;\n"
        "	MUL R0.zw, R0.z, c5.x;\n"
        "	MOV R0.xy, c7.x;\n"
        "	LIT R0.z, R0;\n"
        "	RCP result.fogcoord.x, R0.z;\n"
        "END\n";
    String TerrainVertexProgram::mExp2FogArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 1, 1, 1 };\n"
        "PARAM c7 = { 0.002, 2.71828, 0, 0 };\n"
        "#var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "#var float morphFactor :  : c[4] : 10 : 1\n"
        "#var float fogDensity :  : c[5] : 11 : 1\n"
        "TEMP R0, R1;\n"
        "ATTRIB v17 = vertex.attrib[1];\n"
        "ATTRIB v25 = vertex.texcoord[1];\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "	MOV result.texcoord[0], v24;\n"
        "	MOV result.texcoord[1], v25;\n"
        "	MOV R1, v16;\n"
        "	MAD R1.y, v17.x, c4.x, R1.y;\n"
        "	DP4 R0.x, c0[0], R1;\n"
        "	DP4 R0.y, c0[1], R1;\n"
        "	DP4 R0.z, c0[2], R1;\n"
        "	DP4 R0.w, c0[3], R1;\n"
        "	MOV result.position, R0;\n"
        "	MOV result.color.front.primary, c6.x;\n"
        "	MUL R0.x, R0.z, c7.x;\n"
        "	MUL R0.zw, R0.x, R0.x;\n"
        "	MOV R0.xy, c7.y;\n"
        "	LIT R0.z, R0;\n"
        "	RCP result.fogcoord.x, R0.z;\n"
        "END\n";

    String TerrainVertexProgram::mNoFogVs_1_1 = 
        "vs_1_1\n"
        "def c5, 1, 1, 1, 1\n"
        "//var float4x4 worldViewProj :  : c[0], 4 : 8 : 1\n"
        "//var float morphFactor :  : c[4] : 9 : 1\n"
        "dcl_blendweight v1\n"
        "dcl_texcoord1 v8\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "	mov oT0.xy, v7\n"
        "	mov oT1.xy, v8\n"
        "	mov r0, v0\n"
        "	mad r0.y, v1.x, c4.x, r0.y\n"
        "	dp4 oPos.x, c0, r0\n"
        "	dp4 oPos.y, c1, r0\n"
        "	dp4 oPos.z, c2, r0\n"
        "	dp4 oPos.w, c3, r0\n"
        "	mov oD0, c5.x\n";
    String TerrainVertexProgram::mLinearFogVs_1_1 = 
        "vs_1_1\n"
        "def c5, 1, 1, 1, 1\n"
        "//var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "//var float morphFactor :  : c[4] : 10 : 1\n"
        "dcl_blendweight v1\n"
        "dcl_texcoord1 v8\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "	mov oT0.xy, v7\n"
        "	mov oT1.xy, v8\n"
        "	mov r1, v0\n"
        "	mad r1.y, v1.x, c4.x, r1.y\n"
        "	dp4 r0.x, c0, r1\n"
        "	dp4 r0.y, c1, r1\n"
        "	dp4 r0.z, c2, r1\n"
        "	dp4 r0.w, c3, r1\n"
        "	mov oFog, r0.z\n"
        "	mov oPos, r0\n"
        "	mov oD0, c5.x\n";

	String TerrainVertexProgram::mShadowReceiverVs_1_1 = 
		"vs_1_1\n"
		"def c13, 1, 1, 1, 1\n"
		"dcl_blendweight v1\n"
		"dcl_texcoord1 v8\n"
		"dcl_texcoord0 v7\n"
		"dcl_position v0\n"
		"mov r1, v0\n"
		"mad r1.y, v1.x, c12.x, r1.y\n"
		"dp4 oPos.x, c0, r1\n"
		"dp4 oPos.y, c1, r1\n"
		"dp4 oPos.z, c2, r1\n"
		"dp4 oPos.w, c3, r1\n"
		"dp4 r0.x, c4, r1\n"
		"dp4 r0.y, c5, r1\n"
		"dp4 r0.z, c6, r1\n"
		"dp4 r0.w, c7, r1\n"
		"dp4 r1.x, c8, r0\n"
		"dp4 r1.y, c9, r0\n"
		"dp4 r1.w, c11, r0\n"
		"rcp r0.x, r1.w\n"
		"mul oT0.xy, r1.xy, r0.x\n"
		"mov oD0, c13\n";
	String TerrainVertexProgram::mExpFogVs_1_1 = 
        "vs_1_1\n"
        "def c6, 1, 1, 1, 1\n"
        "def c7, 2.71828, 0, 0, 0\n"
        "//var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "//var float morphFactor :  : c[4] : 10 : 1\n"
        "//var float fogDensity :  : c[5] : 11 : 1\n"
        "dcl_blendweight v1\n"
        "dcl_texcoord1 v8\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "	mov oT0.xy, v7\n"
        "	mov oT1.xy, v8\n"
        "	mov r1, v0\n"
        "	mad r1.y, v1.x, c4.x, r1.y\n"
        "	dp4 r0.x, c0, r1\n"
        "	dp4 r0.y, c1, r1\n"
        "	dp4 r0.z, c2, r1\n"
        "	dp4 r0.w, c3, r1\n"
        "	mov oPos, r0\n"
        "	mov oD0, c6.x\n"
        "	mul r0.zw, r0.z, c5.x\n"
        "	mov r0.xy, c7.x\n"
        "	lit r0.z, r0\n"
        "	rcp oFog, r0.z\n";
    String TerrainVertexProgram::mExp2FogVs_1_1 = 
        "vs_1_1\n"
        "def c6, 1, 1, 1, 1\n"
        "def c7, 0.002, 2.71828, 0, 0\n"
        "//var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "//var float morphFactor :  : c[4] : 10 : 1\n"
        "//var float fogDensity :  : c[5] : 11 : 1\n"
        "dcl_blendweight v1\n"
        "dcl_texcoord1 v8\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "	mov oT0.xy, v7\n"
        "	mov oT1.xy, v8\n"
        "	mov r1, v0\n"
        "	mad r1.y, v1.x, c4.x, r1.y\n"
        "	dp4 r0.x, c0, r1\n"
        "	dp4 r0.y, c1, r1\n"
        "	dp4 r0.z, c2, r1\n"
        "	dp4 r0.w, c3, r1\n"
        "	mov oPos, r0\n"
        "	mov oD0, c6.x\n"
        "	mul r0.x, r0.z, c7.x\n"
        "	mul r0.zw, r0.x, r0.x\n"
        "	mov r0.xy, c7.y\n"
        "	lit r0.z, r0\n"
        "	rcp oFog, r0.z\n";

    const String& TerrainVertexProgram::getProgramSource(
        FogMode fogMode, const String syntax, bool shadowReceiver)
    {
		if (shadowReceiver)
		{
			if (syntax == "arbvp1")
			{
				return mShadowReceiverArbvp1;
			}
			else
			{
				return mShadowReceiverVs_1_1;
			}
		}
		else
		{

			switch(fogMode)
			{
			case FOG_NONE:
				if (syntax == "arbvp1")
				{
					return mNoFogArbvp1;
				}
				else
				{
					return mNoFogVs_1_1;
				}
				break;
			case FOG_LINEAR:
				if (syntax == "arbvp1")
				{
					return mLinearFogArbvp1;
				}
				else
				{
					return mLinearFogVs_1_1;
				}
				break;
			case FOG_EXP:
				if (syntax == "arbvp1")
				{
					return mExpFogArbvp1;
				}
				else
				{
					return mExpFogVs_1_1;
				}
				break;
			case FOG_EXP2:
				if (syntax == "arbvp1")
				{
					return mExp2FogArbvp1;
				}
				else
				{
					return mExp2FogVs_1_1;
				}
				break;
			};
		}
        // default
        return StringUtil::BLANK;

    }
}
