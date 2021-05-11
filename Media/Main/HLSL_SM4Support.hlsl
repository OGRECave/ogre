// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#if OGRE_HLSL >= 4

// SM4 separates sampler into Texture and SamplerState

#define sampler1D Sampler1D
#define sampler2D Sampler2D
#define sampler3D Sampler3D
#define samplerCUBE SamplerCube

struct Sampler1D
{
    Texture1D t;
    SamplerState s;
};
struct Sampler2D
{
    Texture2D t;
    SamplerState s;
};
struct Sampler3D
{
    Texture3D t;
    SamplerState s;
};
struct SamplerCube
{
    TextureCube t;
    SamplerState s;
};

float4 tex1D(Sampler1D s, float v) { return s.t.Sample(s.s, v); }
float4 tex2D(Sampler2D s, float2 v) { return s.t.Sample(s.s, v); }
float4 tex3D(Sampler3D s, float3 v) { return s.t.Sample(s.s, v); }
float4 texCUBE(SamplerCube s, float3 v) { return s.t.Sample(s.s, v); }

float4 tex2D(Sampler2D s, float2 v, float2 ddx, float2 ddy) { return s.t.SampleGrad(s.s, v, ddx, ddy); }
float4 tex2Dproj(Sampler2D s, float4 v) { return s.t.Sample(s.s, v.xy/v.w); }
float4 tex2Dlod(Sampler2D s, float4 v) { return s.t.SampleLevel(s.s, v.xy, v.w); }

#define SAMPLER1D(name, reg) \
    Texture1D name ## Tex : register(t ## reg);\
    SamplerState name ## State : register(s ## reg);\
    static Sampler1D name = {name ## Tex, name ## State}

#define SAMPLER2D(name, reg) \
    Texture2D name ## Tex : register(t ## reg);\
    SamplerState name ## State : register(s ## reg);\
    static Sampler2D name = {name ## Tex, name ## State}

#define SAMPLER3D(name, reg) \
    Texture3D name ## Tex : register(t ## reg);\
    SamplerState name ## State : register(s ## reg);\
    static Sampler3D name = {name ## Tex, name ## State}

#define SAMPLERCUBE(name, reg) \
    TextureCube name ## Tex : register(t ## reg);\
    SamplerState name ## State : register(s ## reg);\
    static SamplerCube name = {name ## Tex, name ## State}

// the following are not available in D3D9, but provided for convenience
struct Sampler2DShadow
{
    Texture2D t;
    SamplerComparisonState s;
};
struct Sampler2DArray
{
    Texture2DArray t;
    SamplerState s;
};

#define SAMPLER2DSHADOW(name, reg) \
    Texture2D name ## Tex : register(t ## reg);\
    SamplerComparisonState name ## State : register(s ## reg);\
    static Sampler2DShadow name = {name ## Tex, name ## State}

#define SAMPLER2DARRAY(name, reg) \
    Texture2DArray name ## Tex : register(t ## reg);\
    SamplerState name ## State : register(s ## reg);\
    static Sampler2DShadow name = {name ## Tex, name ## State}

float tex2Dcmp(Sampler2DShadow s, float3 v) { return s.t.SampleCmpLevelZero(s.s, v.xy, v.z); }
float4 tex2DARRAY(Sampler2DArray s, float3 v) { return s.t.Sample(s.s, v); }
#else

#define SAMPLER1D(name, reg) sampler1D name : register(s ## reg)
#define SAMPLER2D(name, reg) sampler2D name : register(s ## reg)
#define SAMPLER3D(name, reg) sampler3D name : register(s ## reg)
#define SAMPLERCUBE(name, reg) samplerCUBE name : register(s ## reg)

#endif