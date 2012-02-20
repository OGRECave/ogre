#version 120

attribute vec4 position;

varying vec3 oUv;

uniform mat4 worldViewProj;

void main(void)
{
	gl_Position = worldViewProj * position;
	oUv = gl_MultiTexCoord0.xyz;
}
