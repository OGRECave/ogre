// a very simple and slightly dumb depth aware 4x4 box filter
// the kernel has the following form
//   o o o o
//   o o o o
//   o o x o
//   o o o o 
// where x marks the fragment position and the o marks a sampling point
#version 120
varying vec2 oUv0;

uniform sampler2D sMrt1;
uniform sampler2D sOcclusion;
uniform vec4 screenSize;
uniform float farClipDistance;
	
void main()
{
    float fragmentDepth = texture2D(sMrt1, oUv0).x;

    float color = 0;
    float weight = 0;
    for (int x = -2; x < 2; x++)
    for (int y = -2; y < 2; y++)
    {
        float sampleDepth = texture2D(sMrt1, vec2(oUv0.x + x * screenSize.z, oUv0.y + y * screenSize.w)).x;
        float dist = abs(fragmentDepth - sampleDepth) * farClipDistance + 0.5;
        float sampleWeight = 1 / (pow(dist, 1) + 1);
        color += sampleWeight * texture2D(sOcclusion, vec2(oUv0.x + x * screenSize.z, oUv0.y + y * screenSize.w)).x;
        weight += sampleWeight;
    }
    color /= weight;
        
    gl_FragColor = vec4(color, color, color, 1);
}