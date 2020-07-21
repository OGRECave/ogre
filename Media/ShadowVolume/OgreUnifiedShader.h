// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

// greatly inspired by
// - bgfx: https://github.com/bkaradzic/bgfx/blob/master/src/bgfx_shader.sh
// - shiny: https://ogrecave.github.io/shiny/defining-materials-shaders.html

#if defined(OGRE_HLSL) || defined(OGRE_CG)
// HLSL
#include "HLSL_SM4Support.hlsl"
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define mat3 float3x3
#define mat4 float4x4

#define texture2D tex2D
#define texture3D tex3D
#define textureCube texCUBE
#define shadow2D tex2Dcmp

#define samplerCube samplerCUBE
#define sampler2DShadow Sampler2DShadow

#define mix lerp

mat4 mtxFromRows(vec4 a, vec4 b, vec4 c, vec4 d)
{
    return mat4(a, b, c, d);
}

mat3 mtxFromRows(vec3 a, vec3 b, vec3 c)
{
    return mat3(a, b, c);
}

#define STATIC static

#define ATTRIBUTES_BEGIN void main(

#ifdef OGRE_VERTEX_SHADER
#define MAIN_DECLARATION out float4 gl_Position : POSITION)
#else
#define MAIN_DECLARATION out float4 gl_FragColor : COLOR)
#endif

#define ATTRIBUTE(decl, sem) in decl : sem,
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

mat4 mtxFromRows(vec4 a, vec4 b, vec4 c, vec4 d)
{
    return transpose(mat4(a, b, c, d));
}

mat3 mtxFromRows(vec3 a, vec3 b, vec3 c)
{
    return transpose(mat3(a, b, c));
}

#define STATIC

#define ATTRIBUTES_BEGIN
#define MAIN_DECLARATION void main()

#define ATTRIBUTE(decl, sem) attribute decl;
#endif
