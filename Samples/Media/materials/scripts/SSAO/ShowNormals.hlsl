SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

uniform Texture2D mrt1;

float4 ShowNormals_fp
(
	float4 position : SV_POSITION,
    in float2 iTexCoord : TEXCOORD0
) : SV_Target
{
    float4 oColor0 = float4(mrt1.Sample(g_samLinear, iTexCoord).xyz / 2 + 0.5, 1.0);
	return oColor0;
}
