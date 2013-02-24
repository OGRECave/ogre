#version 150

uniform vec4 ambient;
uniform mat4 worldViewProj;

in vec4 position;
in vec4 uv0;

out vec4 ambColour;
out vec4 ambientUV;

/*
  Basic ambient lighting vertex program for GLSL
*/
void main()
{
	gl_Position = worldViewProj * position;
	ambColour = ambient;
	ambientUV = uv0;
}
