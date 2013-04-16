#version 150

uniform vec4 ambient;
uniform mat4 worldViewProj;

in vec4 position;

out vec4 colour;

/*
  Basic ambient lighting vertex program for GLSL
*/
void main()
{
	gl_Position = worldViewProj * position;
	colour = ambient;
}
