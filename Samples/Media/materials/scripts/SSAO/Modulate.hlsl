SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

uniform Texture2D ssao : register(s0);
uniform Texture2D scene : register(s1);

struct fragmentIn
{
	float4 position : SV_POSITION;
    float2 uv: TEXCOORD0; 
};

float4 Modulate_fp
(
	fragmentIn input
) : SV_Target
{
    float4 oColor0 = float4((scene.Sample(g_samLinear, input.uv) * ssao.Sample(g_samLinear, input.uv)).rgb, 1.0);
	return oColor0;
}
