#version 450
layout(location=0) out vec4 fragColour;

layout(location=0) in vec3 colour;

void main(void)
{
    fragColour = vec4(colour, 1);
}
