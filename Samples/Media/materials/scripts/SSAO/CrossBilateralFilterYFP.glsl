#version 120
varying vec2 oUv0;

uniform sampler2D sAccessibility;
uniform sampler2D sMRT2;
uniform float stepY; // inverse viewport width
uniform float cPhotometricExponent;

void main()
{
    const int kernelWidth = 13;
    float sigma = (kernelWidth - 1) / 6; // make the kernel span 6 sigma
    
    float fragmentDepth = texture2D(sMRT2, oUv0).z;

    float weights = 0;
    float blurred = 0;
    
    for (float i = -(kernelWidth - 1) / 2; i < (kernelWidth - 1) / 2; i++)
    {
        float geometricWeight = exp(-pow(i, 2) / (2 * pow(sigma, 2)));
        float sampleDepth = texture2D(sMRT2, vec2(oUv0.x, oUv0.y - i * stepY)).z;
        float photometricWeight = 1 / pow((1 + abs(fragmentDepth - sampleDepth)), cPhotometricExponent);
        
        weights += (geometricWeight * photometricWeight);
        blurred += texture2D(sAccessibility, vec2(oUv0.x, oUv0.y - i * stepY)).r * geometricWeight * photometricWeight;
    }

    blurred /= weights;
    gl_FragColor = vec4(blurred, blurred, blurred, 1);
}
