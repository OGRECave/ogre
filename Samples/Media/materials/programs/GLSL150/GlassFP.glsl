#version 150

uniform sampler2D RT;
uniform sampler2D NormalMap;

in vec2 oUv0;
out vec4 fragColour;

void main()
{
	vec4 normal = 2.0 * (texture(NormalMap, oUv0 * 2.5) - 0.5);

	fragColour = texture(RT, oUv0 + normal.xy * 0.05);
}
