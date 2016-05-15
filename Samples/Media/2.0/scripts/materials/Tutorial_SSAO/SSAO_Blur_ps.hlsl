struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;

};

Texture2D<float> ssaoTexture	: register(t0);

SamplerState samplerState0		: register(s0);

uniform float2 texelSize;

float main
(
	PS_INPUT inPs
) : SV_Target
{
	float result = 0.0;
	for (int x = -2; x < 2; ++x)
	{
		for (int y = -2; y < 2; ++y)
		{
			float2 offset = float2(float(x), float(y)) * texelSize;
			result += ssaoTexture.Sample(samplerState0, inPs.uv0 + offset).r;
		}
	}
	result = result / (4.0 * 4.0);

	return result;
}
