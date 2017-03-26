
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

uniform sampler2D rt_input;  //Must not be sRGB
#if SMAA_PREDICATION
	uniform sampler2D depthTex;
#endif

void main()
{
	#if SMAA_PREDICATION
		gl_FragColor.xy = SMAAColorEdgeDetectionPS( inPs.uv0, inPs.offset, rt_input, depthTex );
	#else
		gl_FragColor.xy = SMAAColorEdgeDetectionPS( inPs.uv0, inPs.offset, rt_input );
	#endif
}
