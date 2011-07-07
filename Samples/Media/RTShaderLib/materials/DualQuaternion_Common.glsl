#version 120

mat2x4 blendTwoWeights(vec4 blendWgt, vec4 blendIdx, mat4x2 dualQuaternions[24])
{
	mat4x2 blendDQ = blendWgt.x*dualQuaternions[int(blendIdx.x)];
	blendDQ += blendWgt.y*dualQuaternions[int(blendIdx.y)];

	return transpose(blendDQ);
}

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat4x2 dualQuaternions[24])
{
	mat2x4 dq0 = transpose(dualQuaternions[int(blendIdx.x)]);
	mat2x4 dq1 = transpose(dualQuaternions[int(blendIdx.y)]);

	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	
	mat2x4 blendDQ = blendWgt.x*dq0;
	blendDQ += blendWgt.y*dq1;

	return blendDQ;
}

mat2x4 blendThreeWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat4x2 dualQuaternions[24])
{
	mat2x4 dq0 = transpose(dualQuaternions[int(blendIdx.x)]);
	mat2x4 dq1 = transpose(dualQuaternions[int(blendIdx.y)]);
	mat2x4 dq2 = transpose(dualQuaternions[int(blendIdx.z)]);

	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	
	mat2x4 blendDQ = blendWgt.x*dq0;
	blendDQ += blendWgt.y*dq1;

	return blendDQ;
}

vec3 calculateBlendPosition(vec4 position, mat2x4 blendDQ)
{
	vec3 blendPosition = position.xyz + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, position.xyz) + blendDQ[0].x*position.xyz);
	vec3 trans = 2.0*(blendDQ[0].x*blendDQ[1].yzw - blendDQ[1].x*blendDQ[0].yzw + cross(blendDQ[0].yzw, blendDQ[1].yzw));
	blendPosition += trans;

	return blendPosition;
}

vec3 calculateBlendNormal(vec3 normal, mat2x4 blendDQ)
{
	return normal + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, normal) + blendDQ[0].x*normal);
}

