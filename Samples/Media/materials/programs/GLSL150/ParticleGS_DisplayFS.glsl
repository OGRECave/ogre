#version 150

in vec4 colour;
out vec4 fragColour;
uniform sampler2D diffuseTex;

// Colours the fireworks.
void main()
{
    fragColour = texture(diffuseTex, gl_PointCoord.st) * colour;
}
