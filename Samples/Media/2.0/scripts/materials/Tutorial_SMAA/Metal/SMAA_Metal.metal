#include <metal_stdlib>
using namespace metal;

#ifndef SMAA_RT_METRICS
	#define SMAA_RT_METRICS viewportSize.zwxy
#endif

#define SMAA_EXTRA_PARAM_ARG_DECL , float4 viewportSize
#define SMAA_EXTRA_PARAM_ARG , viewportSize
#define SMAA_METAL 1

#if !SMAA_INITIALIZED
	//Leave compatible defaults in case this file gets compiled
	//before calling SmaaUtils::initialize from C++
	#define SMAA_PRESET_ULTRA 1
#endif

/*float toSRGB( float x )
{
	if (x <= 0.0031308)
		return 12.92 * x;
	else
		return 1.055 * pow( x,(1.0 / 2.4) ) - 0.055;
}

float fromSRGB( float x )
{
	if( x <= 0.040449907 )
		return x / 12.92;
	else
		return pow( (x + 0.055) / 1.055, 2.4 );
}*/

inline float toSRGB( float x )
{
	return (x < 0.0031308 ? x * 12.92 : 1.055 * pow( x, 0.41666 ) - 0.055 );
}

inline float fromSRGB( float x )
{
	return (x <= 0.040449907) ? x / 12.92 : pow( (x + 0.055) / 1.055, 2.4 );
}

float4 toSRGB( float4 x )
{
	return float4( toSRGB( x.x ), toSRGB( x.y ), toSRGB( x.z ), x.w );
}

inline float4 fromSRGB( float4 x )
{
	return float4( fromSRGB( x.x ), fromSRGB( x.y ), fromSRGB( x.z ), x.w );
}

#include "SMAA.metal"

struct Params
{
#if VERTEX_SHADER
	float4x4	worldViewProj;
#endif
	float4		viewportSize;
};

#if VERTEX_SHADER
struct VS_INPUT
{
	float4 position	[[attribute(VES_POSITION)]];
	float2 uv0		[[attribute(VES_TEXTURE_COORDINATES0)]];
};
#endif
