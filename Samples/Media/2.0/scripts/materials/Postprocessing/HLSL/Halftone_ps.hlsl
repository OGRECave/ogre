Texture2D RT					: register(t0);
Texture3D<float> pattern		: register(t1);
SamplerState samplerState[2]	: register(s0);

float4 main
(
	float2 TexCoord : TEXCOORD0,
	uniform float2 numTiles,
	uniform float2 iNumTiles,
	uniform float2 iNumTiles2,
	uniform float4 lum
) : SV_Target
{	
	float3 local;
	local.xy = fmod(TexCoord, iNumTiles);
	float2 middle = TexCoord - local.xy;
	local.xy = local.xy * numTiles;
	middle +=  iNumTiles2;
	local.z = dot(RT.Sample(samplerState[0], middle ) , lum);
	float4 c = pattern.Sample(samplerState[1],local).x;
	return c;
}
