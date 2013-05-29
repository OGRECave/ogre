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

#ifndef __SSE2_ArrayVector3_H__
	#error "Don't include this file directly. include Math/Array/OgreArrayVector3.h"
#endif

namespace Ogre
{
	class ArrayVector3;

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Intermediate container for standard 3-dimensional vector in SoA (structure of arrays) form.
		ArrayInterVector3 is often not meant to be used directly, but rather to hold intermadiate results.
		It's memory lives on the stack. Because ArrayVector3's memory lives on heap, doing something as
		simple as "a + (b + c)" will need to generate a temporary container to hold the result of
		b' = (b + c) -> a + b'
		This vector provides that kind of functionality
    */
	// 
	class _OgreExport ArrayInterVector3
	{
	public:
		ArrayReal				m_chunkBase[3]; //Should live in stack memory

		explicit ArrayInterVector3() {}

		ArrayInterVector3( ArrayReal chunkX, ArrayReal chunkY, ArrayReal chunkZ )
		{
			m_chunkBase[0] = chunkX;
			m_chunkBase[1] = chunkY;
			m_chunkBase[2] = chunkZ;
		}

		inline explicit ArrayInterVector3( const ArrayVector3 &copy );

		void setAll( const Vector3 &v )
		{
			m_chunkBase[0] = _mm_set_ps1( v.x );	//X = v.XXXX;
			m_chunkBase[1] = _mm_set_ps1( v.y );	//Y = v.YYYY;
			m_chunkBase[2] = _mm_set_ps1( v.z );	//Z = v.ZZZZ;
		}

		void insertVector3( const Vector3 &v, size_t index )
		{
			//Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
			Real *aliasedReal = reinterpret_cast<Real*>( m_chunkBase );
			aliasedReal[ARRAY_PACKED_REALS * 0 + index] = v.x;		//X
			aliasedReal[ARRAY_PACKED_REALS * 1 + index] = v.y;		//Y
			aliasedReal[ARRAY_PACKED_REALS * 2 + index] = v.z;		//Z
		}

		void extractVector3( Vector3 &out, size_t index ) const
		{
			//Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
			const Real *aliasedReal = reinterpret_cast<const Real*>( m_chunkBase );
			out.x = aliasedReal[ARRAY_PACKED_REALS * 0 + index];		//X
			out.y = aliasedReal[ARRAY_PACKED_REALS * 1 + index];		//Y
			out.z = aliasedReal[ARRAY_PACKED_REALS * 2 + index];		//Z
		}

		inline const ArrayInterVector3& operator + () const;
		inline ArrayInterVector3 operator - () const;

        inline void operator += ( const ArrayVector3 &a );
		inline void operator += ( const ArrayInterVector3 &a );
		inline void operator += ( const Real fScalar );
		inline void operator += ( const ArrayReal fScalar );

		inline void operator -= ( const ArrayVector3 &a );
		inline void operator -= ( const ArrayInterVector3 &a );
		inline void operator -= ( const Real fScalar );
		inline void operator -= ( const ArrayReal fScalar );

		inline void operator *= ( const ArrayVector3 &a );
		inline void operator *= ( const ArrayInterVector3 &a );
		inline void operator *= ( const Real fScalar );
		inline void operator *= ( const ArrayReal fScalar );

		inline void operator /= ( const ArrayVector3 &a );
		inline void operator /= ( const ArrayInterVector3 &a );
		inline void operator /= ( const Real fScalar );
		inline void operator /= ( const ArrayReal fScalar );

		/// @copydoc Vector3::length()
		inline ArrayReal length() const;

		/// @copydoc Vector3::squaredLength()
		inline ArrayReal squaredLength() const;

		/// @copydoc Vector3::distance()
		inline ArrayReal distance( const ArrayVector3& rhs ) const;
		inline ArrayReal distance( const ArrayInterVector3& rhs ) const;

		/// @copydoc Vector3::squaredDistance()
		inline ArrayReal squaredDistance( const ArrayVector3& rhs ) const;
		inline ArrayReal squaredDistance( const ArrayInterVector3& rhs ) const;

		/// @copydoc Vector3::dotProduct()
		inline ArrayReal dotProduct( const ArrayVector3& vec ) const;
		inline ArrayReal dotProduct( const ArrayInterVector3& vec ) const;

		/// @copydoc Vector3::absDotProduct()
		inline ArrayReal absDotProduct( const ArrayVector3& vec ) const;
		inline ArrayReal absDotProduct( const ArrayInterVector3& vec ) const;

		/// Unlike Vector3::normalise(), this function does not return the length of the vector
		/// because such value was not cached and was never available @see Vector3::normalise()
		inline void normalise( void );

		/// @copydoc Vector3::crossProduct()
		inline ArrayInterVector3 crossProduct( const ArrayVector3& rkVector ) const;
		inline ArrayInterVector3 crossProduct( const ArrayInterVector3& rkVector ) const;

		/// @copydoc Vector3::midPoint()
		inline ArrayInterVector3 midPoint( const ArrayVector3& vec ) const;
		inline ArrayInterVector3 midPoint( const ArrayInterVector3& vec ) const;

		/// @copydoc Vector3::makeFloor()
		inline void makeFloor( const ArrayVector3& cmp );
		inline void makeFloor( const ArrayInterVector3& cmp );

		/// @copydoc Vector3::makeCeil()
		inline void makeCeil( const ArrayVector3& cmp );
		inline void makeCeil( const ArrayInterVector3& cmp );

		/// @copydoc Vector3::perpendicular()
		inline ArrayInterVector3 perpendicular( void ) const;

		/// @copydoc Vector3::normalisedCopy()
		inline ArrayInterVector3 normalisedCopy( void ) const;

		/// @copydoc Vector3::reflect()
        inline ArrayInterVector3 reflect( const ArrayVector3& normal ) const;
		inline ArrayInterVector3 reflect( const ArrayInterVector3& normal ) const;

		/// @see Vector3::isNaN()
		///	@return
		///		Return value differs from Vector3's counterpart. We return an int
		///		bits 0-4 are set for each NaN of each vector inside.
		///		if the int is non-zero, there is a NaN.
		inline int isNaN( void ) const;

		/// @copydoc Vector3::primaryAxis()
		inline ArrayInterVector3 primaryAxis( void ) const;

		/** Conditional move update. @See MathlibSSE2::Cmov4
			Changes each of the four vectors contained in 'this' with
			the replacement provided
			@remarks
				If mask param contains anything other than 0's or 0xffffffff's
				the result is undefined.
				Use this version if you want to decide whether to keep current
				result or overwrite with a replacement (performance optimization).
				If this vector hasn't been assigned yet any value and want to
				decide between two ArrayVector3s,
				@see Cmov4( const ArrayVector3 &arg1, const ArrayVector3 &arg2, ArrayReal mask );
				instead.
			@param
				replacement vectors
			@param
				mask filled with either 0's or 0xFFFFFFFF
			@return
				this[i] = mask[i] != 0 ? this[i] : replacement[i]
		*/
		inline void Cmov4( ArrayReal mask, const ArrayVector3 &replacement );
		inline void Cmov4( ArrayReal mask, const ArrayInterVector3 &replacement );
	};
	/** @} */
	/** @} */
}
