Texture2D testTexture			: register(t0);
SamplerState samplerState       : register(s0);

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

float4 main( PS_INPUT inPs ) : SV_Target
{
	return testTexture.Sample( samplerState, inPs.uv0.xy );
}
