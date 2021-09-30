// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

// greatly inspired by
// - shiny: https://ogrecave.github.io/shiny/defining-materials-shaders.html
// - bgfx: https://github.com/bkaradzic/bgfx/blob/master/src/bgfx_shader.sh

// general usage:
// MAIN_PARAMETERS
// IN(vec4 vertex, POSITION)
// MAIN_DECLARATION
// {
//     GLSL code here
// }

// @public-api

#if defined(OGRE_HLSL) || defined(OGRE_CG)
// HLSL
#include "HLSL_SM4Support.hlsl"
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define mat3 float3x3
#define mat4 float4x4

#define texture1D tex1D
#define texture2D tex2D
#define texture3D tex3D
#define texture2DArray tex2DARRAY
#define textureCube texCUBE
#define shadow2D tex2Dcmp
#define texture2DProj tex2Dproj
vec4 texture2DLod(sampler2D s, vec2 v, float lod) { return tex2Dlod(s, vec4(v.x, v.y, 0, lod)); }

#define samplerCube samplerCUBE
#define sampler2DShadow Sampler2DShadow

#define mix lerp
#define fract frac

vec2 vec2_splat(float x) { return vec2(x, x); }
vec3 vec3_splat(float x) { return vec3(x, x, x); }
vec4 vec4_splat(float x) { return vec4(x, x, x, x); }

mat4 mtxFromRows(vec4 a, vec4 b, vec4 c, vec4 d)
{
    return mat4(a, b, c, d);
}

mat3 mtxFromRows(vec3 a, vec3 b, vec3 c)
{
    return mat3(a, b, c);
}

mat3 mtxFromCols(vec3 a, vec3 b, vec3 c)
{
    return transpose(mat3(a, b, c));
}

#define STATIC static

#define MAIN_PARAMETERS void main(

#ifdef OGRE_VERTEX_SHADER
#define MAIN_DECLARATION out float4 gl_Position : POSITION)
#else
#define MAIN_DECLARATION in float4 gl_FragCoord : POSITION, out float4 gl_FragColor : COLOR)
#endif

#define IN(decl, sem) in decl : sem,
#define OUT(decl, sem) out decl : sem,
#elif defined(OGRE_METAL)
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define mat3 metal::float3x3
#define mat4 metal::float4x4

#define IN(decl, sem) decl [[ attribute(sem) ]];
#else
// GLSL
#include "GLSL_GL3Support.glsl"

#define SAMPLER1D(name, reg) sampler1D name
#define SAMPLER2D(name, reg) sampler2D name
#define SAMPLER3D(name, reg) sampler3D name
#define SAMPLER2DARRAY(name, reg) sampler2DArray name
#define SAMPLERCUBE(name, reg) samplerCube name

#define saturate(x) clamp(x, 0.0, 1.0)
#define mul(a, b) ((a) * (b))

#define vec2_splat vec2
#define vec3_splat vec3
#define vec4_splat vec4

mat4 mtxFromRows(vec4 a, vec4 b, vec4 c, vec4 d)
{
    return transpose(mat4(a, b, c, d));
}

mat3 mtxFromRows(vec3 a, vec3 b, vec3 c)
{
    return transpose(mat3(a, b, c));
}

mat3 mtxFromCols(vec3 a, vec3 b, vec3 c)
{
    return mat3(a, b, c);
}

#define STATIC

#define MAIN_PARAMETERS
#define MAIN_DECLARATION void main()

#endif

#if defined(OGRE_METAL) || defined(OGRE_GLSLANG)
// semantics as aliases for attribute locations
#define POSITION    0
#define BLENDWEIGHT 1
#define NORMAL      2
#define COLOR0      3
#define COLOR1      4
#define COLOR COLOR0
#define BLENDIDICES 7
#define TEXCOORD0   8
#define TEXCOORD1   9
#define TEXCOORD2  10
#define TEXCOORD3  11
#define TANGENT    15
#endif