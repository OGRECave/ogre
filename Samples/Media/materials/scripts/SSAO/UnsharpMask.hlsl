SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct v2p
{
	float4 position : SV_POSITION;
    float2 uv : TEXCOORD0; 
};

float4 UnsharpMask_fp
(
	v2p input,
    
    uniform Texture2D blurred : register(s0),
    uniform Texture2D mrt0 : register(s1),
    uniform float cLambda
) : SV_Target
{
    float spacialImportance = blurred.Sample(g_samLinear, input.uv).w - mrt0.Sample(g_samLinear, input.uv).w;
    float4 color = float4(1,1,1,1);
	float4 oColor0;
    if (spacialImportance < 0) // darkening only
    {
        oColor0 = float4(color.rgb + (cLambda * spacialImportance), 1);
    } else 
    {
        oColor0 = color;
    }
	return oColor0;
}

float4 GaussianBlurX_fp
(
	v2p input,

    uniform Texture2D mrt1 : register(s0),
    uniform float stepX,
    uniform const float cKernelWidthBias
) : SV_Target
{
    const int kernelWidth = 19;
    float sigma = (kernelWidth - 1) / 6; // make the kernel span 6 sigma

    float weights = 0;
    float blurredDepth = 0;
    
    for (float i = -(kernelWidth - 1) / 2; i < (kernelWidth - 1) / 2; i++)
    {
        float geometricWeight = exp(-pow(i, 2) / (2 * pow(sigma, 2)));
        weights += geometricWeight;
        blurredDepth += mrt1.SampleLevel(g_samLinear, float2(input.uv.x - i * stepX * cKernelWidthBias, input.uv.y), 0).w * geometricWeight;
    }

    blurredDepth /= weights;
    float4 oColor0 = float4(mrt1.Sample(g_samLinear, input.uv).xyz, blurredDepth);
	return oColor0;
}

float4 GaussianBlurY_fp
(
	v2p input,

    uniform Texture2D mrt1 : register(s0),
    uniform const float stepY,
    uniform const float cKernelWidthBias
) : SV_Target
{
    const int kernelWidth = 19;
    float sigma = (kernelWidth - 1) / 6; // make the kernel span 6 sigma

    float weights = 0;
    float blurredDepth = 0;
    
    for (float i = -(kernelWidth - 1) / 2; i < (kernelWidth - 1) / 2; i++)
    {
        float geometricWeight = exp(-pow(i, 2) / (2 * pow(sigma, 2)));
        weights += geometricWeight;
        blurredDepth += mrt1.SampleLevel(g_samLinear, float2(input.uv.x, input.uv.y - i * stepY * cKernelWidthBias), 0).w * geometricWeight;
    }

    blurredDepth /= weights;
    float4 oColor0 = float4(mrt1.Sample(g_samLinear, input.uv).xyz, blurredDepth);
	return oColor0;
}
    
