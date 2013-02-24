#version 150

uniform sampler2D RT;
uniform sampler2D noise;
in vec2 oUv0;

out vec4 fragColour;

void main()
{
	float c = dot(texture(RT, oUv0), vec4(0.30, 0.11, 0.59, 0.00));
	float n = texture(noise, oUv0).r * 2.0 - 1.0;
	c += n;

	c = step(c, 0.5);

	fragColour = vec4(c,c,c,1.0);
}
