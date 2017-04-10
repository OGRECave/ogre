
struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
};

Texture2D tex : register(t0);

float4 main
(
	PS_INPUT inPs,
	uniform float4 weights[(NUM_WEIGHTS + 3u) / 4u],
	float4 gl_FragCoord : SV_Position
) : SV_Target
{
	float val;
	float outColour;
	float firstSmpl;

	firstSmpl = tex.Load( int3( int2( gl_FragCoord.xy ) - int2( HORIZONTAL_STEP	* (NUM_WEIGHTS - 1),
																VERTICAL_STEP	* (NUM_WEIGHTS - 1) ), 0 ) ).x;
	outColour = weights[0][0];

	int i;
	for( i=NUM_WEIGHTS - 1; (--i) > 0; )
	{
		val = tex.Load( int3( int2( gl_FragCoord.xy ) - int2( HORIZONTAL_STEP	* i,
															  VERTICAL_STEP		* i ), 0 ) ).x;
		outColour += exp( K * (val - firstSmpl) ) * weights[(NUM_WEIGHTS-i-1u) >> 2u][(NUM_WEIGHTS-i-1u) & 3u];
	}

	val = tex.Load( int3( gl_FragCoord.xy, 0 ) ).x;
	outColour += exp( K * (val - firstSmpl) ) * weights[(NUM_WEIGHTS-1u) >> 2u][(NUM_WEIGHTS-1u) & 3u];

	for( i=0; i<NUM_WEIGHTS - 1; ++i )
	{
		val = tex.Load( int3( int2( gl_FragCoord.xy ) + int2( HORIZONTAL_STEP	* (i+1),
															  VERTICAL_STEP		* (i+1) ), 0 ) ).x;
		outColour += exp( K * (val - firstSmpl) ) * weights[(NUM_WEIGHTS-i-2u) >> 2u][(NUM_WEIGHTS-i-2u) & 3u];
	}

	return firstSmpl + log( outColour ) / K;
}
