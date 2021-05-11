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
#define textureCube texCUBE
#define shadow2D tex2Dcmp
#define texture2DProj tex2Dproj
vec4 texture2DLod(sampler2D s, vec2 v, float lod) { return tex2Dlod(s, vec4(v.x, v.y, 0, lod)); }

#define samplerCube samplerCUBE
#define sampler2DShadow Sampler2DShadow

#define mix lerp

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

// fake semantics for attribute locations
enum {
    POSITION = 0,
    BLENDWEIGHT,
    NORMAL,
    COLOR0,
    COLOR = COLOR0,
    COLOR1,
    BLENDIDICES = 7,
    TEXCOORD0,
    TEXCOORD1,
    TEXCOORD2,
    TEXCOORD3,
    TANGENT = 15
};

#define IN(decl, sem) decl [[ attribute(sem) ]];
#else
// GLSL
#define SAMPLER1D(name, reg) sampler1D name
#define SAMPLER2D(name, reg) sampler2D name
#define SAMPLER3D(name, reg) sampler3D name
#define SAMPLERCUBE(name, reg) samplerCube name

#define saturate(x) clamp(x, 0.0, 1.0)
#define mul(a, b) ((a) * (b))

#if __VERSION__ == 100
mat2 transpose(mat2 m)
{
  return mat2(m[0][0], m[1][0],
              m[0][1], m[1][1]);
}

mat3 transpose(mat3 m)
{
  return mat3(m[0][0], m[1][0], m[2][0],
              m[0][1], m[1][1], m[2][1],
              m[0][2], m[1][2], m[2][2]);
}

mat4 transpose(mat4 m)
{
  return mat4(m[0][0], m[1][0], m[2][0], m[3][0],
              m[0][1], m[1][1], m[2][1], m[3][1],
              m[0][2], m[1][2], m[2][2], m[3][2],
              m[0][3], m[1][3], m[2][3], m[3][3]);
}
#endif

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

#ifdef OGRE_VERTEX_SHADER
#define IN(decl, sem) attribute decl;
#define OUT(decl, sem) varying decl;
#else
#define IN(decl, sem) varying decl;
#define OUT(decl, sem) out decl;
#endif
#endif
