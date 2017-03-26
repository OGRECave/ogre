
#version 410 core

#include "SMAA_GLSL.glsl"
#define SMAA_INCLUDE_VS 0
#define SMAA_INCLUDE_PS 1

#include "SMAA.hlsl"

in block
{
	vec2 uv0;
	vec4 offset[3];
} inPs;

uniform sampler2D rt_input;  //Must not be be sRGB
#if SMAA_PREDICATION
	uniform sampler2D depthTex;
#endif

/*float toSRGB( float3 x )
{
	if (x <= 0.0031308)
		return 12.92 * x;
	else
		return 1.055 * pow( x,(1.0 / 2.4) ) - 0.055;
}

float fromSRGB( float3 x )
{
	if( x <= 0.040449907 )
		return x / 12.92;
	else
		return pow( (x + 0.055) / 1.055, 2.4 );
}*/

void main()
{
	#if SMAA_PREDICATION
		gl_FragColor.xy = SMAAColorEdgeDetectionPS( inPs.uv0, inPs.offset, rt_input, depthTex );
	#else
		gl_FragColor.xy = SMAAColorEdgeDetectionPS( inPs.uv0, inPs.offset, rt_input );
	#endif
}
