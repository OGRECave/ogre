
#include "SMAA_GLSL.glsl"
#define SMAA_INCLUDE_VS 0
#define SMAA_INCLUDE_PS 1

#include "SMAA.hlsl"

in block
{
	vec2 uv0;
	vec4 offset;
} inPs;

uniform sampler2D rt_input; //Can be sRGB
uniform sampler2D blendTex;
#if SMAA_REPROJECTION
	uniform sampler2D velocityTex;
#endif

void main()
{
#if SMAA_REPROJECTION
	gl_FragColor = SMAANeighborhoodBlendingPS( inPs.uv0, inPs.offset,
											   rt_input, blendTex, velocityTex );
#else
	gl_FragColor = SMAANeighborhoodBlendingPS( inPs.uv0, inPs.offset,
											   rt_input, blendTex );
#endif
}
