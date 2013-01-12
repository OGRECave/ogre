#version 150

in vec2 iTexCoord;
in vec4 iColor;
out vec4 fragColour;

uniform sampler2D diffuseTex;

//The pixels shaders that colors the fireworks
void main()
{
	fragColour = texture(diffuseTex, iTexCoord) * iColor;
}
