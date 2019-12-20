/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2014 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

//These functions are based on dqs.cg from http://isg.cs.tcd.ie/kavanl/dq/
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

//-----------------------------------------------------------------------------
// Program Name: SGXLib_DualQuaternion
// Program Desc: Dual quaternion skinning functions.
// Program Type: Vertex shader
// Language: GLSL
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void SGX_BlendWeight(in float blendWgt, in mat2x4 dualQuaternion, out mat2x4 vOut)
{
	vOut = blendWgt*dualQuaternion;
}

//-----------------------------------------------------------------------------
void SGX_BlendWeight(in float blendWgt, in mat3x4 scaleShearMatrix, out mat3x4 vOut)
{
	vOut = blendWgt*scaleShearMatrix;
}

//-----------------------------------------------------------------------------
void SGX_AntipodalityAdjustment(in mat2x4 dq0, in mat2x4 dq1,out mat2x4 dq2)
{
	//Accurate antipodality handling. For speed increase, remove the following line, 
	//though, the results will only be valid for rotations less than 180 degrees.
	dq2 = (dot(dq0[0], dq1[0]) < 0.0) ? dq1 * -1.0 : dq1;
}

//-----------------------------------------------------------------------------
void SGX_CalculateBlendPosition(in vec3 position, in mat2x4 blendDQ, out vec4 vOut)
{
	vec3 blendPosition = position + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, position) + blendDQ[0].x*position);
	vec3 trans = 2.0*(blendDQ[0].x*blendDQ[1].yzw - blendDQ[1].x*blendDQ[0].yzw + cross(blendDQ[0].yzw, blendDQ[1].yzw));
	blendPosition += trans;

	vOut = vec4(blendPosition, 1.0);
}

//-----------------------------------------------------------------------------
void SGX_CalculateBlendNormal(in vec3 normal, in mat2x4 blendDQ, out vec3 vOut)
{
	vOut = normal + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, normal) + blendDQ[0].x*normal);
}

//-----------------------------------------------------------------------------
void SGX_NormalizeDualQuaternion(inout mat2x4 dq)
{
	dq /= length(dq[0]);
}

//-----------------------------------------------------------------------------
void SGX_AdjointTransposeMatrix(in mat3x4 M, out mat3 vOut)
{
	mat3x3 atM;
	atM[0][0] = M[2][2] * M[1][1] - M[1][2] * M[2][1];
	atM[0][1] = M[1][2] * M[2][0] - M[1][0] * M[2][2];
	atM[0][2] = M[1][0] * M[2][1] - M[2][0] * M[1][1];

	atM[1][0] = M[0][2] * M[2][1] - M[2][2] * M[0][1];
	atM[1][1] = M[2][2] * M[0][0] - M[0][2] * M[2][0];
	atM[1][2] = M[2][0] * M[0][1] - M[0][0] * M[2][1];

	atM[2][0] = M[1][2] * M[0][1] - M[0][2] * M[1][1];
	atM[2][1] = M[1][0] * M[0][2] - M[1][2] * M[0][0];
	atM[2][2] = M[0][0] * M[1][1] - M[1][0] * M[0][1];

	vOut = atM;
}

//-----------------------------------------------------------------------------
void SGX_BuildDualQuaternionMatrix(in vec4 r1, in vec4 r2, out mat2x4 vOut)
{
	vOut = mat2x4(r1, r2);
}
