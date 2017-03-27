
#include "SMAA_GLSL.glsl"
#define SMAA_INCLUDE_VS 1
#define SMAA_INCLUDE_PS 0

#include "SMAA.hlsl"

in vec4 vertex;
in vec2 uv0;
uniform mat4 worldViewProj;

out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec2 uv0;
	vec4 offset[3];
} outVs;

void main()
{
	gl_Position = worldViewProj * vertex;
	outVs.uv0 = uv0.xy;
	SMAAEdgeDetectionVS( uv0.xy, outVs.offset );
}
