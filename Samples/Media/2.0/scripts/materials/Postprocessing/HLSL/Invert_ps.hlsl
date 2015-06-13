Texture2D RT				: register(t0);
SamplerState samplerState	: register(s0);

float4 main( float2 uv0 : TEXCOORD0 ) : SV_Target
{
	return 1 - RT.Sample( samplerState, uv0 );
}
