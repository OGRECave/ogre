struct PS_INPUT
{
	float3 cameraDir	: TEXCOORD0;
};

Texture3D<float3> skyCubemap	: register(t0);
SamplerState samplerState		: register(s0);

float3 main
(
	PS_INPUT inPs
) : SV_Target0
{
	return skyCubemap.Sample( samplerState, inPs.cameraDir ).xyz;
}
