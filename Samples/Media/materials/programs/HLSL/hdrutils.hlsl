// RGBE mode utilities
// RGB each carry a mantissa, A carries a shared exponent
// The exponent is calculated based on the largest colour channel


float3 decodeRGBE8(in float4 rgbe)
{
    // get exponent (-128 since it can be +ve or -ve)
	float exp = rgbe.a * 255 - 128;
	
	// expand out the rgb value
	return rgbe.rgb * exp2(exp);
}

float4 encodeRGBE8(in float3 rgb)
{
	float4 ret;

    // What is the largest colour channel?
	float highVal = max(rgb.r, max(rgb.g, rgb.b));
	
	// Take the logarithm, clamp it to a whole value
	float exp = ceil(log2(highVal));

    // Divide the components by the shared exponent
	ret.rgb = rgb / exp2(exp);
	
	// Store the shared exponent in the alpha channel
	ret.a = (exp + 128) / 255;

	return ret;
}


static const float4 LUMINENCE_FACTOR  = float4(0.27f, 0.67f, 0.06f, 0.0f);
static const float MIDDLE_GREY = 0.72f;
static const float FUDGE = 0.001f;
static const float L_WHITE = 1.5f;
static const float4 BRIGHT_LIMITER = float4(0.6f, 0.6f, 0.6f, 0.0f);


/** Tone mapping function 
@note Only affects rgb, not a
@param inColour The HDR colour
@param lum The scene lumninence 
@returns Tone mapped colour
*/
float4 toneMap(float4 inColour, float lum)
{
	// From Reinhard et al
	// "Photographic Tone Reproduction for Digital Images"
	
	// Initial luminence scaling (equation 2)
    inColour.rgb *= MIDDLE_GREY / (FUDGE + lum);

	// Control white out (equation 4 nom)
    inColour.rgb *= (1.0f + inColour.rgb / L_WHITE);

	// Final mapping (equation 4 denom)
	inColour.rgb /= (1.0f + inColour.rgb);
	
	return inColour;

}

