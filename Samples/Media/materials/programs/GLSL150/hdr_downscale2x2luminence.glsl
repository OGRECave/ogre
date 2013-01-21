#version 150

uniform sampler2D inRTT;
uniform vec2 texelSize;

in vec2 oUv0;
out vec4 fragColour;

void main(void)
{
	
    vec4 accum = vec4(0.0);
	vec4 LUMINENCE_FACTOR = vec4(0.27, 0.67, 0.06, 0.0);

    // Get colour from source
    accum += texture(inRTT, oUv0 + texelSize * vec2(-0.5, -0.5));
    accum += texture(inRTT, oUv0 + texelSize * vec2(-0.5, 0.5));
    accum += texture(inRTT, oUv0 + texelSize * vec2(0.5, 0.5));
    accum += texture(inRTT, oUv0 + texelSize * vec2(0.5, -0.5));

	// Adjust the accumulated amount by lum factor
	float lum = dot(accum, LUMINENCE_FACTOR);

	// Take average of 4 samples
	lum *= 0.25;
	fragColour = vec4(lum, lum, lum, 1.0);
}
