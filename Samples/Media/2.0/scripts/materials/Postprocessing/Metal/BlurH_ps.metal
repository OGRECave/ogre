//-------------------------------
// Horizontal Gaussian-Blur pass
//-------------------------------

#include <metal_stdlib>
using namespace metal;

// Simple blur filter

//We use the Normal-gauss distribution formula
//f(x) being the formula, we used f(0.5)-f(-0.5); f(1.5)-f(0.5)...
constexpr constant float samples[11] =
{//stddev=2.0
	0.01222447,
	0.02783468,
	0.06559061,
	0.12097757,
	0.17466632,

	0.19741265,

	0.17466632,
	0.12097757,
	0.06559061,
	0.02783468,
	0.01222447
};

constexpr constant float offsets[11] =
{
	-5.0, -4.0, -3.0, -2.0, -1.0,
	 0.0,
	 1.0,  2.0,  3.0,  4.0,  5.0
};


struct PS_INPUT
{
	float2 uv0;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	Blur0			[[texture(0)]],
	sampler				samplerState	[[sampler(0)]]
)
{
	float2 uv = inPs.uv0.xy;
	uv.x += offsets[0] * 0.01;

	float4 sum = Blur0.sample( samplerState, uv ) * samples[0];

	for( int i=1; i<11; ++i)
	{
		uv = inPs.uv0.xy;
		uv.x += offsets[i] * 0.01;
		sum += Blur0.sample( samplerState, uv ) * samples[i];
	}

	return sum;
}
