#version 120

attribute vec3 uv0;
attribute vec4 vertex;
varying vec3 oUv;

uniform mat4 worldViewProj;

void main(void)
{
	gl_Position = worldViewProj * vertex;
	oUv = uv0;
}
