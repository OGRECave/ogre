#include "OgreUnifiedShader.h"

#include "hdr_tonemap_util.glsl"

SAMPLER2D(inRTT, 0);
SAMPLER2D(inLum, 1);

uniform vec2 texelSize;

STATIC const vec4 BRIGHT_LIMITER = vec4(0.6, 0.6, 0.6, 0.0);

MAIN_PARAMETERS
IN(vec2 oUv0, TEXCOORD0)
MAIN_DECLARATION
{
    vec4 accum = vec4(0.0, 0.0, 0.0, 0.0);

    accum += texture2D(inRTT, oUv0 + texelSize * vec2(-1.0, -1.0));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2( 0.0, -1.0));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2( 1.0, -1.0));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2(-1.0,  0.0));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2( 0.0,  0.0));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2( 1.0,  0.0));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2(-1.0,  1.0));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2( 0.0,  1.0));
    accum += texture2D(inRTT, oUv0 + texelSize * vec2( 1.0,  1.0));

	// take average of 9 samples
	accum *= 0.1111111111111111;

    // Reduce bright and clamp
    accum = max(vec4(0.0, 0.0, 0.0, 1.0), accum - BRIGHT_LIMITER);

	// Sample the luminence texture
	vec4 lum = texture2D(inLum, vec2(0.5, 0.5));

	// Tone map result
	gl_FragColor = toneMap(accum, lum.r);

}
