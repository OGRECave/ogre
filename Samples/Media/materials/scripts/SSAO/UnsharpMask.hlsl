SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 UnsharpMask_fp
(
	float4 position : SV_POSITION,
    in float2 iTexCoord : TEXCOORD0, 
    
    uniform Texture2D blurred,
    uniform Texture2D mrt0,
    uniform float cLambda
) : SV_Target
{
    float spacialImportance = blurred.Sample(g_samLinear, iTexCoord).w - mrt0.Sample(g_samLinear, iTexCoord).w;
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
	float4 position : SV_POSITION,
    in float2 uv : TEXCOORD0,

    uniform Texture2D mrt1,
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
        blurredDepth += mrt1.SampleLevel(g_samLinear, float2(uv.x - i * stepX * cKernelWidthBias, uv.y), 0).w * geometricWeight;
    }

    blurredDepth /= weights;
    float4 oColor0 = float4(mrt1.Sample(g_samLinear, uv).xyz, blurredDepth);
	return oColor0;
}

float4 GaussianBlurY_fp
(
	float4 position : SV_POSITION,
    in float2 uv : TEXCOORD0,

    uniform Texture2D mrt1,
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
        blurredDepth += mrt1.SampleLevel(g_samLinear, float2(uv.x, uv.y - i * stepY * cKernelWidthBias), 0).w * geometricWeight;
    }

    blurredDepth /= weights;
    float4 oColor0 = float4(mrt1.Sample(g_samLinear, uv).xyz, blurredDepth);
	return oColor0;
}
    
