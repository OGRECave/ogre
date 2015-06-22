struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
	float3 cameraDir	: TEXCOORD1;
};

Texture2D<float> depthTexture	: register(t0);
SamplerState samplerState		: register(s0);

uniform float2 projectionParams;

float3 main
(
	PS_INPUT inPs
) : SV_Target0
{
	float fDepth = depthTexture.Sample( samplerState, inPs.uv0 ).x;
	
	float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
	
	float3 viewSpacePosition = inPs.cameraDir * linearDepth;
	
	//For this next line to work cameraPos would have to be an uniform in world space, and cameraDir
	//would have to be sent using the compositor setting "quad_normals camera_far_corners_world_space_centered"
	//float3 worldSpacePosition = inPs.cameraDir * linearDepth + cameraPos;
	
	//Scale the values down to get some useful colour debugging in the valid range
	return float3( viewSpacePosition.xy / 5, -viewSpacePosition.z / 80.0 );
	//return linearDepth.xxx;
}
