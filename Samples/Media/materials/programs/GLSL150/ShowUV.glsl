#version 150

in vec4 ambientUV;
in vec4 ambColour;

out vec4 fragColour;

// Basic fragment program to display UV
void main()
{
	// wrap values using fract
	fragColour = vec4(fract(ambientUV.x), fract(ambientUV.y), 0.0, 1.0);
}
