#include <metal_stdlib>
using namespace metal;

#define gl_FragCoord inPs.inputPos

struct PS_INPUT
{
	float2 uv0;
	float4 inputPos [[position]];
};

struct Params
{
	float weights[NUM_WEIGHTS];
};

fragment float main_metal
(
	PS_INPUT inPs [[stage_in]],
	constant Params &p                  [[buffer(PARAMETER_SLOT)]],
	texture2d<float, access::read>	tex [[texture(0)]]
)
{
	float val;
	float outColour;
	float firstSmpl;

	firstSmpl = tex.read( uint2( gl_FragCoord.xy ) - uint2( HORIZONTAL_STEP	* (NUM_WEIGHTS - 1),
															VERTICAL_STEP	* (NUM_WEIGHTS - 1) ), 0 ).x;
	outColour = p.weights[0];

	int i;
	for( i=NUM_WEIGHTS - 1; (--i) > 0; )
	{
		val = tex.read( uint2( gl_FragCoord.xy ) - uint2( HORIZONTAL_STEP	* i,
														  VERTICAL_STEP		* i ), 0 ).x;
		outColour += exp( K * (val - firstSmpl) ) * p.weights[NUM_WEIGHTS-i-1];
	}

	val = tex.read( uint2( gl_FragCoord.xy ), 0 ).x;
	outColour += exp( K * (val - firstSmpl) ) * p.weights[NUM_WEIGHTS-1];

	for( i=0; i<NUM_WEIGHTS - 1; ++i )
	{
		val = tex.read( uint2( gl_FragCoord.xy ) + uint2( HORIZONTAL_STEP	* (i+1),
														  VERTICAL_STEP		* (i+1) ), 0 ).x;
		outColour += exp( K * (val - firstSmpl) ) * p.weights[NUM_WEIGHTS-i-2];
	}

	return firstSmpl + log( outColour ) / K;
}
