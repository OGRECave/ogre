Texture2D rt0				: register(t0);
SamplerState samplerState	: register(s0);

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

#define NUM_SAMPLES 7

float4 main
(
	PS_INPUT inPs,

	uniform float4	centerUVPos, //z = min, w = 1 / (max - min)
	uniform float	exponent

) : SV_Target
{
	const float c_multipliers[NUM_SAMPLES] =
	{
		1.00f,
		0.99f,
		0.98f,
		0.97f,
		0.96f,
		0.94f,
		0.93f
	};

	float atten = ( distance( inPs.uv0, centerUVPos.xy ) - centerUVPos.z ) * centerUVPos.w;
	atten = 1.0f - saturate( atten );
	atten = pow( atten, exponent );

	float4 originalColour = rt0.Sample( samplerState, (inPs.uv0 - centerUVPos.xy) *
														c_multipliers[0] + centerUVPos.xy );

	float4 sumColours = float4( 0.0, 0.0, 0.0, 0.0 );
	for( int i=1; i<NUM_SAMPLES; ++i )
	{
		sumColours += rt0.Sample( samplerState, (inPs.uv0 - centerUVPos.xy) *
												c_multipliers[i] + centerUVPos.xy );
	}

	//'originalColour' has always the same strength. 'sumColours' gets weaker over distance (past min distance).
	//The denominator is altered according to the distance, so that the average is always consistent.
	return ( originalColour + sumColours * atten ) / (1 + (NUM_SAMPLES-1) * atten );
}
