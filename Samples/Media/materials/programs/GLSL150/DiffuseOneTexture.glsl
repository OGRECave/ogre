#version 150

uniform sampler2D texMap;

in vec4 colour;
in vec4 uv;

out vec4 fragColour;

/*
  Basic fragment program using texture and diffuse colour.
*/
void main()
{
	fragColour = texture(texMap, uv.xy) * colour;
}
