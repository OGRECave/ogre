//------------------------------------------------------------------
//Bloom
//------------------------------------------------------------------

#include <metal_stdlib>
using namespace metal;

inline float3 toSRGB( float3 x )
{
	return sqrt( x );
}

struct PS_INPUT
{
	float2 uv0;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>				rt0		[[texture(0)]],
	texture2d<float, access::read>	lumRt	[[texture(1)]],
	sampler				samplerBilinear		[[sampler(0)]],
	sampler				samplerPoint		[[sampler(1)]],

	//.x = 'low' threshold. All colour below this value is 0.
	//.y = 1 / (low - high). All colour above 'high' is at full brightness.
	//Colour between low and high is faded.
	constant float2 &brightThreshold [[buffer(PARAMETER_SLOT)]]
)
{
	//Perform a high-pass filter on the image
	float fInvLumAvg = lumRt.read( uint2( 0, 0 ) ).x;

	float3 vSample = rt0.sample( samplerBilinear, inPs.uv0 ).xyz;
	vSample.xyz *= fInvLumAvg;
	
	float3 w = saturate( (vSample.xyz - brightThreshold.xxx) * brightThreshold.yyy );
	vSample.xyz *= w * w * (3.0 - 2.0 * w);

	//We use RGB10 format which doesn't natively support sRGB, so we use a cheap approximation (gamma = 2)
	//This preserves a lot of quality. Don't forget to convert it back when sampling from the texture.
	return float4( toSRGB( vSample * 0.0625 ), 1.0 );
}
