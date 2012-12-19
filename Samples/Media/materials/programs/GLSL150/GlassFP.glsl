#version 150

uniform sampler2D RT;
uniform sampler2D NormalMap;

in vec2 uv;
out vec4 fragColour;

void main()
{
	vec4 normal = 2.0 * (texture(NormalMap, uv * 2.5) - 0.5);

	fragColour = texture(RT, uv + normal.xy * 0.05);
}
