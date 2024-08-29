// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

// greatly inspired by
// - shiny: https://ogrecave.github.io/shiny/defining-materials-shaders.html
// - bgfx: https://github.com/bkaradzic/bgfx/blob/master/src/bgfx_shader.sh

/// general usage:
// MAIN_PARAMETERS
// IN(vec4 vertex, POSITION)
// MAIN_DECLARATION
// {
//     GLSL code here
// }

/// configuration
// use macros that will be default with Ogre 15
// #define USE_OGRE_FROM_FUTURE

// @public-api

#if defined(OGRE_HLSL) || defined(OGRE_CG)
// HLSL
#include "HLSL_SM4Support.hlsl"
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define mat3 float3x3
#define mat4 float4x4

#define ivec2 int2
#define ivec3 int3
#define ivec4 int4

#define texture1D tex1D
#define texture2D tex2D
#define texture3D tex3D
#define texture2DArray tex2DARRAY
#define textureCube texCUBE
#define shadow2D tex2Dcmp
#define texture2DProj tex2Dproj
vec4 texture2DLod(sampler2D s, vec2 v, float lod) { return tex2Dlod(s, vec4(v.x, v.y, 0, lod)); }

#define samplerCube samplerCUBE
vec4 textureCubeLod(samplerCube s, vec3 v, float lod) { return texCUBElod(s, vec4(v.x, v.y, v.z, lod)); }

#define sampler2DShadow Sampler2DShadow

#define mix lerp
#define fract frac
#define inversesqrt rsqrt
#define dFdx ddx
#define dFdy ddy

float mod(float _a, float _b) { return _a - _b * floor(_a / _b); }
vec2  mod(vec2  _a, vec2  _b) { return _a - _b * floor(_a / _b); }
vec3  mod(vec3  _a, vec3  _b) { return _a - _b * floor(_a / _b); }
vec4  mod(vec4  _a, vec4  _b) { return _a - _b * floor(_a / _b); }

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

#define OGRE_UNIFORMS_BEGIN
#define OGRE_UNIFORMS_END

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

#ifdef VULKAN
#define _UNIFORM_BINDING(b) layout(binding = b + 2) uniform
#elif __VERSION__ >= 420
#define _UNIFORM_BINDING(b) layout(binding = b) uniform
#else
#define _UNIFORM_BINDING(b) uniform
#endif

#define SAMPLER1D(name, reg) _UNIFORM_BINDING(reg) sampler1D name
#define SAMPLER2D(name, reg) _UNIFORM_BINDING(reg) sampler2D name
#define SAMPLER3D(name, reg) _UNIFORM_BINDING(reg) sampler3D name
#define SAMPLER2DARRAY(name, reg) _UNIFORM_BINDING(reg) sampler2DArray name
#define SAMPLERCUBE(name, reg) _UNIFORM_BINDING(reg) samplerCube name
#define SAMPLER2DSHADOW(name, reg) _UNIFORM_BINDING(reg) sampler2DShadow name

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

#if !defined(OGRE_HLSL) && !defined(OGRE_CG)
// semantics as aliases for attribute locations
#define POSITION    0
#define BLENDWEIGHT 1
#define NORMAL      2
#define COLOR0      3
#define COLOR1      4
#define COLOR COLOR0
#define FOG         5
#define BLENDINDICES 7
#define TEXCOORD0   8
#define TEXCOORD1   9
#define TEXCOORD2  10
#define TEXCOORD3  11
#define TEXCOORD4  12
#define TEXCOORD5  13
#define TEXCOORD6  14
#define TEXCOORD7  15
#define TANGENT    14
#endif

#define OGRE_UNIFORMS(params) OGRE_UNIFORMS_BEGIN params OGRE_UNIFORMS_END

// GL_EXT_shader_explicit_arithmetic_types polyfill
#ifdef OGRE_GLSLES
#define float32_t highp float
#define f32vec2 highp vec2
#define f32vec3 highp vec3
#define f32vec4 highp vec4
#else
#define float32_t float
#define f32vec2 vec2
#define f32vec3 vec3
#define f32vec4 vec4
#endif
