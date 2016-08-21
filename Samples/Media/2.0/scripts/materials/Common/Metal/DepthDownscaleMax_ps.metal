#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct PS_OUTPUT
{
	float depth [[depth]];
};

fragment PS_OUTPUT main_metal
(
	PS_INPUT inPs [[stage_in]],

	depth2d<float, access::read> depthTexture [[texture(0)]],

	float4 gl_FragCoord [[position]]
)
{
	float fDepth0 = depthTexture.read( uint2(gl_FragCoord.xy * 2.0) ).x;
	float fDepth1 = depthTexture.read( uint2(gl_FragCoord.xy * 2.0) + uint2( 0, 1 ) ).x;
	float fDepth2 = depthTexture.read( uint2(gl_FragCoord.xy * 2.0) + uint2( 1, 0 ) ).x;
	float fDepth3 = depthTexture.read( uint2(gl_FragCoord.xy * 2.0) + uint2( 1, 1 ) ).x;
	
	PS_OUTPUT outPs;
	//outPs.depth =depthTexture.read( uint2(gl_FragCoord.xy * 2.0) ).x;
	outPs.depth = max( max( fDepth0, fDepth1 ), max( fDepth2, fDepth3 ) );
	return outPs;
}
