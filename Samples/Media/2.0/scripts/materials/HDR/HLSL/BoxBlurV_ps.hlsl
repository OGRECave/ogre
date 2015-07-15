
Texture2D<float3> Blur0		: register(t0);
SamplerState samplerState	: register(s0);

#define NUM_SAMPLES 11

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

float3 fromSRGB( float3 x )
{
	return x * x;
}
float3 toSRGB( float3 x )
{
	return sqrt( x );
}

float3 main
(
	PS_INPUT inPs,

	uniform float4 invTex0Size
) : SV_Target
{
	float2 uv = inPs.uv0.xy;

	uv.y -= invTex0Size.y * ((NUM_SAMPLES-1) * 0.5);
	float3 sum = fromSRGB( Blur0.Sample( samplerState, uv ).xyz );

	for( int i=1; i<NUM_SAMPLES; ++i )
	{
		uv.y += invTex0Size.y;
		sum += fromSRGB( Blur0.Sample( samplerState, uv ).xyz );
	}

	return toSRGB( sum / NUM_SAMPLES );
}
