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
// This file is based on material originally from:
// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt


#ifndef __DualQuaternion_H__
#define __DualQuaternion_H__

#include "OgrePrerequisites.h"
#include "OgreMath.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Implementation of a Dual Quaternion, i.e. a rotation around an axis and a translation.
		For more information about Dual Quaternions and the theory behind it, we recommend reading:
    */
    class _OgreExport DualQuaternion
    {
    public:
		/// Default constructor, initializes to identity rotation (aka 0°), and zero translation (0,0,0)
		inline DualQuaternion ()
			: w(1), x(0), y(0), z(0), dw(1), dx(0), dy(0), dz(0)
		{
		}
		
		/// Construct from an explicit list of values
		inline DualQuaternion (
			Real fW,
			Real fX, Real fY, Real fZ, Real fdW, Real fdX, Real fdY, Real fdZ)
			: w(fW), x(fX), y(fY), z(fZ), dw(fdW), dx(fdX), dy(fdY), dz(fdZ)
		{
		}
        
		/// Construct a dual quaternion from a transformation matrix
        inline DualQuaternion(const Matrix4& rot)
        {
            this->FromTransformationMatrix(rot);
        }
		
		/// Construct a dual quaternion from a unit quaternion and a translation vector
		inline DualQuaternion(const Quaternion& q, const Vector3& trans)
		{
			this->FromRotationTranslation(q, trans);
		}
		
		/// Construct a dual quaternion from 8 manual w/x/y/z/dw/dx/dy/dz values
		inline DualQuaternion(Real* valptr)
		{
			memcpy(&w, valptr, sizeof(Real)*8);
		}

		/// Exchange the contents of this dual quaternion with another. 
		inline void swap(DualQuaternion& other)
		{
			std::swap(w, other.w);
			std::swap(x, other.x);
			std::swap(y, other.y);
			std::swap(z, other.z);
			std::swap(dw, other.dw);
			std::swap(dx, other.dx);
			std::swap(dy, other.dy);
			std::swap(dz, other.dz);
		}

		/// Array accessor operator
		inline Real operator [] ( const size_t i ) const
		{
			assert( i < 8 );

			return *(&w+i);
		}

		/// Array accessor operator
		inline Real& operator [] ( const size_t i )
		{
			assert( i < 8 );

			return *(&w+i);
		}

		/// Pointer accessor for direct copying
		inline Real* ptr()
		{
			return &w;
		}

		/// Pointer accessor for direct copying
		inline const Real* ptr() const
		{
			return &w;
		}

		/// Setup the dual quaternion with a certain rotation described by a Quaternion and a translation described by a Vector3
		void FromRotationTranslation (const Quaternion& q, const Vector3& trans);
		void ToRotationTranslation (Quaternion& q, Vector3& translation) const;

		void FromTransformationMatrix (const Matrix4& kTrans);
        void ToTransformationMatrix (Matrix4& kTrans) const;

        inline DualQuaternion& operator= (const DualQuaternion& rkQ)
		{
			w = rkQ.w;
			x = rkQ.x;
			y = rkQ.y;
			z = rkQ.z;
			dw = rkQ.dw;
			dx = rkQ.dx;
			dy = rkQ.dy;
			dz = rkQ.dz;
			return *this;
		}

        inline bool operator== (const DualQuaternion& rhs) const
		{
			return (rhs.x == x) && (rhs.y == y) &&
				(rhs.z == z) && (rhs.w == w) && (rhs.dx == dx) && (rhs.dy == dy) &&
				(rhs.dz == dz) && (rhs.dw == dw);
		}

        inline bool operator!= (const DualQuaternion& rhs) const
		{
			return !operator==(rhs);
		}

		Real w, x, y, z, dw, dx, dy, dz;

		/// Check whether this dual quaternion contains valid values
		inline bool isNaN() const
		{
			return Math::isNaN(x) || Math::isNaN(y) || Math::isNaN(z) || Math::isNaN(w) || Math::isNaN(dx) || Math::isNaN(dy) || Math::isNaN(dz) || Math::isNaN(dw);
		}

        /** Function for writing to a stream. Outputs "DualQuaternion(w, x, y, z, dw, dx, dy, dz)" with w, x, y, z, dw, dx, dy, dz
            being the member values of the dual quaternion.
        */
        inline _OgreExport friend std::ostream& operator <<
            ( std::ostream& o, const DualQuaternion& q )
        {
			o << "DualQuaternion(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ", " << q.dw << ", " << q.dx << ", " << q.dy << ", " << q.dz << ")";
            return o;
        }

    };
	/** @} */
	/** @} */

}

#endif 
