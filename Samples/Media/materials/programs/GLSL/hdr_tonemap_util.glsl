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


