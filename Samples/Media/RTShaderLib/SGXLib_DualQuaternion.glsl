#version 120

void SGX_BlendWeight(in float blendWgt, in mat2x4 dualQuaternion, out mat2x4 vOut)
{
	vOut = blendWgt*dualQuaternion;
}

void SGX_BlendWeight(in float blendWgt, in mat3x4 scaleShearMatrix, out mat3x4 vOut)
{
	vOut = blendWgt*scaleShearMatrix;
}

void SGX_AntipodalityAdjustment(in mat2x4 dq0, in mat2x4 dq1,out mat2x4 dq2)
{
	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	dq2 = (dot(dq0[0], dq1[0]) < 0.0) ? dq1 * -1.0 : dq1;
}

void SGX_CalculateBlendPosition(in vec4 position, in mat2x4 blendDQ, out vec3 vOut)
{
	vec3 blendPosition = position.xyz + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, position.xyz) + blendDQ[0].x*position.xyz);
	vec3 trans = 2.0*(blendDQ[0].x*blendDQ[1].yzw - blendDQ[1].x*blendDQ[0].yzw + cross(blendDQ[0].yzw, blendDQ[1].yzw));
	blendPosition += trans;

	vOut = blendPosition;
}

void SGX_CalculateBlendNormal(in vec3 normal, in mat2x4 blendDQ, out vec3 vOut)
{
	vOut = normal + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, normal) + blendDQ[0].x*normal);
}

void SGX_NormalizeDualQuaternion(inout mat2x4 dq)
{
	dq /= length(dq[0]);
}

void SGX_AdjointTransposeMatrix(in mat3x3 M, out mat3x3 vOut)
{
	mat3x3 atM;
	atM._m00 = M._m22 * M._m11 - M._m12 * M._m21;
	atM._m01 = M._m12 * M._m20 - M._m10 * M._m22;
	atM._m02 = M._m10 * M._m21 - M._m20 * M._m11;

	atM._m10 = M._m02 * M._m21 - M._m22 * M._m01;
	atM._m11 = M._m22 * M._m00 - M._m02 * M._m20;
	atM._m12 = M._m20 * M._m01 - M._m00 * M._m21;

	atM._m20 = M._m12 * M._m01 - M._m02 * M._m11;
	atM._m21 = M._m10 * M._m02 - M._m12 * M._m00;
	atM._m22 = M._m00 * M._m11 - M._m10 * M._m01;

	vOut = atM;
}
