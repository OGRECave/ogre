#version 100

precision mediump int;
precision mediump float;

uniform vec4 ambient;
uniform mat4 worldViewProj;

attribute vec4 position;

varying vec4 colour;

/*
  Basic ambient lighting vertex program for GLSL ES
*/
void main()
{
	gl_Position = worldViewProj * position;
	colour = ambient;
}
