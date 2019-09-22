// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#define sampler1D Texture1D
#define sampler2D Texture2D
#define sampler3D Texture3D
#define samplerCUBE TextureCube

SamplerState defaultSampler; // TODO: read the actual sampler states

float4 tex1D(Texture1D t, float v) { return t.Sample(defaultSampler, v); }
float4 tex2D(Texture2D t, float2 v) { return t.Sample(defaultSampler, v); }
float4 tex3D(Texture3D t, float3 v) { return t.Sample(defaultSampler, v); }
float4 texCUBE(TextureCube t, float3 v) { return t.Sample(defaultSampler, v); }

float4 tex2Dproj(Texture2D t, float4 v) { return t.Sample(defaultSampler, v.xy/v.w); }