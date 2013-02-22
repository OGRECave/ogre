
Texture2D RT;          
Texture2D Sum;          

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 Combine_fp_ps40
(
	float4 posIn: SV_Position ,
    float2 texCoord: TEXCOORD0,

    uniform float blur
) : SV_Target
{
   float4 render = RT.Sample(g_samLinear, texCoord);
   float4 sum = Sum.Sample(g_samLinear, texCoord);

   return lerp(render, sum, blur);
}