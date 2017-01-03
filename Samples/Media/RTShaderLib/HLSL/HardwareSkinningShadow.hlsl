void shadow_caster_ps(
	out float4 oColour	: SV_TARGET)
{	
	oColour = 0.5;
}

void shadow_caster_skinning_2weight_vs(
	float4 position		 : POSITION,
#ifdef D3D11
	out float4 oPosition : SV_POSITION,
#else
	out float4 oPosition : POSITION,
#endif
	out float2 oDepthInfo : TEXCOORD0,
	uint4 blendIdx : BLENDINDICES,
	float4 blendWgt : BLENDWEIGHT,
	
	uniform float4x4 viewProjectionMatrix,
	uniform float3x4  worldMatrix3x4Array[80])
{
	// output position.
	oPosition.xyz = mul(worldMatrix3x4Array[blendIdx.x], position) * blendWgt.x +
				mul(worldMatrix3x4Array[blendIdx.y], position) * blendWgt.y;
	oPosition.w = 1;
	oPosition = mul(viewProjectionMatrix, oPosition);


	// depth info for the fragment.
	oDepthInfo.x = oPosition.z;
	oDepthInfo.y = oPosition.w;
}
