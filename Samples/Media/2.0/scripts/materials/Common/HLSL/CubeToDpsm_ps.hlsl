
TextureCube<float>	depthTexture : register(t0);
SamplerState		samplerState : register(s0);

struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
};

float main
(
	PS_INPUT inPs
#if OUTPUT_TO_COLOUR
) : SV_Target0
#else
) : SV_Depth
#endif
{
	float3 cubeDir;

	cubeDir.x = fmod( inPs.uv0.x, 0.5 ) * 4.0 - 1.0;
	cubeDir.y = inPs.uv0.y * 2.0 - 1.0;
	cubeDir.z = 0.5 - 0.5 * (cubeDir.x * cubeDir.x + cubeDir.y * cubeDir.y);

	cubeDir.z = inPs.uv0.x < 0.5 ? cubeDir.z : -cubeDir.z;

	float depthValue = depthTexture.SampleLevel( samplerState, cubeDir.xyz, 0 ).x;

	return depthValue;
}
