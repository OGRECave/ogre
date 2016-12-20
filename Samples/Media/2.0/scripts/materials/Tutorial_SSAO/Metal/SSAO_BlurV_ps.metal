#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct Params
{
	float2 projectionParams;
	float4 texelSize;
};

constant constexpr float offsets[9] = { -8.0, -6.0, -4.0, -2.0, 0.0, 2.0, 4.0, 6.0, 8.0 };

inline float getLinearDepth( float2 uv, constant const Params &p,
							 texture2d<float> depthTexture, sampler samplerState )
{
	float fDepth = depthTexture.sample( samplerState, uv ).x;
	float linearDepth = p.projectionParams.y / (fDepth - p.projectionParams.x);
	return linearDepth;
}

fragment float main_metal
(
	PS_INPUT inPs [[stage_in]],

	texture2d<float>	ssaoTexture		[[texture(0)]],
	texture2d<float>	depthTexture	[[texture(1)]],

	sampler				samplerState	[[sampler(0)]],

	constant Params &p					[[buffer(PARAMETER_SLOT)]]
)
{
	float flDepth = getLinearDepth( inPs.uv0, p, depthTexture, samplerState );

	float weights = 0.0;
	float result = 0.0;

	for( int i = 0; i < 9; ++i )
	{
		float2 offset = float2( 0.0, p.texelSize.w * offsets[i] ); //Vertical sample offsets
		float2 samplePos = inPs.uv0 + offset;

		float slDepth = getLinearDepth( samplePos, p, depthTexture, samplerState );

		float weight = ( 1.0 / (abs(flDepth - slDepth) + 0.0001) );

		result += ssaoTexture.sample( samplerState, samplePos ).x * weight;

		weights += weight;
	}

	result /= weights;

	return result;
}
