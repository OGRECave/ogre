//All of these functions are based on dqs.cg from http://isg.cs.tcd.ie/kavanl/dq/
/* dqs.cg

  Dual quaternion skinning vertex shaders (no shading computations)

  Version 1.0.3, November 1st, 2007

  Copyright (C) 2006-2007 University of Dublin, Trinity College, All Rights 
  Reserved

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author(s) be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Author: Ladislav Kavan, kavanl@cs.tcd.ie

*/

float2x4 blendTwoWeights(float4 blendWgt, float4 blendIdx, float2x4 dualQuaternions[24])
{
	float2x4 blendDQ = blendWgt.x*dualQuaternions[blendIdx.x];
	blendDQ += blendWgt.y*dualQuaternions[blendIdx.y];

	return blendDQ;
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

float3 calculateBlendPosition(float3 position, float2x4 blendDQ)
{
	float3 blendPosition = position + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, position.xyz) + blendDQ[0].x*position);
	float3 trans = 2.0*(blendDQ[0].x*blendDQ[1].yzw - blendDQ[1].x*blendDQ[0].yzw + cross(blendDQ[0].yzw, blendDQ[1].yzw));
	blendPosition += trans;

	return blendPosition;
}

float3 calculateBlendNormal(float3 normal, float2x4 blendDQ)
{
	return normal + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, normal) + blendDQ[0].x*normal);
}

float3x3 adjointTransposeMatrix(float3x4 M)
{
	float3x3 atM;
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

