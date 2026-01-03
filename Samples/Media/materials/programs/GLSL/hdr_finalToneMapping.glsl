#include "OgreUnifiedShader.h"

#include "hdr_tonemap_util.glsl"

SAMPLER2D(inRTT, 0);
SAMPLER2D(inBloom, 1);
SAMPLER2D(inLum, 2);

MAIN_PARAMETERS
IN(vec2 oUv0, TEXCOORD0)
MAIN_DECLARATION
{
	// Get main scene colour
    vec4 sceneCol = texture2D(inRTT, oUv0);

	// Get luminence value
	vec4 lum = texture2D(inLum, vec2(0.5, 0.5));

	// tone map this
	vec4 toneMappedSceneCol = toneMap(sceneCol, lum.r);
	
	// Get bloom colour
    vec4 bloom = texture2D(inBloom, oUv0);

	// Add scene & bloom
	gl_FragColor = vec4(toneMappedSceneCol.rgb + bloom.rgb, 1.0);
    
}

