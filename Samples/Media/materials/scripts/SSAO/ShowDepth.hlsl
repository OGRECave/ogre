SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

uniform Texture2D mrt1 : register(s0);
uniform Texture2D tex : register(s1);

struct fragmentIn
{
	float4 position : SV_POSITION;
    float2 iTexCoord: TEXCOORD0;
};

float4 ShowDepth_fp
(
	fragmentIn input
) : SV_Target
{
    float depth = mrt1.Sample(g_samLinear, input.iTexCoord).w;
    float4 oColor0 = float4(tex.Sample(g_samLinear, float2(depth*20, 0)).rgb, 1.0);
	return oColor0;
}

