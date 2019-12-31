uniform sampler2D inRTT;
uniform sampler2D inBloom;
uniform sampler2D inLum;

varying vec2 oUv0;

#include "hdr_tonemap_util.glsl"

void main(void)
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

