#include "OgreUnifiedShader.h"

SAMPLER2D(inRTT, 0);

uniform vec2 texelSize;

STATIC const vec4 LUMINENCE_FACTOR  = vec4(0.27, 0.67, 0.06, 0.0); // NTSC (1953)

MAIN_PARAMETERS
IN(vec2 oUv0, TEXCOORD0)
MAIN_DECLARATION
{
    vec4 accum = vec4(0.0, 0.0, 0.0, 0.0);

    // Get colour from source
    accum += texture2D(inRTT, oUv0 + texelSize * vec2(-0.5, -0.5));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2(-0.5, 0.5));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2(0.5, 0.5));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2(0.5, -0.5));
    
	// Adjust the accumulated amount by lum factor
	float lum = dot(accum, LUMINENCE_FACTOR);
	// take average of 4 samples
	lum *= 0.25;
	gl_FragColor = vec4(lum, lum, lum, 1.0);

}
