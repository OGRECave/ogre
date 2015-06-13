#version 330

uniform sampler2D RT;
uniform sampler2D Sum;

uniform float blur;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
	vec4 render = texture( RT, inPs.uv0 );
	vec4 sum = texture( Sum, inPs.uv0 );

	fragColour = mix( render, sum, blur );
}
