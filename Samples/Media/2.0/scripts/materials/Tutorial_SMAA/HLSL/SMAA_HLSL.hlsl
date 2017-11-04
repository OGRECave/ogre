#ifndef SMAA_RT_METRICS
	#define SMAA_RT_METRICS viewportSize.zwxy
#endif

#define SMAA_EXTRA_PARAM_ARG_DECL , float4 viewportSize
#define SMAA_EXTRA_PARAM_ARG , viewportSize

#if !SMAA_INITIALIZED
	//Leave compatible defaults in case this file gets compiled
	//before calling SmaaUtils::initialize from C++
	#define SMAA_PRESET_ULTRA 1
	#define SMAA_HLSL_4 1
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

float toSRGB( float x )
{
	return (x < 0.0031308 ? x * 12.92 : 1.055 * pow( x, 0.41666 ) - 0.055 );
}

float fromSRGB( float x )
{
	return (x <= 0.040449907) ? x / 12.92 : pow( (x + 0.055) / 1.055, 2.4 );
}

float4 toSRGB( float4 x )
{
	return float4( toSRGB( x.x ), toSRGB( x.y ), toSRGB( x.z ), x.w );
}

float4 fromSRGB( float4 x )
{
    return float4( fromSRGB( x.x ), fromSRGB( x.y ), fromSRGB( x.z ), x.w );
}

#include "SMAA.hlsl"
