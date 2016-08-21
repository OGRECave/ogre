struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;

};

Texture2D<float> ssaoTexture	: register(t0);
Texture2D<float4> sceneTexture	: register(t1);

SamplerState samplerState0		: register(s0);

uniform float powerScale;

float4 main
(
	PS_INPUT inPs
) : SV_Target
{
	float ssao = ssaoTexture.Sample(samplerState0, inPs.uv0);
	
	ssao = saturate( pow(ssao, powerScale) );

	float4 col = sceneTexture.Sample(samplerState0, inPs.uv0);
	//return float4(ssao, ssao, ssao, col.w);
	return float4( col.xyz * ssao, col.w );
}
