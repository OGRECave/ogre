#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct PS_OUTPUT
{
	float depth [[depth(any)]];
};

fragment PS_OUTPUT main_metal
(
	PS_INPUT inPs [[stage_in]],

	depth2d<float, access::read> depthTexture [[texture(0)]],

	float4 gl_FragCoord [[position]]
)
{
	uint2 iFragCoord = (uint2)(gl_FragCoord.xy * 2.0);
	float fDepth0 = depthTexture.read( iFragCoord );
	float fDepth1 = depthTexture.read( iFragCoord + uint2( 0, 1 ) );
	float fDepth2 = depthTexture.read( iFragCoord + uint2( 1, 0 ) );
	float fDepth3 = depthTexture.read( iFragCoord + uint2( 1, 1 ) );
	
	PS_OUTPUT outPs;
	//outPs.depth =depthTexture.read( uint2(gl_FragCoord.xy * 2.0) ).x;
	outPs.depth = max( max( fDepth0, fDepth1 ), max( fDepth2, fDepth3 ) );
	return outPs;
}
