/*
----------------------------------------------------------
Terrain Tessellation sample from NVIDIA's DirectX 11 SDK:
http://developer.nvidia.com/nvidia-graphics-sdk-11-direct3d
----------------------------------------------------------
*/

#ifndef INCLUDED_COMMON_HLSL
#define INCLUDED_COMMON_HLSL

static const int CONTROL_VTX_PER_TILE_EDGE = 9;
static const int PATCHES_PER_TILE_EDGE = 8;
static const float RECIP_CONTROL_VTX_PER_TILE_EDGE = 1.0 / 9;
static const float WORLD_SCALE = 400;
static const float VERTICAL_SCALE = 0.65;
static const float WORLD_UV_REPEATS = 8;	// How many UV repeats across the world for fractal generation.
static const float WORLD_UV_REPEATS_RECIP = 1.0 / WORLD_UV_REPEATS;

//int3   g_FractalOctaves;		// ridge, fBm, uv twist
//float3 g_TextureWorldOffset;	// Offset of fractal terrain in texture space.
//float  g_CoarseSampleSpacing;	// World space distance between samples in the coarse height map.

struct Adjacency
{
	// These are the size of the neighbours along +/- x or y axes.  For interior tiles
	// this is 1.  For edge tiles it is 0.5 or 2.0.
	float neighbourMinusX : ADJACENCY_SIZES0;
	float neighbourMinusY : ADJACENCY_SIZES1;
	float neighbourPlusX  : ADJACENCY_SIZES2;
	float neighbourPlusY  : ADJACENCY_SIZES3;
};

struct AppVertexTessellation
{
	float2 position  : POSITION_2D;
	Adjacency adjacency;
    uint VertexId    : SV_VertexID;
    uint InstanceId  : SV_InstanceID;
};

struct AppVertex
{
	float2 position  : POSITION_2D;
	Adjacency adjacency;
    uint VertexId    : SV_VertexID;
    uint InstanceId  : SV_InstanceID;
};

SamplerState SamplerRepeatMaxAniso
{
    Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState SamplerRepeatMedAniso
{
    Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState SamplerRepeatLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState SamplerClampLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
}; 

SamplerState SamplerRepeatPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

#endif
