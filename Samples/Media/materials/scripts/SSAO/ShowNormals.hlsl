SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

uniform Texture2D mrt1 : register(s0);

struct fragmentIn
{
	float4 position : SV_POSITION;
    float2 iTexCoord : TEXCOORD0;
};

float4 ShowNormals_fp
(
	fragmentIn input
) : SV_Target
{
    float4 oColor0 = float4(mrt1.Sample(g_samLinear, input.iTexCoord).xyz / 2 + 0.5, 1.0);
	return oColor0;
}
