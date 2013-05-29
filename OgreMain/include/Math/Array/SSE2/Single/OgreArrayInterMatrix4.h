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
	#error "Don't include this file directly. include Math/Array/OgreArrayMatrix4.h"
#endif

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Intermediate container for 4x4 Matrices represented in SoA (structure of arrays) form.
        @remarks
            ArrayInterMatrix4 is often not meant to be used directly, but rather to hold intermadiate results.
			It's memory lives on the stack. Because ArrayMatrix4's memory lives on heap, doing something as
			simple as "a * (b * c)" will need to generate a temporary container to hold the result of
			b' = (b * c) -> a * b'
			This Matrix4 implementation provides that kind of functionality
    */

    class _OgreExport ArrayInterMatrix4
    {
    public:
		ArrayReal				m_chunkBase[16]; //Should live in stack memory

		//Avoid ArrayInterMatrix4() to be implicitly legal
		ArrayInterMatrix4( int unused )
		{
		}

		inline explicit ArrayInterMatrix4( const ArrayMatrix4 &copy );

		void extractMatrix4( Matrix4 &out, size_t index ) const
		{
			//Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
			const Real *aliasedReal = reinterpret_cast<const Real*>( m_chunkBase );
			for( size_t i=0; i<16; i+=4 )
			{
				out._m[i  ] = aliasedReal[ARRAY_PACKED_REALS * (i  ) + index];
				out._m[i+1] = aliasedReal[ARRAY_PACKED_REALS * (i+1) + index];
				out._m[i+2] = aliasedReal[ARRAY_PACKED_REALS * (i+2) + index];
				out._m[i+3] = aliasedReal[ARRAY_PACKED_REALS * (i+3) + index];
			}
		}

		inline ArrayInterVector3 operator * ( const ArrayVector3 &rhs ) const;
		inline ArrayInterVector3 operator * ( const ArrayInterVector3 &rhs ) const;

		/// Prefer the update version 'a *= b' A LOT over 'a = a * b'
		///	(copying from an ArrayInterMatrix4 is 256 bytes!)
		inline void operator *= ( const ArrayMatrix4 &rhs );
		inline void operator *= ( const ArrayInterMatrix4 &rhs );

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
    };
	/** @} */
	/** @} */
}
