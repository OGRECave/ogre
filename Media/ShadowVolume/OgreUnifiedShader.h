// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

// greatly inspired by
// - bgfx: https://github.com/bkaradzic/bgfx/blob/master/src/bgfx_shader.sh
// - shiny: https://ogrecave.github.io/shiny/defining-materials-shaders.html

#if defined(OGRE_HLSL) || defined(OGRE_CG)
// HLSL
#include "HLSL_SM4Support.hlsl"
#define vec3 float3
#define vec4 float4
#define mat4 float4x4
#define texture2D tex2D
#define texture3D tex3D

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

#define mul(a, b) ((a) * (b))

#define ATTRIBUTES_BEGIN
#define MAIN_DECLARATION void main()

#define ATTRIBUTE(decl, sem) attribute decl;
#endif
