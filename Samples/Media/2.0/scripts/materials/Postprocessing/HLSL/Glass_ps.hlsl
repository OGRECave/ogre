
Texture2D RT				: register(t0);
Texture2D<float2> NormalMap	: register(t1);

SamplerState samplerState[2]: register(s0);

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

float4 main( PS_INPUT inPs ) : SV_Target
{
	float2 normal = 2 * (NormalMap.Sample( samplerState[1], inPs.uv0 * 2.5 ).xy - 0.5);

	return RT.Sample( samplerState[0], inPs.uv0 + normal.xy * 0.05 );
}
