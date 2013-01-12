#version 150

in vec4 colour;
out vec4 fragColour;

/*
  Basic ambient lighting fragment program for GLSL ES
*/
void main()
{
	fragColour = colour;
}
