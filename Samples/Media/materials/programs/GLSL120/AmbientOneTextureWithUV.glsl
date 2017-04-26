#version 120

uniform vec4 ambient;
uniform mat4 worldViewProj;

attribute vec4 position;
attribute vec4 uv0;

varying vec4 colour;
varying vec4 ambientUV;

/*
  Basic ambient lighting vertex program for GLSL
*/
void main()
{
	gl_Position = worldViewProj * position;
	colour = ambient;
	ambientUV = uv0;
}
