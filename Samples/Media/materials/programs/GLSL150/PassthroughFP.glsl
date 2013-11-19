#version 150

in vec4 colour;
out vec4 fragColour;

/*
  Basic ambient lighting fragment program for GLSL.
*/
void main()
{
    // fragColour = colour;
    fragColour = vec4(255, 255, 255, 1);
}
