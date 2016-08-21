struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
};

Texture2D<float> ssaoTexture	: register(t0);
Texture2D<float> depthTexture	: register(t1);

SamplerState samplerState		: register(s0);

uniform float2 projectionParams;
uniform float4 texelSize;

static const float offsets[9] = { -8.0, -6.0, -4.0, -2.0, 0.0, 2.0, 4.0, 6.0, 8.0 };

float getLinearDepth(float2 uv)
{
	float fDepth = depthTexture.Sample(samplerState, uv).x;
	float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
	return linearDepth;
}

float main
(
	PS_INPUT inPs
) : SV_Target
{
	float flDepth = getLinearDepth(inPs.uv0);
	
	float weights = 0.0;
	float result = 0.0;

	for (int i = 0; i < 9; ++i)
	{
		float2 offset = float2(texelSize.z*offsets[i], 0.0); //Horizontal sample offsets
		float2 samplePos = inPs.uv0 + offset;

		float slDepth = getLinearDepth(samplePos);

		float weight = (1.0 / (abs(flDepth - slDepth) + 0.0001));

		result += ssaoTexture.Sample(samplerState, samplePos).x*weight;

		weights += weight;
	}
	result /= weights;

	return result;
}
