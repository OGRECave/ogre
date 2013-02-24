#version 150

in vec2 NDotV;
out vec4 fragColour;

void main()
{
   fragColour = vec4(NDotV.x / 2.0);
}
