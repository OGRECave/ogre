mat2x4 blendTwoWeights(vec4 blendWgt, vec4 blendIdx, mat2x4 dualQuaternions[24])
{
	mat2x4 blendDQ = blendWgt.x*dualQuaternions[int(blendIdx.x)];
	blendDQ += blendWgt.y*dualQuaternions[int(blendIdx.y)];

	return blendDQ;
}

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat2x4 dualQuaternions[24])
{
	mat2x4 dq0 = dualQuaternions[int(blendIdx.x)];
	mat2x4 dq1 = dualQuaternions[int(blendIdx.y)];

	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	
	mat2x4 blendDQ = blendWgt.x*dq0;
	blendDQ += blendWgt.y*dq1;

	return blendDQ;
}

mat2x4 blendThreeWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat2x4 dualQuaternions[24])
{
	mat2x4 dq0 = dualQuaternions[int(blendIdx.x)];
	mat2x4 dq1 = dualQuaternions[int(blendIdx.y)];
	mat2x4 dq2 = dualQuaternions[int(blendIdx.z)];

	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	if (dot(dq0[0], dq2[0]) < 0.0) dq2 *= -1.0;
	
	mat2x4 blendDQ = blendWgt.x*dq0;
	blendDQ += blendWgt.y*dq1;
	blendDQ += blendWgt.z*dq2;
	
	return blendDQ;
}

mat2x4 blendFourWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat2x4 dualQuaternions[24])
{
	mat2x4 dq0 = dualQuaternions[int(blendIdx.x)];
	mat2x4 dq1 = dualQuaternions[int(blendIdx.y)];
	mat2x4 dq2 = dualQuaternions[int(blendIdx.z)];
	mat2x4 dq3 = dualQuaternions[int(blendIdx.w)];

	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	if (dot(dq0[0], dq2[0]) < 0.0) dq2 *= -1.0;
	if (dot(dq0[0], dq3[0]) < 0.0) dq3 *= -1.0;
	
	mat2x4 blendDQ = blendWgt.x*dq0;
	blendDQ += blendWgt.y*dq1;
	blendDQ += blendWgt.z*dq2;
	blendDQ += blendWgt.w*dq3;

	return blendDQ;
}

vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ)
{
	vec3 blendPosition = position + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, position) + blendDQ[0].x*position);
	vec3 trans = 2.0*(blendDQ[0].x*blendDQ[1].yzw - blendDQ[1].x*blendDQ[0].yzw + cross(blendDQ[0].yzw, blendDQ[1].yzw));
	blendPosition += trans;

	return blendPosition;
}

vec3 calculateBlendNormal(vec3 normal, mat2x4 blendDQ)
{
	return normal + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, normal) + blendDQ[0].x*normal);
}

mat3 adjointTransposeMatrix(mat3 M)
{
	mat3 atM;
	atM._m00 = M._m22 * M._m11 - M._m12 * M._m21;
	atM._m01 = M._m12 * M._m20 - M._m10 * M._m22;
	atM._m02 = M._m10 * M._m21 - M._m20 * M._m11;

	atM._m10 = M._m02 * M._m21 - M._m22 * M._m01;
	atM._m11 = M._m22 * M._m00 - M._m02 * M._m20;
	atM._m12 = M._m20 * M._m01 - M._m00 * M._m21;

	atM._m20 = M._m12 * M._m01 - M._m02 * M._m11;
	atM._m21 = M._m10 * M._m02 - M._m12 * M._m00;
	atM._m22 = M._m00 * M._m11 - M._m10 * M._m01;

	return atM;
}
