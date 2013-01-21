#version 150

uniform sampler2D inRTT;

uniform vec4 sampleOffsets[15];
uniform vec4 sampleWeights[15];

in vec2 oUv0;
out vec4 fragColour;

void main(void)
{
    vec4 accum = vec4(0.0, 0.0, 0.0, 1.0);
	vec2 sampleUV;

    for( int i = 0; i < 15; i++ )
    {
        // Sample from adjacent points, 7 each side and central
        sampleUV = oUv0 + sampleOffsets[i].xy;
        accum += sampleWeights[i] * texture(inRTT, sampleUV);
    }

    fragColour = accum;
}
