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
	uniform float minLuminance,
	uniform float maxLuminance,
	uniform float4 tex0Size,
	uniform float timeSinceLast
) : SV_Target
{
	float fLumAvg = lumRt.Sample( samplerBilinear, uv + c_offsets[0] * tex0Size.zw ).x;

	for( int i=1; i<4; ++i )
		fLumAvg += lumRt.Sample( samplerBilinear, uv + c_offsets[i] * tex0Size.zw ).x;

	fLumAvg *= 0.25f; // /= 4.0f;

	//Save the expresion already inverted and non-zero,  executing it only once, instead of doing it every frame
	//when we use it
	float newLum = 1.0f / exp( clamp( fLumAvg, minLuminance, maxLuminance ) );
	float oldLum = oldLumRt.Sample( samplerPoint, float( 0.0 ).xx ).x;

	//Adapt luminicense based 95% per second.
	return lerp( newLum, oldLum, pow( 0.05f, timeSinceLast ) );
}
