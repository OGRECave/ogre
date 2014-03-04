SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
	AddressW = Wrap;
};

Texture2D sMRT1 : register(s0); // fragment normals
Texture2D sMRT2 : register(s1); // view space position, remember that we are looking down the negative Z axis!!!
Texture2D sRand : register(s2); // MxN random texture, M sets of N precomputed low-discrepancy samples

struct v2p
{
	float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 HemisphereMC_fp
(     
	v2p input,
    uniform const float4 cViewportSize, // (viewport_width, viewport_height, inverse_viewport_width, inverse_viewport_height)
    uniform const float cFov, // vertical field of view in radians
    uniform const float cSampleInScreenspace, // whether to sample in screen or world space
    uniform const float cSampleLengthScreenSpace, // The sample length in screen space [0, 1]
    uniform const float cSampleLengthWorldSpace, // the sample length in world space in units
    uniform const float cSampleLengthExponent // The exponent of the sample length
) : SV_Target
{
    const int interleaved = 4;
    const int m = 8;
    const int n = 4;
    const int numSamples = m * n;
    const float2 interleaveOffset = input.uv * cViewportSize.xy / interleaved;
    const float3 fragmentPosition = sMRT2.Sample(g_samLinear, input.uv).xyz; // the current fragment in view space
    const float3 fragmentNormal = sMRT1.Sample(g_samLinear, input.uv).xyz; // the fragment normal
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
    
	float accessibility = 0; // accessibility of the fragment

    const float3 viewVector = float3(0, 0, 1); // the constant view vector in view space

    // the reflection vector to align the hemisphere with the fragment normal
    // somehow the x component must be flipped...???
    const float3 reflector = normalize(fragmentNormal + viewVector) * float3(-1, 1, 1); 

    float count = 0;
    float sampleLength;
	float2 randomTC;
	float3 randomVector;
	float3 sampleVector;
	float2 sampleTC;
	float3 samplePosition;
    for (float i = 0.0f; i < m; i++)
    for (float j = 0.0f; j < n; j++)
    {
        count ++;

        randomTC = interleaveOffset + float2(i/(interleaved * m), j/(interleaved * n)); 
        randomVector = (sRand.SampleLevel(g_samLinear, randomTC, 0) * 2 - 1).xyz; // unpack to [-1, 1]x[-1, 1]x[1, 1]

        sampleLength = pow(count/(float)numSamples, cSampleLengthExponent);

        sampleVector = reflect(randomVector, reflector) * sampleLength;

        sampleTC = input.uv + sampleVector.xy * rUV;

        samplePosition = sMRT2.SampleLevel(g_samLinear, sampleTC, 0).xyz;

        if (samplePosition.z < (fragmentPosition.z - sampleVector.z * r)) // thin air
            accessibility++;
        else // solid geometry
            accessibility += length(fragmentPosition - samplePosition) > r; // out of reach, i.e. false occluder
    }

    accessibility /= numSamples;

    float3 direction = 0;
    direction += reflect(float3(0, 0, -1), reflector);
    direction = normalize(direction);
	
	return float4(accessibility.xxx, 1);
}
