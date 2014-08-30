/*
----------------------------------------------------------
Terrain Tessellation sample from NVIDIA's DirectX 11 SDK:
http://developer.nvidia.com/nvidia-graphics-sdk-11-direct3d
----------------------------------------------------------
*/

#include "TerrainTessellationCommon.hlsl"
#include "TerrainTessellationINoise.hlsl"

// z in both cases is the height scale.

// Null in that there is no associated VB set by the API.
struct NullVertex
{
    uint VertexId   : SV_VertexID;
    uint InstanceId : SV_InstanceID;
};

struct DeformVertex
{
    float4 pos : SV_Position;
    float2 texCoord : TEXCOORD1;
};

DeformVertex InitializationVS( NullVertex input,
							   uniform float3 g_DeformMin,
							   uniform float3 g_DeformMax)
{
    DeformVertex output = (DeformVertex) 0;
    
	if (input.VertexId == 0)
	{
	    output.pos = float4(g_DeformMin.x, g_DeformMin.y, 0, 1);
	    output.texCoord = float2(0,0);
	}
	else if (input.VertexId == 1)
	{
	    output.pos = float4(g_DeformMax.x, g_DeformMin.y, 0, 1);
	    output.texCoord = float2(1,0);
	}
	else if (input.VertexId == 2)
	{
	    output.pos = float4(g_DeformMin.x, g_DeformMax.y, 0, 1);
	    output.texCoord = float2(0,1);
	}
	else if (input.VertexId == 3)
	{
	    output.pos = float4(g_DeformMax.x, g_DeformMax.y, 0, 1);
	    output.texCoord = float2(1,1);
	}
    
    return output;
}

float3 debugCubes(float2 uv)
{
	const float HORIZ_SCALE = 4, VERT_SCALE = 1;
	uv *= HORIZ_SCALE;
	return VERT_SCALE * floor(fmod(uv.x, 2.0)) * floor(fmod(uv.y, 2.0));
}

float3 debugXRamps(float2 uv)
{
	const float HORIZ_SCALE = 2, VERT_SCALE = 1;
	uv *= HORIZ_SCALE;
	return VERT_SCALE * frac(uv.x);
}

float3 debugSineHills(float2 uv)
{
	const float HORIZ_SCALE = 2 * 3.14159, VERT_SCALE = 0.5;
	uv *= HORIZ_SCALE;
	//uv += 2.8;			// arbitrarily not centered - test asymetric fns.
	return VERT_SCALE * (sin(uv.x) + 1) * (sin(uv.y) + 1);
}

float3 debugFlat(float2 uv)
{
	const float VERT_SCALE = 0.1;
	return VERT_SCALE;
}

float4 InitializationPS( DeformVertex input,
	uniform int3 g_FractalOctaves,
	uniform float3 g_TextureWorldOffset ) : SV_Target
{
	const float2 uv = g_TextureWorldOffset.xz + WORLD_UV_REPEATS * input.texCoord;
	//return float4(debugXRamps(uv),  1);
	//return float4(debugFlat(uv),  1);
	//return float4(debugSineHills(uv),  1);
	//return float4(debugCubes(uv), 1);
	return hybridTerrain(uv, g_FractalOctaves);
}

Texture2D g_InputTexture;

float4 GradientPS( DeformVertex input ) : SV_Target
{
	input.texCoord.y = 1 - input.texCoord.y;
	float x0 = g_InputTexture.Sample(SamplerClampLinear, input.texCoord, int2( 1,0)).x;
	float x1 = g_InputTexture.Sample(SamplerClampLinear, input.texCoord, int2(-1,0)).x;
	float y0 = g_InputTexture.Sample(SamplerClampLinear, input.texCoord, int2(0, 1)).x;
	float y1 = g_InputTexture.Sample(SamplerClampLinear, input.texCoord, int2(0,-1)).x;
	return float4(x0-x1, y0-y1, 0,0);
}
