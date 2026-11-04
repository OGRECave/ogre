// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

// cross-language macros for declaring compute shader resources on D3D11, GL and Vulkan
// can be used alongside OgreUnifiedShader.h, which maps the GLSL types and functions to HLSL

/// general usage:
// writeonly IMAGE2D(rgba8, image, 0);
// OGRE_COMPUTE_UNIFORMS(
// uniform float time;
// )
// MAIN_COMPUTE(16, 16, 1)
// {
//     imageStore(image, ivec2(gl_GlobalInvocationID.xy), vec4(1.0, 0.0, 0.0, 1.0));
// }

/// bindings
// - OGRE_COMPUTE_UNIFORMS: the parameters of the compute_program_ref
// - IMAGE*(format, name, reg): reg is the index of the texture_unit, that has unordered_access_mip set
//   prefix with readonly or writeonly as in GLSL. They are ignored in HLSL.
//   format is the GLSL image format qualifier. Only floating point formats are supported.
//   imageLoad returns a vec4 in GLSL, but the type of the format in HLSL
// - TEXTURE*(name, reg): reg is the index of the texture_unit, that is sampled
//   use textureLod and texelFetch to read it
// - BUFFER(block, type, name, reg): an array of type, backed by the shared_params_ref with index reg
//   prefix with readonly or writeonly as in GLSL
//   on GL, the shared parameters are looked up by the block name, which is unused in HLSL

// @public-api

#if defined(OGRE_HLSL)
// HLSL

// map the image format to the element type. The prefix is needed, as the format names are no keywords in GLSL,
// so a plain #define would break shaders using them as identifiers, e.g. float r8
#define _IMAGE_TYPE_rgba32f float4
#define _IMAGE_TYPE_rgba16f float4
#define _IMAGE_TYPE_rgba16 float4
#define _IMAGE_TYPE_rgb10_a2 float4
#define _IMAGE_TYPE_rgba8 float4
#define _IMAGE_TYPE_rgba16_snorm float4
#define _IMAGE_TYPE_rgba8_snorm float4
#define _IMAGE_TYPE_r11f_g11f_b10f float3
#define _IMAGE_TYPE_rg32f float2
#define _IMAGE_TYPE_rg16f float2
#define _IMAGE_TYPE_rg16 float2
#define _IMAGE_TYPE_rg8 float2
#define _IMAGE_TYPE_r32f float
#define _IMAGE_TYPE_r16f float
#define _IMAGE_TYPE_r16 float
#define _IMAGE_TYPE_r8 float

// global uniforms end up in the default constant buffer
#define OGRE_COMPUTE_UNIFORMS(params) params

// unordered access views are always read-write in D3D11
#define readonly
#define writeonly

#define _IMAGE(type, name, format, reg) type<_IMAGE_TYPE_ ## format> name : register(u ## reg)

#define IMAGE2D(format, name, reg) _IMAGE(RWTexture2D, name, format, reg)
#define IMAGE2DARRAY(format, name, reg) _IMAGE(RWTexture2DArray, name, format, reg)
#define IMAGE3D(format, name, reg) _IMAGE(RWTexture3D, name, format, reg)

#define imageLoad(image, coord) image[coord]
#define imageStore(image, coord, value) image[coord] = value

#define _TEXTURE(type, name, reg)                                                                                      \
    type name ## Tex : register(t ## reg);                                                                            \
    SamplerState name ## State : register(s ## reg)

#define TEXTURE2D(name, reg) _TEXTURE(Texture2D, name, reg)
#define TEXTURE2DARRAY(name, reg) _TEXTURE(Texture2DArray, name, reg)
#define TEXTURE3D(name, reg) _TEXTURE(Texture3D, name, reg)

#define BUFFER(block, type, name, reg) RWStructuredBuffer<type> name : register(u ## reg)

#define textureLod(name, coord, lod) name ## Tex.SampleLevel(name ## State, coord, lod)
#define texelFetch(name, coord, lod) name ## Tex.Load(int3(coord, lod))

#define MAIN_COMPUTE(x, y, z)                                                                                          \
    [numthreads(x, y, z)] void main(uint3 gl_WorkGroupID : SV_GroupID, uint3 gl_LocalInvocationID : SV_GroupThreadID, \
                                    uint3 gl_GlobalInvocationID : SV_DispatchThreadID,                                \
                                    uint gl_LocalInvocationIndex : SV_GroupIndex)

#else
// GLSL

#ifdef VULKAN
// see the compute descriptor set in OgreVulkanRenderSystem.cpp
#define _IMAGE_BINDING(reg) (1 + reg)
#define _TEXTURE_BINDING(reg) (9 + reg)
#define _BUFFER_BINDING(reg) (25 + reg)
#else
#define _IMAGE_BINDING(reg) reg
#define _TEXTURE_BINDING(reg) reg
#define _BUFFER_BINDING(reg) reg
#endif

#ifdef OGRE_GLSLANG
#define OGRE_COMPUTE_UNIFORMS(params) layout(binding = 0, row_major) uniform OgreUniforms { params };
#else
#define OGRE_COMPUTE_UNIFORMS(params) params
#endif

#define _IMAGE(type, name, format, reg) layout(binding = _IMAGE_BINDING(reg), format) uniform type name

#define IMAGE2D(format, name, reg) _IMAGE(image2D, name, format, reg)
#define IMAGE2DARRAY(format, name, reg) _IMAGE(image2DArray, name, format, reg)
#define IMAGE3D(format, name, reg) _IMAGE(image3D, name, format, reg)

#define TEXTURE2D(name, reg) layout(binding = _TEXTURE_BINDING(reg)) uniform sampler2D name
#define TEXTURE2DARRAY(name, reg) layout(binding = _TEXTURE_BINDING(reg)) uniform sampler2DArray name
#define TEXTURE3D(name, reg) layout(binding = _TEXTURE_BINDING(reg)) uniform sampler3D name

#define BUFFER(block, type, name, reg) layout(std430, binding = _BUFFER_BINDING(reg)) buffer block { type name[]; }

#define MAIN_COMPUTE(x, y, z) layout(local_size_x = x, local_size_y = y, local_size_z = z) in; void main()

#endif
