#version 330

uniform sampler2D tex;

in block
{
	vec2 uv0;
} inPs;

out vec4 fragColour;

void main()
{
	fragColour = texture( tex, inPs.uv0 );
}
