#version 330

uniform sampler2D RT;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
    vec4 Color;
    Color.a = 1.0;
    Color.rgb = vec3(0.5);
	Color -= texture( RT, inPs.uv0 - 0.001 ) * 2.0;
	Color += texture( RT, inPs.uv0 + 0.001 ) * 2.0;
    Color.rgb = vec3((Color.r+Color.g+Color.b)/3.0);
    fragColour = Color;
}
