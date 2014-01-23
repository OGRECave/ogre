#include "DualQuaternion_Common.hlsl"


struct v2p
{
	float4 oPosition : SV_POSITION;
	float2 oUv       : TEXCOORD0;
	float4 colour    : COLOR;
};

struct dualQuaternionHardwareSkinningTwoWeights_vp_in
{
	float4 position : SV_POSITION;
	float3 normal   : NORMAL;
	float2 uv       : TEXCOORD0;
	float4 blendIdx : BLENDINDICES;
	float4 blendWgt : BLENDWEIGHT;
};
//Dual quaternion skinning with per-vertex antipodality handling (this is the most robust, but not the most efficient way):
v2p dualQuaternionHardwareSkinningTwoWeights_vp(	
	dualQuaternionHardwareSkinningTwoWeights_vp_in input,
	uniform float2x4 worldDualQuaternion2x4Array[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   lightPos[2],
	uniform float4   lightDiffuseColour[2],
	uniform float4   ambient,
	uniform float4   diffuse)
{		
	v2p output;
	float2x4 blendDQ = blendTwoWeightsAntipod(input.blendWgt, input.blendIdx, worldDualQuaternion2x4Array);

	float len = length(blendDQ[0]);
	blendDQ /= len;

	float3 blendPosition = calculateBlendPosition(input.position.xyz, blendDQ);

	//No need to normalize, the magnitude of the normal is preserved because only rotation is performed
	float3 blendNormal = calculateBlendNormal(input.normal, blendDQ);
	
	output.oPosition = mul(viewProjectionMatrix, float4(blendPosition, 1.0));
	
	// Lighting - support point and directional
	float3 lightDir0 = normalize(lightPos[0].xyz - (blendPosition.xyz * lightPos[0].w));
	float3 lightDir1 = normalize(lightPos[1].xyz - (blendPosition.xyz * lightPos[1].w));

	output.oUv = input.uv;

	output.colour = diffuse * (ambient + (saturate(dot(lightDir0, blendNormal)) * lightDiffuseColour[0]) + 
		(saturate(dot(lightDir1, blendNormal)) * lightDiffuseColour[1]));		
		
	return output;
}

struct dualQuaternionHardwareSkinningTwoWeightsCaster_vp_in
{
	float4 position : SV_POSITION;
	float4 blendIdx : BLENDINDICES;
	float4 blendWgt : BLENDWEIGHT;
};

struct v2ps
{
	float4 oPosition : SV_POSITION;
	float4 colour    : COLOR;
};
//Shadow caster pass
v2ps dualQuaternionHardwareSkinningTwoWeightsCaster_vp(
	dualQuaternionHardwareSkinningTwoWeightsCaster_vp_in input,
	uniform float2x4 worldDualQuaternion2x4Array[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   ambient)
{
	v2ps output;
	float2x4 blendDQ = blendTwoWeightsAntipod(input.blendWgt, input.blendIdx, worldDualQuaternion2x4Array);

	float len = length(blendDQ[0]);
	blendDQ /= len;

	float3 blendPosition = calculateBlendPosition(input.position.xyz, blendDQ);

	// view / projection
	output.oPosition = mul(viewProjectionMatrix, float4(blendPosition, 1.0));
	
	output.colour = ambient;
	return output;
}

//Two-phase skinning: dual quaternion skinning with scale and shear transformations
v2p dualQuaternionHardwareSkinningTwoWeightsTwoPhase_vp(
	dualQuaternionHardwareSkinningTwoWeights_vp_in input,
	uniform float2x4 worldDualQuaternion2x4Array[24],
	uniform float3x4 scaleM[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   lightPos[2],
	uniform float4   lightDiffuseColour[2],
	uniform float4   ambient,
	uniform float4   diffuse)
{
	v2p output;
	//First phase - applies scaling and shearing:
	float3x4 blendS = input.blendWgt.x*scaleM[input.blendIdx.x];
	blendS += input.blendWgt.y*scaleM[input.blendIdx.y];	
		
	float3 pass1_position = mul(blendS, input.position);
	float3x3 blendSrotAT = adjointTransposeMatrix(blendS);
	float3 pass1_normal = normalize(mul(blendSrotAT, input.normal.xyz));

	//Second phase
	float2x4 blendDQ = blendTwoWeightsAntipod(input.blendWgt, input.blendIdx, worldDualQuaternion2x4Array);
	
	float len = length(blendDQ[0]);
	blendDQ /= len;

	float3 blendPosition = calculateBlendPosition(pass1_position, blendDQ);

	//No need to normalize, the magnitude of the normal is preserved because only rotation is performed
	float3 blendNormal = calculateBlendNormal(pass1_normal, blendDQ);

	output.oPosition = mul(viewProjectionMatrix, float4(blendPosition, 1.0));

	// Lighting - support point and directional
	float3 lightDir0 = normalize(lightPos[0].xyz - (blendPosition.xyz * lightPos[0].w));
	float3 lightDir1 = normalize(lightPos[1].xyz - (blendPosition.xyz * lightPos[1].w));

	output.oUv = input.uv;
	output.colour = diffuse * (ambient + (saturate(dot(lightDir0, blendNormal)) * lightDiffuseColour[0]) + 
		(saturate(dot(lightDir1, blendNormal)) * lightDiffuseColour[1]));
	return output;
}

//Two-phase skinning shadow caster pass
v2ps dualQuaternionHardwareSkinningTwoWeightsTwoPhaseCaster_vp(
	dualQuaternionHardwareSkinningTwoWeightsCaster_vp_in input,
	uniform float2x4 worldDualQuaternion2x4Array[24],
	uniform float3x4 scaleM[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   ambient)
{
	v2ps output;
	//First phase - applies scaling and shearing:
	float3x4 blendS = input.blendWgt.x*scaleM[input.blendIdx.x];
	blendS += input.blendWgt.y*scaleM[input.blendIdx.y];	
		
	float3 pass1_position = mul(blendS, input.position);

	//Second phase
	float2x4 blendDQ = blendTwoWeightsAntipod(input.blendWgt, input.blendIdx, worldDualQuaternion2x4Array);
	
	float len = length(blendDQ[0]);
	blendDQ /= len;

	float3 blendPosition = calculateBlendPosition(pass1_position, blendDQ);

	// view / projection
	output.oPosition = mul(viewProjectionMatrix, float4(blendPosition, 1.0));
	
	output.colour = ambient;
	return output;
}

