SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

uniform Texture2D mrt2;

float4 ShowViewPos_fp
(
	float4 position : SV_POSITION,
    in float2 iTexCoord : TEXCOORD0
) : SV_Target
{
    float4 oColor0 = float4(mrt2.Sample(g_samLinear, iTexCoord).xyz * float3(0.1, 0.1, -0.01), 1.0);
	return oColor0;
}
