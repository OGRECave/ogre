Texture2D<float3> RT		: register(t0);
SamplerState samplerState	: register(s0);

float4 main( float2 uv0 : TEXCOORD0 ) : SV_Target
{
	float greyscale = dot( RT.Sample(samplerState, uv0).xyz, float3(0.3, 0.59, 0.11) );
	return float4(greyscale.xxx, 1.0);
}
