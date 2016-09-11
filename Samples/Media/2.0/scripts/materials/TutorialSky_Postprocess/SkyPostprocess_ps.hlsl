struct PS_INPUT
{
	float3 cameraDir	: TEXCOORD0;
};

TextureCube<float3> skyCubemap	: register(t0);
SamplerState samplerState		: register(s0);

float3 main
(
	PS_INPUT inPs
) : SV_Target0
{
	//Cubemaps are left-handed
	return skyCubemap.Sample( samplerState, float3( inPs.cameraDir.xy, -inPs.cameraDir.z ) ).xyz;
}
