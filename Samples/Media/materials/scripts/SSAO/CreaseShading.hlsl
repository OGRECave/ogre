// original sources found at Game Developer Magazine March 2008

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

uniform Texture2D sNormal;    // xyz normal + depth [0, 1]
uniform Texture2D sPosition;  // view space position
uniform Texture2D sRandom;    // random texture sampler

float4 CreaseShading_fp
(
	float4 position : SV_POSITION,
    float2 uv : TEXCOORD0,
    
    uniform const float cRange, // the three(four) artistic parameters
    uniform const float cBias,
    uniform const float cAverager,
    uniform const float cMinimumCrease,
    uniform const float cKernelSize, // Bias for the kernel size, Hack for the fixed size 11x11 stipple kernel
    uniform const float4 cViewportSize // (width, height, 1/width, 1/height)
) : SV_Target
{
	// get the view space position and normal of the fragment
    const float3 fragmentPosition = sPosition.Sample(g_samLinear, uv).xyz;
    const float3 fragmentNormal = sNormal.Sample(g_samLinear, uv).xyz;

    float totalGI = 0.0f;
    
    // a diamond shaped (45deg rotated square) stipple pattern around (0, 0) this will be used as offset for the samples
    //        O
    //       O O
    //      O O O
    //     O O O O
    //    O O X O O
    //     O O O O
    //      O O O
    //       O O
    //        O
    // the algorith starts with the leftmost element and walks the diagonal to the topmost element
    // a stippleSize of n yields (((n - 1) / 2)^2) - 1 samples
    // the 'image' above has a stipple size of 11 'cuz it has 5 samples (minus
    // the current fragment position and 4 gaps = 11.

    const int stippleSize = 11; // must be odd
    for (int i = 0; i < (stippleSize + 1) / 2; i++)
    {
        float2 diagonalStart = float2(-(stippleSize - 1) / 2, 0) + i;
        for(int j = 0; j < (stippleSize + 1) / 2; j++)
        {
            float2 sampleOffset = diagonalStart + float2(j, -j);

            float2 sampleUV = uv + (sampleOffset * cViewportSize.zw * cKernelSize);
            float3 samplePos = sPosition.SampleLevel(g_samLinear, sampleUV, 0).xyz;

            float3 toCenter = samplePos - fragmentPosition;
            float distance = length(toCenter);

            toCenter = normalize(toCenter);
            float centerContrib = saturate((dot(toCenter, fragmentNormal) - cMinimumCrease) * cBias);
            float rangeAttenuation = 1.0f - saturate(distance / cRange);

            totalGI += centerContrib * rangeAttenuation;
        }
    }
    
    totalGI /= cAverager;
    float4 oColor0 = 1 - float4(totalGI.xxx, 1);
	return oColor0;
}
