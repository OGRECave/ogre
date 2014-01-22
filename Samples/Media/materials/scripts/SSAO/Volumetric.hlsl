SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
	AddressW = Wrap;
};

struct v2p
{
	float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

uniform Texture2D sMRT1 : register(s0); // fragment normals
uniform Texture2D sMRT2 : register(s1); // view space position, remember that we are looking down the negative Z axis!!!
uniform Texture2D sRand : register(s2);

float4 Volumetric_fp
(
	v2p input,
    
    uniform const float4 cViewportSize, // (viewport_width, viewport_height, inverse_viewport_width, inverse_viewport_height)
    uniform const float cFov, // vertical field of view in radians
    uniform const float cSampleInScreenspace, // whether to sample in screen or world space
    uniform const float cSampleLengthScreenSpace, // The sample length in screen space [0, 1]
    uniform const float cSampleLengthWorldSpace // the sample length in world space in units
) : SV_Target
{
    const int interleaved = 4;
    const int m = 8;
    const int n = 4;
    const int numSamples = m * n;    

    const float2 interleaveOffset = input.uv * cViewportSize.xy / interleaved;
    
    const float3 fragmentPosition = sMRT2.Sample(g_samLinear, input.uv).xyz; // the current fragment in view space
    const float3 fragmentNormal = sMRT1.Sample(g_samLinear, input.uv).xyz * float3(1, -1, 1); // the fragment normal

    float rUV = 0; // radius of influence in screen space
    float r = 0; // radius of influence in world space
    if (cSampleInScreenspace == 1)
    {
        rUV = cSampleLengthScreenSpace;
        r = tan(rUV * cFov) * -fragmentPosition.z;
    }
    else
    {
        rUV = atan(cSampleLengthWorldSpace / -fragmentPosition.z) / cFov; // the radius of influence projected into screen space
        r = cSampleLengthWorldSpace;
    }


    if (rUV < (cViewportSize.z)) // abort if the projected radius of influence is smaller than 1 fragment
    {
        return float4(1.0,1.0,1.0,1.0);
    }
    
    const float r2 = r/2;
    const float rUV2 = rUV /2;
    
    const float3 center = fragmentPosition + fragmentNormal * (r2);
    const float2 centerUV = input.uv + fragmentNormal.xy * (rUV2);

    float F = 0; // unoccluded Volume
    float V = 0; // occluded Volume
    float invalid = 0;

    for (float i = 0.0f; i < m; i++)
    for (float j = 0.0f; j < n; j++)
    {
        const float2 randomTC = interleaveOffset + float2(i/(interleaved * m), j/(interleaved * n)); 
        const float2 randomVector = (sRand.SampleLevel(g_samLinear, randomTC, 0) * 2 - 1).xy; // unpack to [-1, 1]^2

        const float2 sample = randomVector * (r2);
        const float2 sampleUV = randomVector * (rUV2);

        const float zEntry = center.z + (r2) * sqrt(1 - sample.x * sample.x - sample.y * sample.y);
        const float zExit = center.z - (r2) * sqrt(1 - sample.x * sample.x - sample.y * sample.y);
        const float zStar = sMRT2.SampleLevel(g_samLinear, centerUV + sampleUV, 0).z;

        F += zExit - zEntry;

        if (zExit <= zStar && zStar <= zEntry)
            V += zStar - zEntry;
        else //if (zStar < zExit)
            V += zExit - zEntry;
    }

    float accessibility = V / F;
    return float4(accessibility.xxx, 1);

}
