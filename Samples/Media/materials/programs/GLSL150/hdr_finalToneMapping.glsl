#version 150

uniform sampler2D inRTT;
uniform sampler2D inBloom;
uniform sampler2D inLum;

in vec2 oUv0;
out vec4 fragColour;

// declare external function
vec4 toneMap(in vec4 inColour, in float lum);

void main(void)
{
	// Get main scene colour
    vec4 sceneCol = texture(inRTT, oUv0);

	// Get luminence value
	vec4 lum = texture(inLum, vec2(0.5));

	// tone map this
	vec4 toneMappedSceneCol = toneMap(sceneCol, lum.r);
	
	// Get bloom colour
    vec4 bloom = texture(inBloom, oUv0);

	// Add scene & bloom
	fragColour = vec4(toneMappedSceneCol.rgb + bloom.rgb, 1.0);
}
