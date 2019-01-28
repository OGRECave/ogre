#version 120

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

mat2x4 blendTwoWeights(vec4 blendWgt, vec4 blendIdx, vec4 dualQuaternions[48])
{
	mat2x4 blendDQ = blendWgt.x*mat2x4(dualQuaternions[int(blendIdx.x) * 2], dualQuaternions[int(blendIdx.x) * 2 + 1]);
	blendDQ += blendWgt.y*mat2x4(dualQuaternions[int(blendIdx.y) * 2], dualQuaternions[int(blendIdx.y) * 2 + 1]);

	return blendDQ;
}

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, vec4 dualQuaternions[48])
{
	mat2x4 dq0 = mat2x4(dualQuaternions[int(blendIdx.x) * 2], dualQuaternions[int(blendIdx.x) * 2 + 1]);
	mat2x4 dq1 = mat2x4(dualQuaternions[int(blendIdx.y) * 2], dualQuaternions[int(blendIdx.y) * 2 + 1]);

	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	
	mat2x4 blendDQ = blendWgt.x*dq0;
	blendDQ += blendWgt.y*dq1;

	return blendDQ;
}

mat2x4 blendThreeWeightsAntipod(vec4 blendWgt, vec4 blendIdx, vec4 dualQuaternions[48])
{
	mat2x4 dq0 = mat2x4(dualQuaternions[int(blendIdx.x) * 2], dualQuaternions[int(blendIdx.x) * 2 + 1]);
	mat2x4 dq1 = mat2x4(dualQuaternions[int(blendIdx.y) * 2], dualQuaternions[int(blendIdx.y) * 2 + 1]);
	mat2x4 dq2 = mat2x4(dualQuaternions[int(blendIdx.z) * 2], dualQuaternions[int(blendIdx.z) * 2 + 1]);

	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	if (dot(dq0[0], dq2[0]) < 0.0) dq2 *= -1.0;
	
	mat2x4 blendDQ = blendWgt.x*dq0;
	blendDQ += blendWgt.y*dq1;
	blendDQ += blendWgt.z*dq2;
	
	return blendDQ;
}

mat2x4 blendFourWeightsAntipod(vec4 blendWgt, vec4 blendIdx, vec4 dualQuaternions[48])
{
	mat2x4 dq0 = mat2x4(dualQuaternions[int(blendIdx.x) * 2], dualQuaternions[int(blendIdx.x) * 2 + 1]);
	mat2x4 dq1 = mat2x4(dualQuaternions[int(blendIdx.y) * 2], dualQuaternions[int(blendIdx.y) * 2 + 1]);
	mat2x4 dq2 = mat2x4(dualQuaternions[int(blendIdx.z) * 2], dualQuaternions[int(blendIdx.z) * 2 + 1]);
	mat2x4 dq3 = mat2x4(dualQuaternions[int(blendIdx.w) * 2], dualQuaternions[int(blendIdx.w) * 2 + 1]);

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
