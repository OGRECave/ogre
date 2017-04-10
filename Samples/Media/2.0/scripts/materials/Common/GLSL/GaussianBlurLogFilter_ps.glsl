#version 330

uniform sampler2D tex;

in block
{
	vec2 uv0;
} inPs;

out float fragColour;

in vec4 gl_FragCoord;

uniform float weights[NUM_WEIGHTS];

void main()
{
	float val;
	float outColour;
	float firstSmpl;

	firstSmpl = texelFetch( tex, ivec2( gl_FragCoord.xy ) - ivec2( HORIZONTAL_STEP	* (NUM_WEIGHTS - 1),
																   VERTICAL_STEP	* (NUM_WEIGHTS - 1) ), 0 ).x;
	outColour = weights[0];

	int i;
	for( i=NUM_WEIGHTS - 1; (--i) > 0; )
	{
		val = texelFetch( tex, ivec2( gl_FragCoord.xy ) - ivec2( HORIZONTAL_STEP* i,
																 VERTICAL_STEP	* i ), 0 ).x;
		outColour += exp( K * (val - firstSmpl) ) * weights[NUM_WEIGHTS-i-1];
	}

	val = texelFetch( tex, ivec2( gl_FragCoord.xy ), 0 ).x;
	outColour += exp( K * (val - firstSmpl) ) * weights[NUM_WEIGHTS-1];

	for( i=0; i<NUM_WEIGHTS - 1; ++i )
	{
		val = texelFetch( tex, ivec2( gl_FragCoord.xy ) + ivec2( HORIZONTAL_STEP* (i+1),
																 VERTICAL_STEP	* (i+1) ), 0 ).x;
		outColour += exp( K * (val - firstSmpl) ) * weights[NUM_WEIGHTS-i-2];
	}

	fragColour = firstSmpl + log( outColour ) / K;
}
