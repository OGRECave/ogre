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

#include "OgreStableHeaders.h"
#include "OgreShadowVolumeExtrudeProgram.h"
#include "OgreString.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreGpuProgram.h"
#include "OgreException.h"

namespace Ogre {

    // c4 is the light position/direction in these
    String ShadowVolumeExtrudeProgram::mPointArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 0, 0, 0, 0 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0.xyz, v16.xyzx, -c4.xyzx;\n"
        "MOV R0.w, c5.x;\n"
        "MAD R0, v24.x, c4, R0;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mPointVs_1_1 = 
        "vs_1_1\n"
        "def c5, 0, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.xyz, v0.xyz, -c4.xyz\n"
        "mov r0.w, c5.x\n"
        "mad r0, v7.x, c4, r0\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

	String ShadowVolumeExtrudeProgram::mPointVs_4_0 = 
		"// Point light shadow volume extrude\n"
		"struct VS_OUTPUT\n"
		"{\n"
		"\tfloat4 Pos : SV_POSITION;\n"
		"};\n"
		"VS_OUTPUT vs_main (\n"
		"    float4 position			: POSITION,\n"
		"    float  wcoord			: TEXCOORD0,\n"
		"    uniform float4x4 worldviewproj_matrix,\n"
		"    uniform float4   light_position_object_space // homogeneous, object space\n"
		"    )\n"
		"{\n"
		"    // extrusion in object space\n"
		"    // vertex unmodified if w==1, extruded if w==0\n"
		"    float4 newpos = \n"
		"        (wcoord.xxxx * light_position_object_space) + \n"
		"        float4(position.xyz - light_position_object_space.xyz, 0);\n"
		"\n"
		"    VS_OUTPUT output = (VS_OUTPUT)0;\n"
		"    output.Pos = mul(worldviewproj_matrix, newpos);\n"
		"    return output;\n"
		"}\n";

    String ShadowVolumeExtrudeProgram::mPointVs_glsles = 
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "precision lowp sampler2D;\n"
        "precision lowp samplerCube;\n\n"
		"// Point light shadow volume extrude\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform vec4 texture_coord;\n\n"
		"void main()\n"
		"{\n"
		"    // Extrusion in object space\n"
		"    // Vertex unmodified if w==1, extruded if w==0\n"
		"    vec4 newpos = \n"
		"        (texture_coord.xxxx * light_position_object_space) + \n"
		"        vec4(gl_Position.xyz - light_position_object_space.xyz, 0);\n"
		"\n"
		"    gl_Position = worldviewproj_matrix * newpos;\n"
		"}\n";

    String ShadowVolumeExtrudeProgram::mDirArbvp1 = 
        "!!ARBvp1.0\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0, v16, c4;\n"
        "MAD R0, v24.x, R0, -c4;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mDirVs_1_1 = 
        "vs_1_1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0, v0, c4\n"
        "mad r0, v7.x, r0, -c4\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

	String ShadowVolumeExtrudeProgram::mDirVs_4_0 = 
		"// Directional light extrude\n"
		"struct VS_OUTPUT\n"
		"{\n"
		"\tfloat4 Pos : SV_POSITION;\n"
		"};\n"
		"VS_OUTPUT vs_main (\n"
		"    float4 position			: POSITION,\n"
		"    float  wcoord			: TEXCOORD0,\n"
		"\n"
		"    uniform float4x4 worldviewproj_matrix,\n"
		"    uniform float4   light_position_object_space // homogenous, object space\n"
		"    )\n"
		"{\n"
		"    // extrusion in object space\n"
		"    // vertex unmodified if w==1, extruded if w==0\n"
		"    float4 newpos = \n"
		"        (wcoord.xxxx * (position + light_position_object_space)) - light_position_object_space;\n"
		"\n"
		"    VS_OUTPUT output = (VS_OUTPUT)0;\n"
		"    output.Pos = mul(worldviewproj_matrix, newpos);\n"
		"    return output;\n"
		"}\n";

    String ShadowVolumeExtrudeProgram::mDirVs_glsles = 
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "precision lowp sampler2D;\n"
        "precision lowp samplerCube;\n\n"
		"// Directional light extrude\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform vec4 texture_coord;\n\n"
        "void main()\n"
		"{\n"
		"    // Extrusion in object space\n"
		"    // Vertex unmodified if w==1, extruded if w==0\n"
		"    vec4 newpos = \n"
		"        (texture_coord.xxxx * (gl_Position + light_position_object_space)) - light_position_object_space;\n"
		"\n"
		"    gl_Position = worldviewproj_matrix * newpos;\n"
		"}\n";


