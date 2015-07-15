static const float2 c_offsets[4] =
{
	float2( -1.0, -1.0 ), float2( 1.0, -1.0 ),
	float2( -1.0,  1.0 ), float2( 1.0,  1.0 )
};

Texture2D<float> lumRt		: register(t0);
SamplerState samplerBilinear: register(s0);

Texture2D<float> oldLumRt	: register(t1);
SamplerState samplerPoint	: register(s1);

float4 main
(
	in float2 uv : TEXCOORD0,
	uniform float3 exposure,
	uniform float timeSinceLast,
	uniform float4 tex0Size
) : SV_Target
{
	float fLumAvg = lumRt.Sample( samplerBilinear, uv + c_offsets[0] * tex0Size.zw ).x;

	for( int i=1; i<4; ++i )
		fLumAvg += lumRt.Sample( samplerBilinear, uv + c_offsets[i] * tex0Size.zw ).x;

	fLumAvg *= 0.25f; // /= 4.0f;

	float newLum = exposure.x / exp( clamp( fLumAvg, exposure.y, exposure.z ) );
	float oldLum = oldLumRt.Sample( samplerPoint, float( 0.0 ).xx ).x;

	//Adapt luminicense based 75% per second.
	return lerp( newLum, oldLum, pow( 0.25f, timeSinceLast ) );
}
