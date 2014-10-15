/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"
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
        "    float4 position            : POSITION,\n"
        "    float  wcoord          : TEXCOORD0,\n"
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

    String ShadowVolumeExtrudeProgram::mPointVs_glsl = 
        "#version 150\n"
        "// Point light shadow volume extrude\n"
        "in vec4 uv0;\n"
        "in vec4 vertex;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "    vec4 newpos = \n"
        "        (uv0.xxxx * light_position_object_space) + \n"
        "        vec4(vertex.xyz - light_position_object_space.xyz, 0.0);\n"
        "\n"
        "    gl_Position = worldviewproj_matrix * newpos;\n"
        "}\n";

    String ShadowVolumeExtrudeProgram::mPointVs_glsles = 
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "precision lowp sampler2D;\n"
        "precision lowp samplerCube;\n\n"
        "// Point light shadow volume extrude\n"
        "attribute vec4 uv0;\n"
        "attribute vec4 position;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "    vec4 newpos = \n"
        "        (uv0.xxxx * light_position_object_space) + \n"
        "        vec4(position.xyz - light_position_object_space.xyz, 0.0);\n"
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
        "    float4 position            : POSITION,\n"
        "    float  wcoord          : TEXCOORD0,\n"
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

    String ShadowVolumeExtrudeProgram::mDirVs_glsl = 
        "#version 150\n"
        "// Directional light extrude\n"
        "in vec4 uv0;\n"
        "in vec4 vertex;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "    vec4 newpos = \n"
        "        (uv0.xxxx * (vertex + light_position_object_space)) - light_position_object_space;\n"
        "\n"
        "    gl_Position = worldviewproj_matrix * newpos;\n"
        "}\n";

    String ShadowVolumeExtrudeProgram::mDirVs_glsles = 
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "precision lowp sampler2D;\n"
        "precision lowp samplerCube;\n\n"
        "// Directional light extrude\n"
        "attribute vec4 uv0;\n"
        "attribute vec4 position;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "    vec4 newpos = \n"
        "        (uv0.xxxx * (position + light_position_object_space)) - light_position_object_space;\n"
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
    String ShadowVolumeExtrudeProgram::mPointVs_glslDebug = mPointVs_glsl;
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
    String ShadowVolumeExtrudeProgram::mDirVs_glslDebug = mDirVs_glsl;
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
        "PARAM c4 = program.local[4];\n"    // c4.xyz = lightPos
        "PARAM c5 = program.local[5];\n"    // c5.x = extrudeDist
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R0.w, R0.x, c5.x;\n"           // R0.w = (1 - wcoord) * extrudeDist
        "ADD R0.xyz, v16.xyzx, -c4.xyzx;\n" // R0.xyz = extrusionDir = Vector3(vertex - lightPos)
        "DP3 R1.w, R0.xyzx, R0.xyzx;\n"     // R1.w = extrusionDir.sqrLength()
        "RSQ R1.w, R1.w;\n"                 // R1.w = 1 / extrusionDir.length()
        "MUL R0.xyz, R1.w, R0.xyzx;\n"      // R0.xyz = extrusionDir.normalisedCopy()
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
        "mul r1.x, r0.x, c5.x\n"            // r1.x = (1 - wcoord) * extrudeDist
        "add r0.yzw, v0.xxyz, -c4.xxyz\n"
        "dp3 r0.x, r0.yzw, r0.yzw\n"        // r0.x = extrusionDir.sqrLength()
        "rsq r0.x, r0.x\n"                  // r0.x = 1 / extrusionDir.length()
        "mul r0.xyz, r0.x, r0.yzw\n"        // r0.xyz = extrusionDir.normalisedCopy()
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
        "    float4 position            : POSITION,\n"
        "    float  wcoord          : TEXCOORD0,\n"
        "\n"
        "    uniform float4x4 worldviewproj_matrix,\n"
        "    uniform float4   light_position_object_space, // homogeneous, object space\n"
        "   uniform float    shadow_extrusion_distance // how far to extrude\n"
        "    )\n"
        "{\n"
        "    // extrusion in object space\n"
        "    // vertex unmodified if w==1, extruded if w==0\n"
		"    float3 extrusionDir = position.xyz - light_position_object_space.xyz;\n"
		"    extrusionDir = normalize(extrusionDir);\n"
		"\n"
        "    float4 newpos = float4(position.xyz +  \n"
        "        ((1 - wcoord.x) * shadow_extrusion_distance * extrusionDir), 1);\n"
        "\n"
        "    VS_OUTPUT output = (VS_OUTPUT)0;\n"
        "    output.Pos = mul(worldviewproj_matrix, newpos);\n"
        "    return output;\n"
        "\n"
        "}\n";

    String ShadowVolumeExtrudeProgram::mPointVs_glslFinite = 
        "#version 150\n"
        "// Point light shadow volume extrude - FINITE\n"
        "in vec4 uv0;\n"
        "in vec4 vertex;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform float shadow_extrusion_distance; // how far to extrude\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "   vec3 extrusionDir = vertex.xyz - light_position_object_space.xyz;\n"
        "   extrusionDir = normalize(extrusionDir);\n"
        "   \n"
        "    vec4 newpos = vec4(vertex.xyz +  \n"
        "        ((1.0 - uv0.x) * shadow_extrusion_distance * extrusionDir), 1.0);\n"
        "\n"
        "    gl_Position = worldviewproj_matrix * newpos;\n"
        "}\n";

    String ShadowVolumeExtrudeProgram::mPointVs_glslesFinite = 
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "precision lowp sampler2D;\n"
        "precision lowp samplerCube;\n\n"
        "// Point light shadow volume extrude - FINITE\n"
        "attribute vec4 uv0;\n"
        "attribute vec4 position;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform float shadow_extrusion_distance; // how far to extrude\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "   vec3 extrusionDir = position.xyz - light_position_object_space.xyz;\n"
        "   extrusionDir = normalize(extrusionDir);\n"
        "   \n"
        "    vec4 newpos = vec4(position.xyz +  \n"
        "        ((1.0 - uv0.x) * shadow_extrusion_distance * extrusionDir), 1.0);\n"
        "\n"
        "    gl_Position = worldviewproj_matrix * newpos;\n"
        "}\n";

    String ShadowVolumeExtrudeProgram::mDirArbvp1Finite = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0, R1;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"    // c4.xyz = lightDir = -extrusionDir
        "PARAM c5 = program.local[5];\n"    // c5.x = extrudeDist
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R0.w, R0.x, c5.x;\n"           // R0.w = (1 - wcoord) * extrudeDist
        "DP3 R1.w, c4.xyzx, c4.xyzx;\n"     // R1.w = extrusionDir.sqrLength()
        "RSQ R1.w, R1.w;\n"                 // R1.w = 1 / extrusionDir.length()
        "MUL R0.xyz, R1.w, -c4.xyzx;\n"     // R0.xyz = extrusionDir.normalisedCopy()
        "MAD R0.xyz, R0.w, R0.xyzx, v16.xyzx;\n"
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
        "mul r0.w, r0.x, c5.x\n"            // r0.w = (1 - wcoord) * extrudeDist
        "dp3 r1.w, c4.xyz, c4.xyz\n"        // r1.w = extrusionDir.sqrLength()
        "rsq r1.w, r1.w\n"                  // r1.w = 1 / extrusionDir.length()
        "mul r0.xyz, r1.w, -c4.xyz\n"       // r0.xyz = extrusionDir.normalisedCopy()
        "mad r0.xyz, r0.w, r0.xyz, v0.xyz;\n"
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
        "    float4 position            : POSITION,\n"
        "    float  wcoord          : TEXCOORD0,\n"
        "\n"
        "    uniform float4x4 worldviewproj_matrix,\n"
        "    uniform float4   light_position_object_space, // homogeneous, object space\n"
        "   uniform float    shadow_extrusion_distance // how far to extrude\n"
        "    )\n"
        "{\n"
        "    // extrusion in object space\n"
        "    // vertex unmodified if w==1, extruded if w==0\n"
		"    float3 extrusionDir = - light_position_object_space.xyz;\n"
		"    extrusionDir = normalize(extrusionDir);\n"
		"\n"
		"    float4 newpos = float4(position.xyz + \n"
		"        ((1 - wcoord.x) * shadow_extrusion_distance * extrusionDir), 1);\n"
        "\n"
        "    VS_OUTPUT output = (VS_OUTPUT)0;\n"
        "    output.Pos = mul(worldviewproj_matrix, newpos);\n"
        "    return output;\n"
        "\n"
        "}\n";

    String ShadowVolumeExtrudeProgram::mDirVs_glslFinite = 
        "#version 150\n"
        "// Directional light extrude - FINITE\n"
        "in vec4 uv0;\n"
        "in vec4 vertex;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform float shadow_extrusion_distance;  // how far to extrude\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "	vec3 extrusionDir = - light_position_object_space.xyz;\n"
        "	extrusionDir = normalize(extrusionDir);\n"
        "	\n"
        "    vec4 newpos = vec4(vertex.xyz +  \n"
        "        ((1.0 - uv0.x) * shadow_extrusion_distance * extrusionDir), 1.0);\n"
        "\n"
        "    gl_Position = worldviewproj_matrix * newpos;\n"
        "\n"
        "}\n";

    String ShadowVolumeExtrudeProgram::mDirVs_glslesFinite = 
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "precision lowp sampler2D;\n"
        "precision lowp samplerCube;\n\n"
        "// Directional light extrude - FINITE\n"
        "attribute vec4 uv0;\n"
        "attribute vec4 position;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform float shadow_extrusion_distance;  // how far to extrude\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
		"	vec3 extrusionDir = - light_position_object_space.xyz;\n"
		"	extrusionDir = normalize(extrusionDir);\n"
		"	\n"
		"    vec4 newpos = vec4(position.xyz +  \n"
		"        ((1.0 - uv0.x) * shadow_extrusion_distance * extrusionDir), 1.0);\n"
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
    String ShadowVolumeExtrudeProgram::mPointVs_glslFiniteDebug = mPointVs_glslFinite;
    String ShadowVolumeExtrudeProgram::mPointVs_glslesFiniteDebug = mPointVs_glslesFinite;

    String ShadowVolumeExtrudeProgram::mDirArbvp1FiniteDebug = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0, R1;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"    // c4.xyz = lightDir = -extrusionDir
        "PARAM c5 = program.local[5];\n"    // c5.x = extrudeDist
        "MOV result.color.front.primary, c6.x;\n"
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R0.w, R0.x, c5.x;\n"           // R0.w = (1 - wcoord) * extrudeDist
        "DP3 R1.w, c4.xyzx, c4.xyzx;\n"     // R1.w = extrusionDir.sqrLength()
        "RSQ R1.w, R1.w;\n"                 // R1.w = 1 / extrusionDir.length()
        "MUL R0.xyz, R1.w, -c4.xyzx;\n"     // R0.xyz = extrusionDir.normalisedCopy()
        "MAD R0.xyz, R0.w, R0.xyzx, v16.xyzx;\n"
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
        "mul r0.w, r0.x, c5.x\n"            // r0.w = (1 - wcoord) * extrudeDist
        "dp3 r1.w, c4.xyz, c4.xyz\n"        // r1.w = extrusionDir.sqrLength()
        "rsq r1.w, r1.w\n"                  // r1.w = 1 / extrusionDir.length()
        "mul r0.xyz, r1.w, -c4.xyz\n"       // r0.xyz = extrusionDir.normalisedCopy()
        "mad r0.xyz, r0.w, r0.xyz, v0.xyz;\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";


    String ShadowVolumeExtrudeProgram::mDirVs_4_0FiniteDebug = mDirVs_4_0Finite;
    String ShadowVolumeExtrudeProgram::mDirVs_glslFiniteDebug = mDirVs_glslFinite;
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

    String ShadowVolumeExtrudeProgram::mGeneralFs_glsl = 
        "#version 150\n"
        "out vec4 fragColour;\n"
        "void main()\n"
        "{\n"
        "    fragColour = vec4(1.0);\n"
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



	 String ShadowVolumeExtrudeProgram::mModulate_Fs_hlsl_4_0 = 
		"Texture2D RT : register(t0);\
		SamplerState RTState : register(s0);\
		uniform float4 shadowColor;\
		float4 ShadowBlend_ps(float4 position : SV_POSITION, float2 iTexCoord : TEXCOORD0) : SV_Target\
		{\
		return float4(shadowColor.xyz, RT.Sample(RTState, iTexCoord).w);\
		}";


	 String ShadowVolumeExtrudeProgram::mModulate_Vs_hlsl_4_0 =
		"	void ShadowBlend_vs\
		(\
		in float4 inPos : POSITION,\
		out float4 pos : SV_POSITION,\
		out float2 uv0 : TEXCOORD0,\
		uniform float4x4 worldViewProj\
		)\
		{\
		pos = mul(worldViewProj, inPos);\
		inPos.xy = sign(inPos.xy);\
		uv0 = (float2(inPos.x, -inPos.y) + 1.0f) * 0.5f;\
				}";

	 String ShadowVolumeExtrudeProgram::mModulate_Fs_cg =
		 "sampler2D RT : register(s0);\n"
		 "uniform float4 shadowColor; \n"
		 "float4 ShadowBlend_ps(float2 iTexCoord : TEXCOORD0) : COLOR\n"
		 "{\n"
		 "return float4(shadowColor.xyz, tex2D(RT, iTexCoord).w);\n"
		"}\n";

	 String ShadowVolumeExtrudeProgram::mModulate_Vs_cg =
			"void ShadowBlend_vs\
			(\
			in float4 inPos : POSITION,\
			out float4 pos : POSITION,\
			out float2 uv0 : TEXCOORD0,\
			uniform float4x4 worldViewProj\
			)\
			{\
				pos = mul(worldViewProj, inPos);\
				inPos.xy = sign(inPos.xy);\
				uv0 = (float2(inPos.x, -inPos.y) + 1.0f) * 0.5f;\
			}";