    String ShadowVolumeExtrudeProgram::mPointArbvp1Debug = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 0, 0, 0, 0 };\n"
        "PARAM c6 = { 1, 1, 1, 1 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0.xyz, v16.xyzx, -c4.xyzx;\n"
        "MOV R0.w, c5.x;\n"
        "MAD R0, v24.x, c4, R0;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "MOV result.color.front.primary, c6.x;\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mPointVs_1_1Debug = 
        "vs_1_1\n"
        "def c5, 0, 0, 0, 0\n"
        "def c6, 1, 1, 1, 1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.xyz, v0.xyz, -c4.xyz\n"
        "mov r0.w, c5.x\n"
        "mad r0, v7.x, c4, r0\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n"
        "mov oD0, c6.x\n";

	String ShadowVolumeExtrudeProgram::mPointVs_4_0Debug = mPointVs_4_0;
	String ShadowVolumeExtrudeProgram::mPointVs_glslesDebug = mPointVs_glsles;

    String ShadowVolumeExtrudeProgram::mDirArbvp1Debug = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 1, 1, 1, 1};\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0, v16, c4;\n"
        "MAD R0, v24.x, R0, -c4;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "MOV result.color.front.primary, c5.x;"
        "END\n";

    String ShadowVolumeExtrudeProgram::mDirVs_1_1Debug = 
        "vs_1_1\n"
        "def c5, 1, 1, 1, 1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0, v0, c4\n"
        "mad r0, v7.x, r0, -c4\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n"
        "mov oD0, c5.x\n";

	String ShadowVolumeExtrudeProgram::mDirVs_4_0Debug = mDirVs_4_0;
	String ShadowVolumeExtrudeProgram::mDirVs_glslesDebug = mDirVs_glsles;


    // c4 is the light position/direction in these
    // c5 is extrusion distance
    String ShadowVolumeExtrudeProgram::mPointArbvp1Finite = 
        "!!ARBvp1.0\n" 
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0, R1;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c5 = program.local[5];\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R0.w, R0.x, c5.x;\n"
        "ADD R0.xyz, v16.xyzx, -c4.xyzx;\n"
        "DP3 R1.w, R0.xyzx, R0.xyzx;\n"     // R1.w = Vector3(vertex - lightpos).sqrLength()
        "RSQ R1.w, R1.w;\n"                 // R1.w = 1 / Vector3(vertex - lightpos).length()
        "MUL R0.xyz, R1.w, R0.xyzx;\n"      // R0.xyz = Vector3(vertex - lightpos).normalisedCopy()
        "MAD R0.xyz, R0.w, R0.xyzx, v16.xyzx;\n"
        "DPH result.position.x, R0.xyzz, c0[0];\n"
        "DPH result.position.y, R0.xyzz, c0[1];\n"
        "DPH result.position.z, R0.xyzz, c0[2];\n"
        "DPH result.position.w, R0.xyzz, c0[3];\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mPointVs_1_1Finite = 
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r1.x, r0.x, c5.x\n"
        "add r0.yzw, v0.xxyz, -c4.xxyz\n"
        "dp3 r0.x, r0.yzw, r0.yzw\n"
        "rsq r0.x, r0.x\n"
        "mul r0.xyz, r0.x, r0.yzw\n"
        "mad r0.xyz, r1.x, r0.xyz, v0.xyz\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

	String ShadowVolumeExtrudeProgram::mPointVs_4_0Finite = 
		"// Point light shadow volume extrude - FINITE\n"
		"struct VS_OUTPUT\n"
		"{\n"
		"\tfloat4 Pos : SV_POSITION;\n"
		"};\n"
		"VS_OUTPUT vs_main (\n"
		"    float4 position			: POSITION,\n"
		"    float  wcoord			: TEXCOORD0,\n"
		"\n"
		"    uniform float4x4 worldviewproj_matrix,\n"
		"    uniform float4   light_position_object_space, // homogeneous, object space\n"
		"	uniform float    shadow_extrusion_distance // how far to extrude\n"
		"    )\n"
		"{\n"
		"    // extrusion in object space\n"
		"    // vertex unmodified if w==1, extruded if w==0\n"
		"	float3 extrusionDir = position.xyz - light_position_object_space.xyz;\n"
		"	extrusionDir = normalize(extrusionDir);\n"
		"	\n"
		"    float4 newpos = float4(position.xyz +  \n"
		"        ((1 - wcoord.x) * shadow_extrusion_distance * extrusionDir), 1);\n"
		"\n"
		"    VS_OUTPUT output = (VS_OUTPUT)0;\n"
		"    output.Pos = mul(worldviewproj_matrix, newpos);\n"
		"    return output;\n"

		"\n"
		"}\n";

    String ShadowVolumeExtrudeProgram::mPointVs_glslesFinite = 
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "precision lowp sampler2D;\n"
        "precision lowp samplerCube;\n\n"
		"// Point light shadow volume extrude - FINITE\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform vec4 texture_coord;\n\n"
		"uniform float shadow_extrusion_distance; // how far to extrude\n"
        "void main()\n"
		"{\n"
		"    // Extrusion in object space\n"
		"    // Vertex unmodified if w==1, extruded if w==0\n"
		"	vec3 extrusionDir = gl_Position.xyz - light_position_object_space.xyz;\n"
		"	extrusionDir = normalize(extrusionDir);\n"
		"	\n"
		"    vec4 newpos = vec4(gl_Position.xyz +  \n"
		"        ((1.0 - texture_coord.x) * shadow_extrusion_distance * extrusionDir), 1.0);\n"
		"\n"
		"    gl_Position = worldviewproj_matrix * newpos;\n"
		"}\n";

    String ShadowVolumeExtrudeProgram::mDirArbvp1Finite = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "PARAM c5 = program.local[5];\n"
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R0.x, R0.x, c5.x;\n"
        "MAD R0.xyz, -R0.x, c4.xyzx, v16.xyzx;\n"
        "DPH result.position.x, R0.xyzz, c0[0];\n"
        "DPH result.position.y, R0.xyzz, c0[1];\n"
        "DPH result.position.z, R0.xyzz, c0[2];\n"
        "DPH result.position.w, R0.xyzz, c0[3];\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mDirVs_1_1Finite = 
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r0.x, r0.x, c5.x\n"
        "mad r0.xyz, -r0.x, c4.xyz, v0.xyz\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

	String ShadowVolumeExtrudeProgram::mDirVs_4_0Finite = 
		"// Directional light extrude - FINITE\n"
		"struct VS_OUTPUT\n"
		"{\n"
		"\tfloat4 Pos : SV_POSITION;\n"
		"};\n"
		"VS_OUTPUT vs_main (\n"
		"    float4 position			: POSITION,\n"
		"    float  wcoord			: TEXCOORD0,\n"
		"\n"
		"    uniform float4x4 worldviewproj_matrix,\n"
		"    uniform float4   light_position_object_space, // homogeneous, object space\n"
		"	uniform float    shadow_extrusion_distance // how far to extrude\n"
		"    )\n"
		"{\n"
		"    // extrusion in object space\n"
		"    // vertex unmodified if w==1, extruded if w==0\n"
		"	// -ve light_position_object_space is direction\n"
		"    float4 newpos = float4(position.xyz - \n"
		"        (wcoord.x * shadow_extrusion_distance * light_position_object_space.xyz), 1);\n"
		"\n"
		"    VS_OUTPUT output = (VS_OUTPUT)0;\n"
		"    output.Pos = mul(worldviewproj_matrix, newpos);\n"
		"    return output;\n"
		"\n"
		"}\n";

	String ShadowVolumeExtrudeProgram::mDirVs_glslesFinite = 
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "precision lowp sampler2D;\n"
        "precision lowp samplerCube;\n\n"
		"// Directional light extrude - FINITE\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform vec4 texture_coord;\n\n"
        "uniform float shadow_extrusion_distance; // how far to extrude\n"
        "void main()\n"
        "{\n"
		"    // Extrusion in object space\n"
		"    // Vertex unmodified if w==1, extruded if w==0\n"
		"	 // -ve light_position_object_space is direction\n"
		"    vec4 newpos = vec4(gl_Position.xyz - \n"
		"        (texture_coord.x * shadow_extrusion_distance * light_position_object_space.xyz), 1);\n"
		"\n"
		"    gl_Position = worldviewproj_matrix * newpos;\n"
		"\n"
		"}\n";

    String ShadowVolumeExtrudeProgram::mPointArbvp1FiniteDebug = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0, R1;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c5 = program.local[5];\n"
        "PARAM c4 = program.local[4];\n"
        "MOV result.color.front.primary, c6.x;\n"
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R1.x, R0.x, c5.x;\n"
        "ADD R0.yzw, v16.xxyz, -c4.xxyz;\n"
        "DP3 R0.x, R0.yzwy, R0.yzwy;\n"
        "RSQ R0.x, R0.x;\n"
        "MUL R0.xyz, R0.x, R0.yzwy;\n"
        "MAD R0.xyz, R1.x, R0.xyzx, v16.xyzx;\n"
        "DPH result.position.x, R0.xyzz, c0[0];\n"
        "DPH result.position.y, R0.xyzz, c0[1];\n"
        "DPH result.position.z, R0.xyzz, c0[2];\n"
        "DPH result.position.w, R0.xyzz, c0[3];\n"
        "END\n";

	String ShadowVolumeExtrudeProgram::mPointVs_1_1FiniteDebug = 
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "mov oD0, c6.x\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r1.x, r0.x, c5.x\n"
        "add r0.yzw, v0.xxyz, -c4.xxyz\n"
        "dp3 r0.x, r0.yzw, r0.yzw\n"
        "rsq r0.x, r0.x\n"
        "mul r0.xyz, r0.x, r0.yzw\n"
        "mad r0.xyz, r1.x, r0.xyz, v0.xyz\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

	String ShadowVolumeExtrudeProgram::mPointVs_4_0FiniteDebug = mPointVs_4_0Finite;
	String ShadowVolumeExtrudeProgram::mPointVs_glslesFiniteDebug = mPointVs_glslesFinite;

    String ShadowVolumeExtrudeProgram::mDirArbvp1FiniteDebug = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "PARAM c5 = program.local[5];\n"
        "MOV result.color.front.primary, c6.x;\n"
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R0.x, R0.x, c5.x;\n"
        "MAD R0.xyz, -R0.x, c4.xyzx, v16.xyzx;\n"
        "DPH result.position.x, R0.xyzz, c0[0];\n"
        "DPH result.position.y, R0.xyzz, c0[1];\n"
        "DPH result.position.z, R0.xyzz, c0[2];\n"
        "DPH result.position.w, R0.xyzz, c0[3];\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mDirVs_1_1FiniteDebug = 
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "mov oD0, c6.x\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r0.x, r0.x, c5.x\n"
        "mad r0.xyz, -r0.x, c4.xyz, v0.xyz\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";


	String ShadowVolumeExtrudeProgram::mDirVs_4_0FiniteDebug = mDirVs_4_0Finite;
	String ShadowVolumeExtrudeProgram::mDirVs_glslesFiniteDebug = mDirVs_glslesFinite;


	String ShadowVolumeExtrudeProgram::mGeneralFs_4_0 = 
		"struct VS_OUTPUT\n"
		"{\n"
		"\tfloat4 Pos : SV_POSITION;\n"
		"};\n"
		"float4 fs_main (VS_OUTPUT input): SV_Target\n"
		"{\n"
		"    float4 finalColor = float4(1,1,1,1);\n"
		"    return finalColor;\n"
		"}\n";

    String ShadowVolumeExtrudeProgram::mGeneralFs_glsles = 
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "precision lowp sampler2D;\n"
        "precision lowp samplerCube;\n\n"
		"void main()\n"
		"{\n"
		"    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"}\n";

    const String ShadowVolumeExtrudeProgram::programNames[OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS] = 
    {
        "Ogre/ShadowExtrudePointLight",
            "Ogre/ShadowExtrudePointLightDebug",
            "Ogre/ShadowExtrudeDirLight",
            "Ogre/ShadowExtrudeDirLightDebug",
            "Ogre/ShadowExtrudePointLightFinite",
            "Ogre/ShadowExtrudePointLightFiniteDebug",
            "Ogre/ShadowExtrudeDirLightFinite",
            "Ogre/ShadowExtrudeDirLightFiniteDebug"
    };

	String ShadowVolumeExtrudeProgram::frgProgramName = "";

	bool ShadowVolumeExtrudeProgram::mInitialised = false;
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ShadowVolumeExtrudeProgram::initialise(void)
    {
		if (!mInitialised)
		{
			String syntax;
			bool vertexProgramFinite[OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS] = 
			{
				false, false, false, false, 
					true, true, true, true
			};
			bool vertexProgramDebug[OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS] = 
			{
				false, true, false, true, 
					false, true, false, true
			};
			Light::LightTypes vertexProgramLightTypes[OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS] = 
			{
				Light::LT_POINT, Light::LT_POINT, 
					Light::LT_DIRECTIONAL, Light::LT_DIRECTIONAL, 
					Light::LT_POINT, Light::LT_POINT, 
					Light::LT_DIRECTIONAL, Light::LT_DIRECTIONAL 
			};

			// load hardware extrusion programs for point & dir lights
			if (GpuProgramManager::getSingleton().isSyntaxSupported("arbvp1"))
			{
				// ARBvp1
				syntax = "arbvp1";
			}
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("vs_1_1"))
			{
				syntax = "vs_1_1";
			}
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0"))
			{
				syntax = "vs_4_0";
			}
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
			{
				syntax = "glsles";
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Vertex programs are supposedly supported, but neither "
					"arbvp1, glsles, vs_1_1 nor vs_4_0 syntaxes are present.", 
					"SceneManager::initShadowVolumeMaterials");
			}
			// Create all programs
			for (unsigned short v = 0; v < OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS; ++v)
			{
				// Create debug extruders
				if (GpuProgramManager::getSingleton().getByName(
					programNames[v]).isNull())
				{
					if (syntax == "vs_4_0")
					{
						HighLevelGpuProgramPtr vp = 
							HighLevelGpuProgramManager::getSingleton().createProgram(
							programNames[v], ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
							"hlsl", GPT_VERTEX_PROGRAM);
						vp->setSource(ShadowVolumeExtrudeProgram::getProgramSource(
							vertexProgramLightTypes[v], syntax, 
							vertexProgramFinite[v], vertexProgramDebug[v]));
						vp->setParameter("target", syntax);
						vp->setParameter("entry_point", "vs_main");			
						vp->load();

						if (frgProgramName.empty())
						{
							frgProgramName = "Ogre/ShadowFrgProgram";
							HighLevelGpuProgramPtr fp = 
								HighLevelGpuProgramManager::getSingleton().createProgram(
								frgProgramName, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
								"hlsl", GPT_FRAGMENT_PROGRAM);
							fp->setSource(mGeneralFs_4_0);
							fp->setParameter("target", "ps_4_0");
							fp->setParameter("entry_point", "fs_main");			
							fp->load();
						}
					}
					else if (syntax == "glsles")
					{
						HighLevelGpuProgramPtr vp = 
							HighLevelGpuProgramManager::getSingleton().createProgram(
							programNames[v], ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
							"glsles", GPT_VERTEX_PROGRAM);
						vp->setSource(ShadowVolumeExtrudeProgram::getProgramSource(
							vertexProgramLightTypes[v], syntax, 
							vertexProgramFinite[v], vertexProgramDebug[v]));
						vp->setParameter("target", syntax);
						vp->load();

						if (frgProgramName.empty())
						{
							frgProgramName = "Ogre/ShadowFrgProgram";
							HighLevelGpuProgramPtr fp = 
								HighLevelGpuProgramManager::getSingleton().createProgram(
								frgProgramName, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
								"glsles", GPT_FRAGMENT_PROGRAM);
							fp->setSource(mGeneralFs_glsles);
							fp->setParameter("target", "glsles");
							fp->load();
						}
					}
					else
					{
						GpuProgramPtr vp = 
							GpuProgramManager::getSingleton().createProgramFromString(
							programNames[v], ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
							ShadowVolumeExtrudeProgram::getProgramSource(
							vertexProgramLightTypes[v], syntax, 
							vertexProgramFinite[v], vertexProgramDebug[v]),
							GPT_VERTEX_PROGRAM, syntax);
						vp->load();
					}

				}
			}
			mInitialised = true;
		}
    }
    //---------------------------------------------------------------------
    void ShadowVolumeExtrudeProgram::shutdown(void)
    {
        if (mInitialised)
        {
            for (unsigned short v = 0; v < OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS; ++v)
            {
                // Destroy debug extruders
                GpuProgramManager::getSingleton().remove(programNames[v]);
            }
            mInitialised = false;
        }
    }
    //---------------------------------------------------------------------
    const String& ShadowVolumeExtrudeProgram::getProgramSource(
        Light::LightTypes lightType, const String syntax, bool finite, bool debug)
    {
        if (lightType == Light::LT_DIRECTIONAL)
        {
            if (syntax == "arbvp1")
            {
                if (finite)
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderArbvp1FiniteDebug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderArbvp1Finite();
                    }
                }
                else
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderArbvp1Debug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderArbvp1();
                    }
                }
            } 
            else if (syntax == "vs_1_1")
            {
                if (finite)
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderVs_1_1FiniteDebug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderVs_1_1Finite();
                    }
                }
                else
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderVs_1_1Debug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderVs_1_1();
                    }
                }
            }
			else if (syntax == "vs_4_0")
			{
				if (finite)
				{
					if (debug)
					{
						return getDirectionalLightExtruderVs_4_0FiniteDebug();
					}
					else
					{
						return getDirectionalLightExtruderVs_4_0Finite();
					}
				}
				else
				{
					if (debug)
					{
						return getDirectionalLightExtruderVs_4_0Debug();
					}
					else
					{
						return getDirectionalLightExtruderVs_4_0();
					}
				}
			}
            else if (syntax == "glsles")
            {
                if (finite)
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderVs_glslesFiniteDebug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderVs_glslesFinite();
                    }
                }
                else
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderVs_glslesDebug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderVs_glsles();
                    }
                }
            }
			else
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Vertex programs are supposedly supported, but neither "
					"arbvp1, glsles, vs_1_1 nor vs_4_0 syntaxes are present.", 
					"SceneManager::getProgramSource");
			}

        }
        else
        {
            if (syntax == "arbvp1")
            {
                if (finite)
                {
                    if (debug)
                    {
                        return getPointLightExtruderArbvp1FiniteDebug();
                    }
                    else
                    {
                        return getPointLightExtruderArbvp1Finite();
                    }
                }
                else
                {
                    if (debug)
                    {
                        return getPointLightExtruderArbvp1Debug();
                    }
                    else
                    {
                        return getPointLightExtruderArbvp1();
                    }
                }
            }
            else if (syntax == "vs_1_1")
            {
                if (finite)
                {
                    if (debug)
                    {
                        return getPointLightExtruderVs_1_1FiniteDebug();
                    }
                    else
                    {
                        return getPointLightExtruderVs_1_1Finite();
                    }
                }
                else
                {
                    if (debug)
                    {
                        return getPointLightExtruderVs_1_1Debug();
                    }
                    else
                    {
                        return getPointLightExtruderVs_1_1();
                    }
                }
            }
			else if (syntax == "vs_4_0")
			{
				if (finite)
				{
					if (debug)
					{
						return getPointLightExtruderVs_4_0FiniteDebug();
					}
					else
					{
						return getPointLightExtruderVs_4_0Finite();
					}
				}
				else
				{
					if (debug)
					{
						return getPointLightExtruderVs_4_0Debug();
					}
					else
					{
						return getPointLightExtruderVs_4_0();
					}
				}
			}
			else if (syntax == "glsles")
			{
				if (finite)
				{
					if (debug)
					{
						return getPointLightExtruderVs_glslesFiniteDebug();
					}
					else
					{
						return getPointLightExtruderVs_glslesFinite();
					}
				}
				else
				{
					if (debug)
					{
						return getPointLightExtruderVs_glslesDebug();
					}
					else
					{
						return getPointLightExtruderVs_glsles();
					}
				}
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Vertex programs are supposedly supported, but neither "
					"arbvp1, glsles, vs_1_1 nor vs_4_0 syntaxes are present.", 
					"SceneManager::getProgramSource");
			}

        }
    }
    //---------------------------------------------------------------------
    const String& ShadowVolumeExtrudeProgram::getProgramName(
        Light::LightTypes lightType, bool finite, bool debug)
    {
        if (lightType == Light::LT_DIRECTIONAL)
        {
            if (finite)
            {
                if (debug)
                {
                    return programNames[DIRECTIONAL_LIGHT_FINITE_DEBUG];
                }
                else
                {
                    return programNames[DIRECTIONAL_LIGHT_FINITE];
                }
            }
            else
            {
                if (debug)
                {
                    return programNames[DIRECTIONAL_LIGHT_DEBUG];
                }
                else
                {
                    return programNames[DIRECTIONAL_LIGHT];
                }
            }
        }
        else
        {
            if (finite)
            {
                if (debug)
                {
                    return programNames[POINT_LIGHT_FINITE_DEBUG];
                }
                else
                {
                    return programNames[POINT_LIGHT_FINITE];
                }
            }
            else
            {
                if (debug)
                {
                    return programNames[POINT_LIGHT_DEBUG];
                }
                else
                {
                    return programNames[POINT_LIGHT];
                }
            }
        }
    }



}
