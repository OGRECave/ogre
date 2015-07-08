#version 330

uniform sampler2D rt0;

uniform vec4	centerUVPos; //z = min, w = 1 / (max - min)
uniform float	exponent;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

#define NUM_SAMPLES 7

void main()
{
	const float c_multipliers[NUM_SAMPLES] = float[NUM_SAMPLES]
	(
		1.00f,
		0.99f,
		0.98f,
		0.97f,
		0.96f,
		0.94f,
		0.93f
	);

	float atten = ( distance( inPs.uv0, centerUVPos.xy ) - centerUVPos.z ) * centerUVPos.w;
	atten = 1.0f - clamp( atten, 0.0, 1.0 );
	atten = pow( atten, exponent );

	vec4 originalColour = texture( rt0, (inPs.uv0 - centerUVPos.xy) *
										c_multipliers[0] + centerUVPos.xy );

	vec4 sumColours = vec4( 0.0, 0.0, 0.0, 0.0 );
	for( int i=1; i<NUM_SAMPLES; ++i )
	{
		sumColours += texture( rt0, (inPs.uv0 - centerUVPos.xy) *
									c_multipliers[i] + centerUVPos.xy );
	}

	//'originalColour' has always the same strength. 'sumColours' gets weaker over distance (past min distance).
	//The denominator is altered according to the distance, so that the average is always consistent.
	fragColour = ( originalColour + sumColours * atten ) / (1 + (NUM_SAMPLES-1) * atten );
}
