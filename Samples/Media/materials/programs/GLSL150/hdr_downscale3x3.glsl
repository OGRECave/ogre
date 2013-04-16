#version 150

uniform sampler2D inRTT;
uniform vec2 texelSize;

in vec2 oUv0;
out vec4 fragColour;

void main(void)
{
    vec4 accum = vec4(0.0);

    // Get colour from source
    accum += texture(inRTT, oUv0 + texelSize * vec2(-1.0, -1.0));
    accum += texture(inRTT, oUv0 + texelSize * vec2( 0.0, -1.0));
    accum += texture(inRTT, oUv0 + texelSize * vec2( 1.0, -1.0));
    accum += texture(inRTT, oUv0 + texelSize * vec2(-1.0,  0.0));
    accum += texture(inRTT, oUv0 + texelSize * vec2( 0.0,  0.0));
    accum += texture(inRTT, oUv0 + texelSize * vec2( 1.0,  0.0));
    accum += texture(inRTT, oUv0 + texelSize * vec2(-1.0,  1.0));
    accum += texture(inRTT, oUv0 + texelSize * vec2( 0.0,  1.0));
    accum += texture(inRTT, oUv0 + texelSize * vec2( 1.0,  1.0));
    
	// take average of 9 samples
	accum *= 0.1111111111111111;

	fragColour = accum;
}
