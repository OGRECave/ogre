uniform vec4 ambient;
uniform mat4 worldViewProj;

attribute vec4 position;

varying vec4 colour;

/*
  Basic ambient lighting vertex program for GLSL
*/
void main()
{
	gl_Position = worldViewProj * position;
	colour = ambient;
}
