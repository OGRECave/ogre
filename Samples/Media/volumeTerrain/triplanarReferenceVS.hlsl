struct v2p
{
	float4 oClipPos: SV_POSITION;
	float3 oPos: TEXCOORD0;
	float4 oNormAndFogVal: TEXCOORD1;
	float3 oEyePos: TEXCOORD2;
};

struct a2v
{
	float4 position: POSITION;
	float3 normal: NORMAL;
	
	#if FOGLINEAR || FOGEXPONENTIAL || FOGEXPONENTIAL2
	float4 fogParams;
	#endif
};

cbuffer MatrixBuffer
{
	matrix worldviewproj;
	float3 eyePosition;
};

v2p main_triplanar_reference_vp(a2v input)
{
	v2p output;
	
	output.oClipPos = mul(worldviewproj, input.position);
	
	output.oPos = input.position.xyz;
	output.oNormAndFogVal.xyz = input.normal;
	output.oEyePos = eyePosition;
		
	// Fog like in the terrain component, but exp2 added
	output.oNormAndFogVal.w = 0;
	#if FOGLINEAR
	output.oNormAndFogVal.w = saturate((output.oClipPos.z - input.fogParams.y) * input.fogParams.w);
	#endif
	#if FOGEXPONENTIAL
    // Fog density increases  exponentially from the camera (fog = 1/e^(distance * density))
	output.oNormAndFogVal.w = 1 - saturate(1 / (exp(output.oClipPos.z * input.fogParams.x)));
	#endif
	#if FOGEXPONENTIAL2
    // Fog density increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2)
	float distanceTimesDensity = exp(output.oClipPos.z * input.fogParams.x);
	output.oNormAndFogVal.w = 1 - saturate(1 / (distanceTimesDensity * distanceTimesDensity));
	#endif
	
	return output;
}
