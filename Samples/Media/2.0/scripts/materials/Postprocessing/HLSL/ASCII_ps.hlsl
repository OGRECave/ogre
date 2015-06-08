Texture2D RT					: register(t0);
Texture3D pattern				: register(t1);
SamplerState samplerState[2]	: register(s0);

float4 main
(
	float2 TexCoord : TEXCOORD0,
	uniform float2 numTiles,
	uniform float2 iNumTiles,
	uniform float2 iNumTiles2,
	uniform float4 lum,
	uniform float charBias
) : SV_Target
{
    float3 local;

	//sample RT
	local.xy = fmod(TexCoord, iNumTiles);
	float2 middle = TexCoord - local.xy;
	local.xy = local.xy * numTiles;
	
	//iNumTiles2 = iNumTiles / 2
	middle = middle + iNumTiles2;
	float4 c = RT.Sample( samplerState[0], middle );
	
	//multiply luminance by charbias , beacause not all slices of the ascii
	//volume texture are used
	local.z = dot(c , lum)*charBias;
	
	//fix to brighten the dark pixels with small characters
	//c *= lerp(2.0,1.0, local.z);
	
	c *= pattern.Sample( samplerState[1],local );
	return c;
}
