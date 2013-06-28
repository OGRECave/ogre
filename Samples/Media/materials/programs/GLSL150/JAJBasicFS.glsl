#version 150

out vec4 fragColour;

uniform vec4 ColourMe[2];

// Pixel shader
void main()
{
  fragColour = ColourMe[0] + ColourMe[1];
  //fragColour = vec4(1, 1, 1, 1);
}
