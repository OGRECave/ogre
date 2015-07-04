static const float2 c_offsets[4] =
{
	float2( -1.0, -1.0 ), float2( 1.0, -1.0 ),
	float2( -1.0,  1.0 ), float2( 1.0,  1.0 )
};

Texture2D<float> lumRt		: register(t0);
SamplerState samplerState	: register(s0);

float main
(
	in float2 uv : TEXCOORD0,
	uniform float4 tex0Size
) : SV_Target
{
	float fLumAvg = lumRt.Sample( samplerState, uv + c_offsets[0] * tex0Size.zw ).x;

	for( int i=1; i<4; ++i )
		fLumAvg += lumRt.Sample( samplerState, uv + c_offsets[i] * tex0Size.zw ).x;
		
	//fLumAvg *= 0.0625f; // /= 16.0f;
	fLumAvg *= 0.25f; // /= 4.0f;
	
	return fLumAvg;
}
