SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct v2p
{
	float4 position : SV_POSITION;
    float2 fragmentTC : TEXCOORD0;
};

uniform Texture2D sSceneDepthSampler; // depth = w component [0, 1]
uniform Texture2D sRotSampler4x4;  // rotation sampler -> pseudo random spherical weighted sampling
float4 Crytek_fp
(
	v2p input,
    
    uniform float4 cViewportSize, // auto param width/height/inv. width/inv. height
    uniform float cFov, // vertical field of view in radians
    uniform float farClipDistance,
    uniform float nearClipDistance,
    uniform float cSampleInScreenspace, // whether to sample in screen or world space
    uniform float cSampleLengthScreenSpace, // The sample length in screen space [0, 1]
    uniform float cSampleLengthWorldSpace, // the sample length in world space in units
    uniform float cOffsetScale, // [0, 1] The distance of the first sample. samples are the 
        // placed in [cOffsetScale * cSampleLengthScreenSpace, cSampleLengthScreenSpace]
    uniform float cDefaultAccessibility, // the default value used in the lerp() expression for invalid samples [0, 1]
    uniform float cEdgeHighlight // multiplier for edge highlighting in [1, 2] 1 is full highlighting 2 is off
) : SV_Target
{
    const int nSampleNum = 32; // number of samples

    // compute the distance between the clipping planes to convert [0, 1] depth to world space units
    const float clipDepth = farClipDistance - nearClipDistance;

    // get the depth of the current pixel and convert into world space unit [0, inf]
    float fragmentWorldDepth = sSceneDepthSampler.Sample(g_samLinear, input.fragmentTC).w * clipDepth;

    // get rotation vector, rotation is tiled every 4 screen pixels
    float2 rotationTC = input.fragmentTC * cViewportSize.xy / 4;
    float3 rotationVector = 2 * sRotSampler4x4.Sample(g_samLinear, rotationTC).xyz - 1; // [-1, 1]x[-1. 1]x[-1. 1]
    
    float rUV = 0; // radius of influence in screen space
    float r = 0; // radius of influence in world space
    if (cSampleInScreenspace == 1)
    {
        rUV = cSampleLengthScreenSpace;
        r = tan(rUV * cFov) * fragmentWorldDepth;
    }
    else
    {
        rUV = atan(cSampleLengthWorldSpace / fragmentWorldDepth) / cFov; // the radius of influence projected into screen space
        r = cSampleLengthWorldSpace;
    }

    float sampleLength = cOffsetScale; // the offset for the first sample
    const float sampleLengthStep = pow((rUV / sampleLength), 1.0f/nSampleNum);
    
    float accessibility = 0;
    // sample the sphere and accumulate accessibility
    for (int i = 0; i < (nSampleNum/8); i++)
    {
        for (int x = -1; x <= 1; x += 2)
        for (int y = -1; y <= 1; y += 2)
        for (int z = -1; z <= 1; z += 2)
        {
            //generate offset vector
            float3 offset = normalize(float3(x, y, z)) * sampleLength;
            
            // update sample length
            sampleLength *= sampleLengthStep;
        
            // reflect offset vector by random rotation sample (i.e. rotating it) 
            float3 rotatedOffset = reflect(offset, rotationVector);
                    
            float2 sampleTC = input.fragmentTC + rotatedOffset.xy * rUV;
                
            // read scene depth at sampling point and convert into world space units (m or whatever)
            float sampleWorldDepth = sSceneDepthSampler.SampleLevel(g_samLinear, sampleTC, 0).w * clipDepth;
            
            // check if depths of both pixels are close enough and sampling point should affect our center pixel
            float fRangeIsInvalid = saturate((fragmentWorldDepth - sampleWorldDepth) / r);
            
            // accumulate accessibility, use default value of 0.5 if right computations are not possible
            accessibility += lerp(sampleWorldDepth > (fragmentWorldDepth + rotatedOffset.z * r), cDefaultAccessibility, fRangeIsInvalid);
        }
    }
    
    // get average value
    accessibility /= nSampleNum;

    // normalize, remove edge highlighting
    accessibility *= cEdgeHighlight;
    
    // amplify and saturate if necessary
    float4 oColor0 = float4(accessibility.xxx, 1);
	return oColor0;
}
