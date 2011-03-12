#version 100
precision mediump int;
precision mediump float;

uniform sampler2D inRTT;
uniform sampler2D inBloom;
uniform sampler2D inLum;

varying vec2 uv;

const float MIDDLE_GREY = 0.72;
const float FUDGE = 0.001;
const float L_WHITE = 1.5;

/** Tone mapping function 
@note Only affects rgb, not a
@param inColour The HDR colour
@param lum The scene lumninence 
@returns Tone mapped colour
*/
vec4 toneMap(in vec4 inColour, in float lum)
{
	// From Reinhard et al
	// "Photographic Tone Reproduction for Digital Images"
	
	// Initial luminence scaling (equation 2)
    inColour.rgb *= MIDDLE_GREY / (FUDGE + lum);

	// Control white out (equation 4 nom)
    inColour.rgb *= (1.0 + inColour.rgb / L_WHITE);

	// Final mapping (equation 4 denom)
	inColour.rgb /= (1.0 + inColour.rgb);
	
	return inColour;
}

void main(void)
{
	// Get main scene colour
    vec4 sceneCol = texture2D(inRTT, uv);

	// Get luminence value
	vec4 lum = texture2D(inLum, vec2(0.5, 0.5));

	// tone map this
	vec4 toneMappedSceneCol = toneMap(sceneCol, lum.r);
	
	// Get bloom colour
    vec4 bloom = texture2D(inBloom, uv);

	// Add scene & bloom
	gl_FragColor = vec4(toneMappedSceneCol.rgb + bloom.rgb, 1.0);
}
