#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
	float4 gl_FragCoord [[position]];
};

fragment float main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d_ms<float, access::read>	myTexture [[texture(0)]]
)
{
	return myTexture.read( uint2( inPs.gl_FragCoord.xy ), 0 ).x;
}
