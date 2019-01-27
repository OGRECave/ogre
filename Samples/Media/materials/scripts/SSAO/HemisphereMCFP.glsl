#version 120
varying vec2 oUv0;

uniform sampler2D sMRT1;
uniform sampler2D sMRT2;
uniform sampler2D sRand;
	
uniform vec4 cViewportSize; // (viewport_width, viewport_height, inverse_viewport_width, inverse_viewport_height)
uniform float cFov; // vertical field of view in radians
uniform float cSampleInScreenspace; // whether to sample in screen or world space
uniform float cSampleLengthScreenSpace; // The sample length in screen space [0, 1]
uniform float cSampleLengthWorldSpace; // the sample length in world space in units
uniform float cSampleLengthExponent; // The exponent of the sample length

void main()
{
	const int interleaved = 4;
    const int m = 8;
    const int n = 4;
    const int numSamples = m * n;
    vec2 interleaveOffset = oUv0 * cViewportSize.xy / interleaved;
    vec3 fragmentPosition = texture2D(sMRT2, oUv0).xyz; // the current fragment in view space
    vec3 fragmentNormal = texture2D(sMRT1, oUv0).xyz; // the fragment normal
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

    if (rUV < cViewportSize.z) // abort if the projected radius of influence is smaller than 1 fragment
    {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    float accessibility = 0; // accessibility of the fragment

    const vec3 viewVector = vec3(0.0, 0.0, 1.0); // the constant view vector in view space

    // the reflection vector to align the hemisphere with the fragment normal
    // somehow the x component must be flipped...???
    vec3 reflector = normalize(fragmentNormal + viewVector) * vec3(-1.0, 1.0, 1.0); 

    float count = 0;
    float sampleLength;

    for (float i = 0.0f; i < m; i++)
    for (float j = 0.0f; j < n; j++)
    {
        count ++;

        vec2 randomTC = interleaveOffset + vec2(i/(interleaved * m), j/(interleaved * n)); 
        vec3 randomVector = (texture2D(sRand, randomTC) * 2 - 1).xyz; // unpack to [-1, 1]x[-1, 1]x[1, 1]

        sampleLength = pow(count/(numSamples * 1.0), cSampleLengthExponent);

        vec3 sampleVector = reflect(randomVector, reflector) * sampleLength;

        vec2 sampleTC = oUv0 + sampleVector.xy * rUV;

        vec3 samplePosition = texture2D(sMRT2, sampleTC).xyz;

        if (samplePosition.z < (fragmentPosition.z - sampleVector.z * r)) // thin air
            accessibility++;
        else if(length(fragmentPosition - samplePosition) > r) // solid geometry
			accessibility++; // out of reach, i.e. false occluder
    }

    accessibility /= numSamples;
    gl_FragColor = vec4(accessibility, accessibility, accessibility, 1.0);
}
