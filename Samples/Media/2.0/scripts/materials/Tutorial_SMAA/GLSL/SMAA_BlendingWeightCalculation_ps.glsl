
#include "SMAA_GLSL.glsl"
#define SMAA_INCLUDE_VS 0
#define SMAA_INCLUDE_PS 1

#include "SMAA.hlsl"

in block
{
	vec2 uv0;
	vec2 pixcoord0;
	vec4 offset[3];
} inPs;

uniform sampler2D edgeTex;
uniform sampler2D areaTex;
uniform sampler2D searchTex;

void main()
{
	gl_FragColor = SMAABlendingWeightCalculationPS( inPs.uv0, inPs.pixcoord0, inPs.offset,
													edgeTex, areaTex, searchTex,
													vec4( 0, 0, 0, 0 ) );
}
