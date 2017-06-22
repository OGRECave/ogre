#version 120

attribute vec4 position;
attribute vec3 tangent;

varying vec4 oUv0;

uniform mat4 worldViewProj;

void main()
{
	gl_Position = worldViewProj * position;
	oUv0 = vec4(tangent, 1.0);
}
