#version 120

varying vec3 oUv;

uniform mat4 worldViewProj;

void main(void)
{
	gl_Position = worldViewProj * gl_Vertex;
	oUv = gl_MultiTexCoord0.xyz;
}
