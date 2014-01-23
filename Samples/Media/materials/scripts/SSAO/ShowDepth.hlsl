SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

uniform Texture2D mrt1;
uniform Texture2D tex;

float4 ShowDepth_fp
(
	float4 position : SV_POSITION,
    in float2 iTexCoord: TEXCOORD0
) : SV_Target
{
    float depth = mrt1.Sample(g_samLinear, iTexCoord).w;
    float4 oColor0 = float4(tex.Sample(g_samLinear, float2(depth*20, 0)).rgb, 1.0);
	return oColor0;
}

