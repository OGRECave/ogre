#version 120
varying vec2 oUv0;

uniform sampler2D sNormal;
uniform sampler2D sPosition;
	
uniform float cRange; // the three(four) artistic parameters
uniform float cBias;
uniform float cAverager;
uniform float cMinimumCrease;
uniform float cKernelSize; // Bias for the kernel size, Hack for the fixed size 11x11 stipple kernel 
uniform vec4 cViewportSize;

void main()
{
    // get the view space position and normal of the fragment
    vec3 fragmentPosition = texture2D(sPosition, oUv0).xyz;
    vec3 fragmentNormal = texture2D(sNormal, oUv0).xyz;
	
	float totalGI = 0.0f;
	
	const int stippleSize = 11; // must be odd
    for (int i = 0; i < (stippleSize + 1) / 2; i++)
    {
        vec2 diagonalStart = vec2(-(stippleSize - 1.0) / 2.0, 0) + i;
        for(int j = 0; j < (stippleSize + 1) / 2; j++)
        {
            vec2 sampleOffset = diagonalStart + vec2(j, -j);

            vec2 sampleUV = oUv0 + (sampleOffset * cViewportSize.zw * cKernelSize);
            vec3 samplePos = texture2D(sPosition, sampleUV).xyz;

            vec3 toCenter = samplePos - fragmentPosition;
            float distance = length(toCenter);

            toCenter = normalize(toCenter);
            float centerContrib = clamp((dot(toCenter, fragmentNormal) - cMinimumCrease) * cBias, 0.0, 1.0);
            float rangeAttenuation = 1.0f - clamp(distance / cRange, 0.0, 1.0);

            totalGI += centerContrib * rangeAttenuation;
        }
    }
    
    totalGI /= cAverager;
    gl_FragColor = 1.0 - vec4(totalGI, totalGI, totalGI, 1.0);
}
