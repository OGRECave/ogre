Texture2D RT				: register(t0);
SamplerState samplerState	: register(s0);

float4 main( float2 uv0 : TEXCOORD0 ) : SV_Target
{
	float nColors = 8;
	float gamma = 0.6;

	float4 texCol = RT.Sample( samplerState, uv0 );
	float3 tc = texCol.xyz;
	tc = pow(tc, gamma);
	tc = tc * nColors;
	tc = floor(tc);
	tc = tc / nColors;
	tc = pow(tc,1.0/gamma);
	return float4(tc,texCol.w);
}
