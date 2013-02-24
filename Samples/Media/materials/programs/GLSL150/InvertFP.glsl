#version 150

uniform sampler2D RT;
in vec2 oUv0;
in vec2 oUv1;
out vec4 fragColour;

void main()
{
	fragColour = 1.0 - texture(RT, oUv0);
}
