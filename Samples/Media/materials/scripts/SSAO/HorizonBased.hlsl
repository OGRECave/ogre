SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
	AddressW = Wrap;
};

uniform Texture2D sMRT1 : register(s0); // fragment normals
uniform Texture2D sMRT2 : register(s1); // view space position, remember that we are looking down the negative Z axis!!!
uniform Texture2D sRand : register(s2); // MxN random texture, M sets of N precomputed low-discrepancy samples

struct fragmentIn
{
	float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 HorizonBased_fp
(
	fragmentIn input,
    
    uniform float4 cViewportSize, // (viewport_width, viewport_height, inverse_viewport_width, inverse_viewport_height)
    uniform float cFov, // vertical field of view in radians
    uniform float cSampleInScreenspace, // whether to sample in screen or world space
    uniform float cSampleLengthScreenSpace, // The sample length in screen space [0, 1]
    uniform float cSampleLengthWorldSpace, // the sample length in world space in units
    uniform float cAngleBias // angle bias to avoid shadows in low tessellated curvatures [0, pi/2]
) : SV_Target
{
    const float pi = 3.1415926535897932384626433832795028841971693993751;

    const float numSteps = 7; // number of samples/steps along a direction
    const float numDirections = 4; // number of sampling directions in uv space
    
    float3 p = sMRT2.Sample(g_samLinear, input.uv).xyz; // the current fragment in view space
    float3 pointNormal = sMRT1.Sample(g_samLinear, input.uv).xyz; // the fragment normal

    float Ruv = 0; // radius of influence in screen space
    float R = 0; // radius of influence in world space
    if (cSampleInScreenspace == 1)
    {
        Ruv = cSampleLengthScreenSpace;
        R = tan(Ruv * cFov) * -p.z;
    }
    else
    {
        Ruv = atan(cSampleLengthWorldSpace / -p.z) / cFov; // the radius of influence projected into screen space
        R = cSampleLengthWorldSpace;
    }

    // if the radius of influence is smaller than one fragment we exit early,
    // since all samples would hit the current fragment.
    if (Ruv < (1 / cViewportSize.x))
    {
        return float4(1, 1, 1, 1);
    }

    float occlusion = 0; // occlusion of the fragment
    float2x2 directionMatrix;    // the matrix to create the sample directions
                                 // the compiler should evaluate the directions at compile time
    directionMatrix._m00 = cos((2 * pi) / numDirections);
    directionMatrix._m01 = sin((2 * pi) / numDirections);
    directionMatrix._m10 = - sin((2 * pi) / numDirections);
    directionMatrix._m11 = cos((2 * pi) / numDirections);
    float2 deltaUV = float2(1, 0) * (Ruv / (numSteps + 1)); // The step vector in view space. scale it to the step size
            // we don't want to sample to the perimeter of R since those samples would be 
            // omitted by the distance attenuation (W(R) = 0 by definition)
            // Therefore we add a extra step and don't use the last sample.

    float3 randomValues = sRand.Sample(g_samLinear, (input.uv * cViewportSize.xy) / 4).xyz; //4px tiles
    float2x2 rotationMatrix;
    rotationMatrix._m00 = (randomValues.x - 0.5) * 2;
    rotationMatrix._m01 = (randomValues.y - 0.5) * 2;
    rotationMatrix._m10 = - rotationMatrix._m01;
    rotationMatrix._m11 = rotationMatrix._m00;
    float jitter = randomValues.z;
	
	float2 sampleDirection;
	float oldAngle;
	float2 sampleUV;
	float3 sample;
	float3 sampleVector;
	float gamma;
	float attenuation;
	
    /*[unroll]*/ for (int i = 0; i < numDirections; i++)
    {
        deltaUV = mul(deltaUV, directionMatrix); // rotate the deltaUV vector by 1/numDirections
        sampleDirection = mul(deltaUV, rotationMatrix); // now rotate this vector with the random rotation

        oldAngle = cAngleBias;

        /*[unroll]*/ for (int j = 1; j <= numSteps; j++) // sample along a direction, needs to start at one, for the sake of the next line
        {
            sampleUV = input.uv + ((jitter + j) * sampleDirection); // jitter the step a little bit
            
            sample = sMRT2.SampleLevel(g_samLinear, sampleUV, 0).xyz; // the sample in view space
            sampleVector = (sample - p);
            gamma = (pi / 2) - acos(dot(pointNormal, normalize(sampleVector))); //the angle between the fragment tangent and the sample

            if (gamma > oldAngle) 
            {
                attenuation = saturate(1 - (pow((length(sampleVector) / R), 2)));
                occlusion += attenuation * (sin(gamma) - sin(oldAngle));
                oldAngle = gamma;
            }
        }
    }

    // ??? should step samples that fall under the horizontal be considered in the following line??? 
    occlusion /= (numDirections * numSteps);
    float4 oColor0 = 1 - float4(occlusion.xxx, 1) * 2 * pi;
	return oColor0;
}