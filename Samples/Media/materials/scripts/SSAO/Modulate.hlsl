SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

uniform Texture2D ssao;
uniform Texture2D scene;

float4 Modulate_fp
(
	float4 position : SV_POSITION,
    float2 uv: TEXCOORD0 
) : SV_Target
{
    float4 oColor0 = float4((scene.Sample(g_samLinear, uv) * ssao.Sample(g_samLinear, uv)).rgb, 1.0);
	return oColor0;
}
