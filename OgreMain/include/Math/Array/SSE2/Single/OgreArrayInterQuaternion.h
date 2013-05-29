/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#ifndef __SSE2_ArrayQuaternion_H__
	#error "Don't include this file directly. include Math/Array/OgreArrayQuaternion.h"
#endif

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Intermediate container for Quaternions represented in SoA (structure of arrays) form.
        @remarks
            ArrayInterQuaternion is often not meant to be used directly, but rather to hold intermadiate results.
			It's memory lives on the stack. Because ArrayQuaternion's memory lives on heap, doing something as
			simple as "a + (b + c)" will need to generate a temporary container to hold the result of
			b' = (b + c) -> a + b'
			This quaternion implementation provides that kind of functionality
    */

    class _OgreExport ArrayInterQuaternion
    {
    public:
		ArrayReal				m_chunkBase[4]; //Should live in stack memory

		explicit ArrayInterQuaternion() {}

		ArrayInterQuaternion( const ArrayReal &chunkW, const ArrayReal &chunkX,
								const ArrayReal &chunkY, const ArrayReal &chunkZ )
		{
			m_chunkBase[0] = chunkW;
			m_chunkBase[1] = chunkX;
			m_chunkBase[2] = chunkY;
			m_chunkBase[3] = chunkZ;
		}

		inline explicit ArrayInterQuaternion( const ArrayQuaternion &copy );

		void insertQuaternion( const Quaternion &q, size_t index )
		{
			//Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
			Real *aliasedReal = reinterpret_cast<Real*>( m_chunkBase );
			aliasedReal[ARRAY_PACKED_REALS * 0 + index] = q.w;		//W
			aliasedReal[ARRAY_PACKED_REALS * 1 + index] = q.x;		//X
			aliasedReal[ARRAY_PACKED_REALS * 2 + index] = q.y;		//Y
			aliasedReal[ARRAY_PACKED_REALS * 3 + index] = q.z;		//Z
		}

		void extractQuaternion( Quaternion &out, size_t index ) const
		{
			//Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
			const Real *aliasedReal = reinterpret_cast<const Real*>( m_chunkBase );
			out.w = aliasedReal[ARRAY_PACKED_REALS * 0 + index];		//W
			out.x = aliasedReal[ARRAY_PACKED_REALS * 1 + index];		//X
			out.y = aliasedReal[ARRAY_PACKED_REALS * 2 + index];		//Y
			out.z = aliasedReal[ARRAY_PACKED_REALS * 3 + index];		//Z
		}

		/// @copydoc Quaternion::FromAngleAxis
		inline void FromAngleAxis( const ArrayRadian& rfAngle, const ArrayVector3& rkAxis );
		inline void FromAngleAxis( const ArrayRadian& rfAngle, const ArrayInterVector3& rkAxis );

		/// @copydoc Quaternion::ToAngleAxis
        inline void ToAngleAxis( ArrayRadian &rfAngle, ArrayVector3 &rkAxis ) const;
		inline void ToAngleAxis( ArrayRadian &rfAngle, ArrayInterVector3 &rkAxis ) const;

		inline friend ArrayInterQuaternion operator * ( const ArrayQuaternion &lhs, const ArrayQuaternion &rhs );

		inline friend ArrayInterQuaternion operator * ( const ArrayQuaternion &lhs, const ArrayInterQuaternion &rhs );
		inline friend ArrayInterQuaternion operator * ( const ArrayInterQuaternion &lhs, const ArrayQuaternion &rhs );

		/// @copydoc Quaternion::xAxis
		inline ArrayInterVector3 xAxis( void ) const;
		/// @copydoc Quaternion::yAxis
		inline ArrayInterVector3 yAxis( void ) const;
		/// @copydoc Quaternion::zAxis
		inline ArrayInterVector3 zAxis( void ) const;

		/// @copydoc Quaternion::Dot
        inline ArrayReal Dot( const ArrayQuaternion& rkQ ) const;
		inline ArrayReal Dot( const ArrayInterQuaternion& rkQ ) const;

		/// @copydoc Quaternion::Norm
		inline ArrayReal Norm( void ) const; //Returns the squared length, doesn't modify

		/// Unlike Quaternion::normalise(), this function does not return the length of the vector
		/// because such value was not cached and was never available @see Quaternion::normalise()
		inline void normalise( void );

		inline ArrayInterQuaternion Inverse( void ) const;		// apply to non-zero quaternion
        inline ArrayInterQuaternion UnitInverse( void ) const;	// apply to unit-length quaternion
        inline ArrayInterQuaternion Exp( void ) const;
        inline ArrayInterQuaternion Log( void ) const;

		/// Rotation of a vector by a quaternion
		inline ArrayInterVector3 operator * ( const ArrayVector3 &v ) const;
		inline ArrayInterVector3 operator * ( const ArrayInterVector3 &v ) const;

		/** Conditional move update. @See MathlibSSE2::Cmov4
			Changes each of the four vectors contained in 'this' with
			the replacement provided
			@remarks
				If mask param contains anything other than 0's or 0xffffffff's
				the result is undefined.
				Use this version if you want to decide whether to keep current
				result or overwrite with a replacement (performance optimization).
				i.e. a = Cmov4( a, b )
				If this vector hasn't been assigned yet any value and want to
				decide between two ArrayQuaternions, i.e. a = Cmov4( b, c ) then
				@see Cmov4( const ArrayQuaternion &arg1, const ArrayQuaternion &arg2, ArrayReal mask );
				instead.
			@param
				Vectors to be used as replacement if the mask is zero.
			@param
				mask filled with either 0's or 0xFFFFFFFF
			@return
				this[i] = mask[i] != 0 ? this[i] : replacement[i]
		*/
		inline void Cmov4( ArrayReal mask, const ArrayQuaternion &replacement );
		inline void Cmov4( ArrayReal mask, const ArrayInterQuaternion &replacement );
    };
	/** @} */
	/** @} */
}
