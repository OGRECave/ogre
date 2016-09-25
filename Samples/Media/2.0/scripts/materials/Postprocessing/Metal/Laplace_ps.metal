#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct Params
{
	float scale;
	float pixelSize;
};

constexpr constant float2 samples[4] =
{
	float2(  0, -1 ),
	float2( -1,  0 ),
	float2(  1,  0 ),
	float2(  0,  1 )
};

// The Laplace filter approximates the second order derivate,
// that is, the rate of change of slope in the image. It can be
// used for edge detection. The Laplace filter gives negative
// response on the higher side of the edge and positive response
// on the lower side.

// This is the filter kernel:
// 0  1  0
// 1 -4  1
// 0  1  0

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	Image			[[texture(0)]],
	sampler				samplerState	[[sampler(0)]],

	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
	float4 laplace = -4 * Image.sample( samplerState, inPs.uv0 );

	// Sample the neighbor pixels
	for (int i = 0; i < 4; i++)
		laplace += Image.sample( samplerState, inPs.uv0 + p.pixelSize * samples[i] );

	return (0.5 + p.scale * laplace);
}
