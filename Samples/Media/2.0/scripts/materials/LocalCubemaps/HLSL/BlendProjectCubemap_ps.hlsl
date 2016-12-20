struct PS_INPUT
{
	float3 posLS		: TEXCOORD0;
	float4 gl_Position	: SV_Position;
};

TextureCube<float4>	cubeTexture : register(t0);
SamplerState		cubeSampler : register(s0);

float4 main
(
	PS_INPUT inPs,
	uniform float weight,
	uniform float lodLevel
) : SV_Target
{
	return cubeTexture.SampleLevel( cubeSampler, inPs.posLS, lodLevel ).xyzw * weight;
}
