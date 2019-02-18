#version 120
varying vec2 oUv0;

uniform sampler2D sMRT1; // fragment normals
uniform sampler2D sMRT2; // view space position, remember that we are looking down the negative Z axis!!!
uniform sampler2D sRand;

uniform vec4 cViewportSize; // (viewport_width, viewport_height, inverse_viewport_width, inverse_viewport_height)
uniform float cFov; // vertical field of view in radians
uniform float cSampleInScreenspace; // whether to sample in screen or world space
uniform float cSampleLengthScreenSpace; // The sample length in screen space [0, 1]
uniform float cSampleLengthWorldSpace; // the sample length in world space in units
	
void main()
{
    const int interleaved = 4;
    const int m = 8;
    const int n = 4;
    const int numSamples = m * n;    

    vec2 interleaveOffset = oUv0 * cViewportSize.xy / interleaved;
    
    vec3 fragmentPosition = texture2D(sMRT2, oUv0).xyz; // the current fragment in view space
    vec3 fragmentNormal = texture2D(sMRT1, oUv0).xyz * vec3(1, -1, 1); // the fragment normal

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
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }
    
    float r2 = r/2;
    float rUV2 = rUV /2;
    
    vec3 center = fragmentPosition + fragmentNormal * (r2);
    vec2 centerUV = oUv0 + (fragmentNormal * (rUV2)).xy;

    float F = 0; // unoccluded Volume
    float V = 0; // occluded Volume
    float invalid = 0;

    for (float i = 0.0f; i < m; i++)
    for (float j = 0.0f; j < n; j++)
    {
        vec2 randomTC = interleaveOffset + vec2(i/(interleaved * m), j/(interleaved * n)); 
        vec2 randomVector = (texture2D(sRand, randomTC) * 2 - 1).xy; // unpack to [-1, 1]^2

        vec2 sample = randomVector * (r2);
        vec2 sampleUV = randomVector * (rUV2);

        float zEntry = center.z + (r2) * sqrt(1 - sample.x * sample.x - sample.y * sample.y);
        float zExit = center.z - (r2) * sqrt(1 - sample.x * sample.x - sample.y * sample.y);
        float zStar = texture2D(sMRT2, centerUV + sampleUV).z;

        F += zExit - zEntry;

        if (zExit <= zStar && zStar <= zEntry)
            V += zStar - zEntry;
        else //if (zStar < zExit)
            V += zExit - zEntry;
    }

    float accessibility = V / F;
    gl_FragColor = vec4(accessibility, accessibility, accessibility, 1.0);

}
