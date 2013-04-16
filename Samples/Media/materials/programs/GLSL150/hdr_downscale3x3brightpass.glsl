#version 150

uniform sampler2D inRTT;
uniform sampler2D inLum;
uniform vec2 texelSize;

in vec2 oUv0;
out vec4 fragColour;
const vec4 BRIGHT_LIMITER = vec4(0.6, 0.6, 0.6, 0.0);

// declare external function
vec4 toneMap(in vec4 inColour, in float lum);

void main(void)
{
    vec4 accum = vec4(0.0);

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

    // Reduce bright and clamp
    accum = max(vec4(0.0, 0.0, 0.0, 1.0), accum - BRIGHT_LIMITER);

	// Sample the luminence texture
	vec4 lum = texture(inLum, vec2(0.5));
	
	// Tone map result
	fragColour = toneMap(accum, lum.r);
}
