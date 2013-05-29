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
#ifndef __SSE2_ArrayMatrix4_H__
#define __SSE2_ArrayMatrix4_H__

#ifndef __ArrayMatrix4_H__
	#error "Don't include this file directly. include Math/Array/OgreArrayMatrix4.h"
#endif

#include "OgreMatrix4.h"

#include "Math/Array/OgreMathlib.h"
#include "Math/Array/OgreArrayVector3.h"
#include "Math/Array/OgreArrayQuaternion.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Cache-friendly container of 4x4 matrices represented as a SoA array.
        @remarks
            ArrayMatrix4 is a SIMD & cache-friendly version of Matrix4.
			An operation on an ArrayMatrix4 is done on 4 vectors at a
			time (the actual amount is defined by ARRAY_PACKED_REALS)
			Assuming ARRAY_PACKED_REALS == 4, the memory layout will
			be as following:
             m_chunkBase	m_chunkBase + 3
			 a00b00c00d00	 a01b01c01d01
			Extracting one Matrix4 needs 256 bytes, which needs 4 line
			fetches for common cache lines of 64 bytes.
			Make sure extractions are made sequentially to avoid cache
			trashing and excessive bandwidth consumption, and prefer
			working on @See ArrayVector3 & @See ArrayQuaternion instead
			Architectures where the cache line == 32 bytes may want to
			set ARRAY_PACKED_REALS = 2 depending on their needs
    */

    class _OgreExport ArrayMatrix4
    {
    public:
		ArrayReal		m_chunkBase[16];

		ArrayMatrix4() {}
		ArrayMatrix4( const ArrayMatrix4 &copy )
		{
			//Using a loop minimizes instruction count (better i-cache)
			//Doing 4 at a time per iteration maximizes instruction pairing
			//Unrolling the whole loop is i-cache unfriendly and
			//becomes unmaintainable (16 lines!?)
			for( size_t i=0; i<16; i+=4 )
			{
				m_chunkBase[i  ] = copy.m_chunkBase[i  ];
				m_chunkBase[i+1] = copy.m_chunkBase[i+1];
				m_chunkBase[i+2] = copy.m_chunkBase[i+2];
				m_chunkBase[i+3] = copy.m_chunkBase[i+3];
			}
		}

		void getAsMatrix4( Matrix4 &out, size_t index ) const
		{
			//Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
			const Real * RESTRICT_ALIAS aliasedReal = reinterpret_cast<const Real*>( m_chunkBase );
			Real * RESTRICT_ALIAS matrix = reinterpret_cast<Real*>( out._m );
			for( size_t i=0; i<16; i+=4 )
			{
				matrix[i  ] = aliasedReal[ARRAY_PACKED_REALS * (i  ) + index];
				matrix[i+1] = aliasedReal[ARRAY_PACKED_REALS * (i+1) + index];
				matrix[i+2] = aliasedReal[ARRAY_PACKED_REALS * (i+2) + index];
				matrix[i+3] = aliasedReal[ARRAY_PACKED_REALS * (i+3) + index];
			}
		}

		/// STRONGLY Prefer using @see getAsMatrix4() because this function may have more
		/// overhead (the other one is faster)
		Matrix4 getAsMatrix4( size_t index ) const
		{
			Matrix4 retVal;
			getAsMatrix4( retVal, index );

			return retVal;
		}

		void setFromMatrix4( const Matrix4 &m, size_t index )
		{
			Real * RESTRICT_ALIAS aliasedReal = reinterpret_cast<Real*>( m_chunkBase );
			const Real * RESTRICT_ALIAS matrix = reinterpret_cast<const Real*>( m._m );
			for( size_t i=0; i<16; i+=4 )
			{
				aliasedReal[ARRAY_PACKED_REALS * (i  ) + index] = matrix[i  ];
				aliasedReal[ARRAY_PACKED_REALS * (i+1) + index] = matrix[i+1];
				aliasedReal[ARRAY_PACKED_REALS * (i+2) + index] = matrix[i+2];
				aliasedReal[ARRAY_PACKED_REALS * (i+3) + index] = matrix[i+3];
			}
		}

		/** Assigns the value of the other matrix. Does not reference the
			ptr address, but rather perform a memory copy
            @param
                rkmatrix The other matrix
        */
        inline ArrayMatrix4& operator = ( const ArrayMatrix4& rkMatrix )
        {
			for( size_t i=0; i<16; i+=4 )
			{
				m_chunkBase[i  ] = rkMatrix.m_chunkBase[i  ];
				m_chunkBase[i+1] = rkMatrix.m_chunkBase[i+1];
				m_chunkBase[i+2] = rkMatrix.m_chunkBase[i+2];
				m_chunkBase[i+3] = rkMatrix.m_chunkBase[i+3];
			}
            return *this;
        }

		// Concatenation
		inline friend ArrayMatrix4 operator * ( const ArrayMatrix4 &lhs, const ArrayMatrix4 &rhs );

		inline ArrayVector3 operator * ( const ArrayVector3 &rhs ) const;

		/// Prefer the update version 'a *= b' A LOT over 'a = a * b'
		///	(copying from an ArrayMatrix4 is 256 bytes!)
		inline void operator *= ( const ArrayMatrix4 &rhs );

		/**	Converts the given quaternion to a 3x3 matrix representation and fill our values
			@remarks
				Similar to @see Quaternion::ToRotationMatrix, this function will take the input
				quaternion and overwrite the first 3x3 subset of this matrix. The 4th row &
				columns are left untouched.
				This function is defined in ArrayMatrix4 to avoid including this header into
				ArrayQuaternion. The idea is that ArrayMatrix4 requires ArrayQuaternion, and
				ArrayQuaternion requires ArrayVector3. Simple dependency order
			@param
				The quaternion to convert from.
		*/
		inline void fromQuaternion( const ArrayQuaternion &q );

		/// @copydoc Matrix4::makeTransform()
		inline void makeTransform( const ArrayVector3 &position, const ArrayVector3 &scale,
									const ArrayQuaternion &orientation );

		/// @copydoc Matrix4::isAffine()
		inline bool isAffine() const;
    };
	/** @} */
	/** @} */

}

#include "OgreArrayMatrix4.inl"

#endif
