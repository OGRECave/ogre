/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreStableHeaders.h"

#include "OgreDualQuaternion.h"

#include "OgreMath.h"
#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreVector3.h"

namespace Ogre {

	//Based on dqconv.c from http://isg.cs.tcd.ie/projects/DualQuaternions/
	//-----------------------------------------------------------------------
	void DualQuaternion::fromRotationTranslation (const Quaternion& q, const Vector3& trans)
	{
		// non-dual part (just copy the quaternion):
		w = q.w;
		x = q.x;
		y = q.y;
		z = q.z;
		
		// dual part:
		Real half = 0.5;
		dw = -half *  (trans.x * x + trans.y * y + trans.z * z ); 
		dx =  half *  (trans.x * w + trans.y * z - trans.z * y ); 
		dy =  half * (-trans.x * z + trans.y * w + trans.z * x ); 
		dz =  half *  (trans.x * y - trans.y * x + trans.z * w ); 
	}

	//Based on dqconv.c from http://isg.cs.tcd.ie/projects/DualQuaternions/
	//-----------------------------------------------------------------------
	void DualQuaternion::toRotationTranslation (Quaternion& q, Vector3& translation) const
	{
		// regular quaternion (just copy the non-dual part):
		q.w = w;
		q.x = x;
		q.y = y;
		q.z = z;

		// translation vector:
		Real doub = 2.0;
		translation.x = doub * (-dw*x + dx*w - dy*z + dz*y);
		translation.y = doub * (-dw*y + dx*z + dy*w - dz*x);
		translation.z = doub * (-dw*z - dx*y + dy*x + dz*w);
	}

	//-----------------------------------------------------------------------
	void DualQuaternion::fromTransformationMatrix (const Matrix4& kTrans)
	{
		Vector3 pos;
		Vector3 scale;
		Quaternion rot;

		kTrans.decomposition(pos, scale, rot);
		fromRotationTranslation(rot, pos);
	}

	//-----------------------------------------------------------------------
	void DualQuaternion::toTransformationMatrix (Matrix4& kTrans) const
	{
		Vector3 pos;
		Quaternion rot;
		toRotationTranslation(rot, pos);

		Vector3 scale = Vector3::UNIT_SCALE;
		kTrans.makeTransform(pos, scale, rot);
	}
}