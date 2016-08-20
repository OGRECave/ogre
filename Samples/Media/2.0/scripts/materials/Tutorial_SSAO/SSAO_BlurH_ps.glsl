#version 330

uniform sampler2D ssaoTexture;
uniform sampler2D depthTexture;

in block
{
	vec2 uv0;
} inPs;

uniform vec2 texelSize;
uniform vec2 projectionParams;

const float offsets[9] = float[9]( -8.0, -6.0, -4.0, -2.0, 0.0, 2.0, 4.0, 6.0, 8.0 );

out float fragColour;

float getLinearDepth(vec2 uv)
{
	float fDepth = texture(depthTexture, uv).x;
	float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
	return linearDepth;
}

void main()
{
	float flDepth = getLinearDepth(inPs.uv0);
	
	float weights = 0.0;
	float result = 0.0;

	for (int i = 0; i < 9; ++i)
	{
		vec2 offset = vec2(texelSize.x*offsets[i], 0.0); //Horizontal sample offsets
		vec2 samplePos = inPs.uv0 + offset;

		float slDepth = getLinearDepth(samplePos);

		float weight = (1.0 / (abs(flDepth - slDepth) + 0.0001)); //Calculate weight using depth

		result += texture(ssaoTexture, samplePos).x*weight;

		weights += weight;
	}
	result /= weights;

	fragColour = result;

	//fragColour = texture(ssaoTexture, inPs.uv0).x; //Use this to disable blur
}