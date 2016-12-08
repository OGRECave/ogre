float3 calculateBlendPosition(float4 position, float2x4 blendDQ)
{
	float3 blendPosition = position.xyz + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, position.xyz) + blendDQ[0].x*position.xyz);
	float3 trans = 2.0*(blendDQ[0].x*blendDQ[1].yzw - blendDQ[1].x*blendDQ[0].yzw + cross(blendDQ[0].yzw, blendDQ[1].yzw));
	blendPosition += trans;

	return blendPosition;
}
float2x4 blendTwoWeightsAntipod(float4 blendWgt, float4 blendIdx, float2x4 dualQuaternions[24])
{
	float2x4 dq0 = dualQuaternions[blendIdx.x];
	float2x4 dq1 = dualQuaternions[blendIdx.y];

	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	
	float2x4 blendDQ = blendWgt.x*dq0;
	blendDQ += blendWgt.y*dq1;

	return blendDQ;
}


void shadow_caster_dq_ps(
#ifdef D3D11
	in float4 iPosition : SV_POSITION
#else
	in float4 iPosition : POSITION
#endif
	,in float4 colour : COLOR
	,out float4 oColour : SV_TARGET
)
{	
	oColour = colour;
}

//Two-phase skinning shadow caster pass
void shadow_caster_dq_skinning_2weight_twophase_vs(
	in float4 position : POSITION,
	in uint4 blendIdx : BLENDINDICES,
	in float4 blendWgt : BLENDWEIGHT,
#ifdef D3D11
	out float4 oPosition : SV_POSITION,
#else
	out float4 oPosition : POSITION,
#endif
	out float4 colour    : COLOR,
	// Support up to 24 bones of float3x4
	// vs_1_1 only supports 96 params so more than this is not feasible
	uniform float2x4 worldDualQuaternion2x4Array[24],
	uniform float3x4 scaleM[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   ambient)
{
	//First phase - applies scaling and shearing:
	float3x4 blendS = blendWgt.x*scaleM[blendIdx.x];
	blendS += blendWgt.y*scaleM[blendIdx.y];	
		
	float3 pass1_position = mul(blendS, position);

	//Second phase
	float2x4 blendDQ = blendTwoWeightsAntipod(blendWgt, blendIdx, worldDualQuaternion2x4Array);
	
	float len = length(blendDQ[0]);
	blendDQ /= len;

	float3 blendPosition = calculateBlendPosition(float4(pass1_position, 1.0), blendDQ);

	// view / projection
	oPosition = mul(viewProjectionMatrix, float4(blendPosition, 1.0));
	
	colour = ambient;
}