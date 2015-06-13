//---------------------------------------------
// Bloom

Texture2D RT		: register(t0);
Texture2D Blur1		: register(t1);

SamplerState samplerState	: register(s0);

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

float4 main
(
	PS_INPUT inPs,
	uniform float OriginalImageWeight,
	uniform float BlurWeight
) : SV_Target
{
	float4 sharp = RT.Sample( samplerState,		inPs.uv0 );
	float4 blur  = Blur1.Sample( samplerState,	inPs.uv0 );

	return blur*BlurWeight + sharp*OriginalImageWeight;
}