static const String glsles_prefix = "precision highp float;\
                                    precision highp int; \
                                    precision lowp sampler2D; \
                                    precision lowp samplerCube;";

    String ShadowVolumeExtrudeProgram::mModulate_Fs_glsl =
        "uniform sampler2D RT; \
        uniform vec4 shadowColor; \
        varying vec2 uv0; \
        \
        void main() {\
            gl_FragColor = vec4(shadowColor.xyz, texture2D(RT, uv0).w);\
        }";

    String ShadowVolumeExtrudeProgram::mModulate_Vs_glsl =
        "uniform mat4 worldViewProj; \
        attribute vec4 inPos; \
        varying vec4 uv0; \
        \
        void main() {\
            gl_Position = worldViewProj*inPos; \
            inPos.xy = sign(inPos.xy); \
            uv0 = (vec2(inPos.x, -inPos.y) + 1.0f) * 0.5f; \
        }";




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
	// programs for Ogre/StencilShadowModulationPass material
	//---------------------------------------------------------------------
	const String gShadowModulativePassVs_Name = "Ogre/StencilShadowModulationPassVs"; // FIXME: unused?!
	const String gShadowModulativePassPs_Name = "Ogre/StencilShadowModulationPassPs"; // FIXME: unused?!

	const String gShadowModulativePassVs_4_0 = // FIXME: unused?!
		"float4x4	worldviewproj_matrix;\n"
		"void vs_main(in float4 iPos : POSITION, out float4 oPos : SV_Position)\n"
		"{\n"
		"    oPos = mul(worldviewproj_matrix, iPos);\n"
		"}\n";

	const String gShadowModulativePassPs_4_0 = // FIXME: unused?!
		"float4 ambient_light_colour;\n"
		"void fs_main(in float4 oPos : SV_Position, out float4 oColor : SV_Target)\n"
		"{\n"
		"    oColor = ambient_light_colour;\n"
		"}\n";

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

	void ShadowVolumeExtrudeProgram::AddInternalProgram(String name, String source, String language, String entryPoint, String target, GpuProgramType type)
	{
		HighLevelGpuProgramPtr program = HighLevelGpuProgramManager::getSingleton()
			.createProgram(name, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, language, type);

		program->setSource(source);
		program->setParameter("entry_point", entryPoint);
		if (language == "cg")
			program->setParameter("profiles", target);
		else
			program->setParameter("target", target);

		program->load();
	}

	void ShadowVolumeExtrudeProgram::initialiseModulationPassPrograms(void)
	{
		bool vs_4_0 = GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0");
		bool ps_4_0 = GpuProgramManager::getSingleton().isSyntaxSupported("ps_4_0");
		bool glsl = GpuProgramManager::getSingleton().isSyntaxSupported("glsl");
		bool glsles = GpuProgramManager::getSingleton().isSyntaxSupported("glsles");

		const String vsProgramName = "Ogre/ShadowBlendVP";
		const String fsProgramName = "Ogre/ShadowBlendFP";

		const String vsEntryPoint = "ShadowBlend_vs";
		const String fsEntryPoint = "ShadowBlend_ps";

		String language;
		String fsTarget;
		String vsTarget;
		String vsProgram;
		String fsProgram;

		if (glsl)
		{
			vsTarget = "glsl";
			fsTarget = "glsl";
			language = "glsl";
			vsProgram = mModulate_Vs_glsl;
			fsProgram = mModulate_Fs_glsl;
		}
		else if (glsles)
		{
            vsTarget = "glsles";
            fsTarget = "glsles";
            language = "glsles";
            vsProgram = glsles_prefix+mModulate_Vs_glsl;
            fsProgram = glsles_prefix+mModulate_Fs_glsl;
		}
		else
		{
			if (ps_4_0 && vs_4_0)
			{
				vsTarget = "vs_4_0";
				fsTarget = "ps_4_0";
				language = "hlsl";
				vsProgram = mModulate_Vs_hlsl_4_0;
				fsProgram = mModulate_Fs_hlsl_4_0;
			}
			else
			{
				vsTarget = "vs_2_0";
				fsTarget = "ps_2_0";
				language = "cg";
				vsProgram = mModulate_Vs_cg;
				fsProgram = mModulate_Fs_cg;
			}
		}

		//Add vertex program
		AddInternalProgram(vsProgramName, vsProgram, language, vsEntryPoint, vsTarget, GPT_VERTEX_PROGRAM);

		//Add fragment program
		AddInternalProgram(fsProgramName, fsProgram, language, fsEntryPoint, fsTarget, GPT_FRAGMENT_PROGRAM);
	}

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
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0")
                  || GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1")
                  || GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_3"))
			{
				syntax = "vs_4_0";
			}
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
			{
				syntax = "glsles";
			}
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
			{
				syntax = "glsl";
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
					"Vertex programs are supposedly supported, but neither "
					"arbvp1, glsl, glsles, vs_1_1 nor vs_4_0 syntaxes are present.",
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

						vp->setParameter("target", "vs_4_0_level_9_1"); // shared subset, to be usable from microcode cache on all devices
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
							fp->setParameter("target", "ps_4_0_level_9_1"); // shared subset, to be usable from microcode cache on all devices
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
					else if (syntax == "glsl")
					{
						HighLevelGpuProgramPtr vp =
							HighLevelGpuProgramManager::getSingleton().createProgram(
							programNames[v], ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
							"glsl", GPT_VERTEX_PROGRAM);
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
								"glsl", GPT_FRAGMENT_PROGRAM);
							fp->setSource(mGeneralFs_glsl);
							fp->setParameter("target", "glsl");
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

			initialiseModulationPassPrograms();
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
			frgProgramName = "";
        }
    }
    //---------------------------------------------------------------------
    const String& ShadowVolumeExtrudeProgram::getProgramSource(
        Light::LightTypes lightType, const String &syntax, bool finite, bool debug)
    {
        if (lightType == Light::LT_DIRECTIONAL)
        {
            if (syntax == "arbvp1")
            {
                if (finite)
                {
                    return debug ? getDirectionalLightExtruderArbvp1FiniteDebug() : getDirectionalLightExtruderArbvp1Finite();
                }
                else
                {
                    return debug ? getDirectionalLightExtruderArbvp1Debug() : getDirectionalLightExtruderArbvp1();
                }
            } 
            else if (syntax == "vs_1_1")
            {
                if (finite)
                {
                    return debug ? getDirectionalLightExtruderVs_1_1FiniteDebug() : getDirectionalLightExtruderVs_1_1Finite();
                }
                else
                {
                    return debug ? getDirectionalLightExtruderVs_1_1Debug() : getDirectionalLightExtruderVs_1_1();
                }
            }
            else if (syntax == "vs_4_0")
            {
                if (finite)
                {
					return debug ? getDirectionalLightExtruderVs_4_0FiniteDebug() : getDirectionalLightExtruderVs_4_0Finite();
				}
				else
				{
					return debug ? getDirectionalLightExtruderVs_4_0Debug() : getDirectionalLightExtruderVs_4_0();
				}
			}
            else if (syntax == "glsl")
            {
                if (finite)
                {
                    return debug ? getDirectionalLightExtruderVs_glslFiniteDebug() : getDirectionalLightExtruderVs_glslFinite();
                }
                else
                {
                    return debug ? getDirectionalLightExtruderVs_glslDebug() : getDirectionalLightExtruderVs_glsl();
                }
            }
            else if (syntax == "glsles")
            {
                if (finite)
                {
                    return debug ? getDirectionalLightExtruderVs_glslesFiniteDebug() : getDirectionalLightExtruderVs_glslesFinite();
                }
                else
                {
                    return debug ? getDirectionalLightExtruderVs_glslesDebug() : getDirectionalLightExtruderVs_glsles();
                }
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                    "Vertex programs are supposedly supported, but neither "
                    "arbvp1, glsl, glsles, vs_1_1 nor vs_4_0 syntaxes are present.", 
                    "SceneManager::getProgramSource");
            }
        }
        else
        {
            if (syntax == "arbvp1")
            {
                if (finite)
                {
                    return debug ? getPointLightExtruderArbvp1FiniteDebug() : getPointLightExtruderArbvp1Finite();
                }
                else
                {
                    return debug ? getPointLightExtruderArbvp1Debug() : getPointLightExtruderArbvp1();
                }
            }
            else if (syntax == "vs_1_1")
            {
                if (finite)
                {
                    return debug ? getPointLightExtruderVs_1_1FiniteDebug() : getPointLightExtruderVs_1_1Finite();
                }
                else
                {
                    return debug ? getPointLightExtruderVs_1_1Debug() : getPointLightExtruderVs_1_1();
                }
            }
            else if (syntax == "vs_4_0")
            {
                if (finite)
                {
					return debug ? getPointLightExtruderVs_4_0FiniteDebug() : getPointLightExtruderVs_4_0Finite();
				}
				else
				{
					return debug ? getPointLightExtruderVs_4_0Debug() : getPointLightExtruderVs_4_0();
				}
			}
            else if (syntax == "glsl")
            {
                if (finite)
                {
					return debug ? getPointLightExtruderVs_glslFiniteDebug() : getPointLightExtruderVs_glslFinite();
				}
				else
				{
					return debug ? getPointLightExtruderVs_glslDebug() : getPointLightExtruderVs_glsl();
				}
			}
            else if (syntax == "glsles")
            {
                if (finite)
                {
					return debug ? getPointLightExtruderVs_glslesFiniteDebug() : getPointLightExtruderVs_glslesFinite();
				}
				else
				{
					return debug ? getPointLightExtruderVs_glslesDebug() : getPointLightExtruderVs_glsles();
				}
			}
            else
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                    "Vertex programs are supposedly supported, but neither "
                    "arbvp1, glsl, glsles, vs_1_1 nor vs_4_0 syntaxes are present.", 
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
                return programNames[debug ? DIRECTIONAL_LIGHT_FINITE_DEBUG : DIRECTIONAL_LIGHT_FINITE];
            }
            else
            {
                return programNames[debug ? DIRECTIONAL_LIGHT_DEBUG : DIRECTIONAL_LIGHT];
            }
        }
        else
        {
            if (finite)
            {
                return programNames[debug ? POINT_LIGHT_FINITE_DEBUG : POINT_LIGHT_FINITE];
            }
            else
            {
                return programNames[debug ? POINT_LIGHT_DEBUG : POINT_LIGHT];
            }
        }
    }

}
