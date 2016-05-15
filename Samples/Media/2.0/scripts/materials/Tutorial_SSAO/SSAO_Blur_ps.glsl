#version 330

uniform sampler2D ssaoTexture;

in block
{
	vec2 uv0;
} inPs;

uniform vec2 texelSize;

out float fragColour;

void main()
{
	float result = 0.0;
	for (int x = -2; x < 2; ++x)
	{
		for (int y = -2; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssaoTexture, inPs.uv0 + offset).r;
		}
	}
	result = result / (4.0 * 4.0);

	fragColour = result;
}
