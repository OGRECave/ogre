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

namespace Ogre
{
	inline ArrayInterMatrix4::ArrayInterMatrix4( const ArrayMatrix4 &copy )
	{
		for( size_t i=0; i<16; i+=4 )
		{
			m_chunkBase[i  ] = copy.m_chunkBase[i  ];
			m_chunkBase[i+1] = copy.m_chunkBase[i+1];
			m_chunkBase[i+2] = copy.m_chunkBase[i+2];
			m_chunkBase[i+3] = copy.m_chunkBase[i+3];
		}
	}

	//-----------------------------------------------------------------------------------
	// START CODE TO BE COPIED TO OgreArrayInterMatrix4.inl
	//	Copy paste and replace "ArrayInterMatrix4::" for "ArrayInterMatrix4::"
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterMatrix4::operator * ( const ArrayVector3 &rhs ) const
	{
		ArrayReal invW = _mm_add_ps( _mm_add_ps(
								_mm_mul_ps( m_chunkBase[12], rhs.m_chunkBase[0] ),
								_mm_mul_ps( m_chunkBase[13], rhs.m_chunkBase[1] ) ),
							_mm_add_ps(
								_mm_mul_ps( m_chunkBase[14], rhs.m_chunkBase[2] ),
								m_chunkBase[15] ) );
		invW = MathlibSSE2::Inv4( invW );

		return ArrayInterVector3(
			//X
			_mm_mul_ps(
			_mm_add_ps( _mm_add_ps(
				_mm_mul_ps( m_chunkBase[0], rhs.m_chunkBase[0] ),	//( m00 * v.x   +
				_mm_mul_ps( m_chunkBase[1], rhs.m_chunkBase[1] ) ),	//  m01 * v.y ) +
			_mm_add_ps(
				_mm_mul_ps( m_chunkBase[2], rhs.m_chunkBase[2] ),	//( m02 * v.z   +
				m_chunkBase[3] ) ) , invW ),						//  m03 ) * fInvW
			//Y
			_mm_mul_ps(
			_mm_add_ps( _mm_add_ps(
				_mm_mul_ps( m_chunkBase[4], rhs.m_chunkBase[0] ),	//( m10 * v.x   +
				_mm_mul_ps( m_chunkBase[5], rhs.m_chunkBase[1] ) ),	//  m11 * v.y ) +
			_mm_add_ps(
				_mm_mul_ps( m_chunkBase[6], rhs.m_chunkBase[2] ),	//( m12 * v.z   +
				m_chunkBase[7] ) ), invW ),							//  m13 ) * fInvW
			//Z
			_mm_mul_ps(
			_mm_add_ps( _mm_add_ps(
				_mm_mul_ps( m_chunkBase[8], rhs.m_chunkBase[0] ),	//( m20 * v.x   +
				_mm_mul_ps( m_chunkBase[9], rhs.m_chunkBase[1] ) ),	//  m21 * v.y ) +
			_mm_add_ps(
				_mm_mul_ps( m_chunkBase[10], rhs.m_chunkBase[2] ),	//( m22 * v.z   +
				m_chunkBase[11] ) ), invW ) );						//  m23 ) * fInvW
	}
	inline ArrayInterVector3 ArrayInterMatrix4::operator * ( const ArrayInterVector3 &rhs ) const
	{
		ArrayReal invW = _mm_add_ps( _mm_add_ps(
								_mm_mul_ps( m_chunkBase[12], rhs.m_chunkBase[0] ),
								_mm_mul_ps( m_chunkBase[13], rhs.m_chunkBase[1] ) ),
							_mm_add_ps(
								_mm_mul_ps( m_chunkBase[14], rhs.m_chunkBase[2] ),
								m_chunkBase[15] ) );
		invW = MathlibSSE2::Inv4( invW );

		return ArrayInterVector3(
			//X
			_mm_mul_ps(
			_mm_add_ps( _mm_add_ps(
				_mm_mul_ps( m_chunkBase[0], rhs.m_chunkBase[0] ),	//( m00 * v.x   +
				_mm_mul_ps( m_chunkBase[1], rhs.m_chunkBase[1] ) ),	//  m01 * v.y ) +
			_mm_add_ps(
				_mm_mul_ps( m_chunkBase[2], rhs.m_chunkBase[2] ),	//( m02 * v.z   +
				m_chunkBase[3] ) ) , invW ),						//  m03 ) * fInvW
			//Y
			_mm_mul_ps(
			_mm_add_ps( _mm_add_ps(
				_mm_mul_ps( m_chunkBase[4], rhs.m_chunkBase[0] ),	//( m10 * v.x   +
				_mm_mul_ps( m_chunkBase[5], rhs.m_chunkBase[1] ) ),	//  m11 * v.y ) +
			_mm_add_ps(
				_mm_mul_ps( m_chunkBase[6], rhs.m_chunkBase[2] ),	//( m12 * v.z   +
				m_chunkBase[7] ) ), invW ),							//  m13 ) * fInvW
			//Z
			_mm_mul_ps(
			_mm_add_ps( _mm_add_ps(
				_mm_mul_ps( m_chunkBase[8], rhs.m_chunkBase[0] ),	//( m20 * v.x   +
				_mm_mul_ps( m_chunkBase[9], rhs.m_chunkBase[1] ) ),	//  m21 * v.y ) +
			_mm_add_ps(
				_mm_mul_ps( m_chunkBase[10], rhs.m_chunkBase[2] ),	//( m22 * v.z   +
				m_chunkBase[11] ) ), invW ) );						//  m23 ) * fInvW
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterMatrix4::operator *= ( const ArrayMatrix4 &rhs )
	{
		concatArrayMat4( m_chunkBase, rhs.m_chunkBase );
	}
	inline void ArrayInterMatrix4::operator *= ( const ArrayInterMatrix4 &rhs )
	{
		concatArrayMat4( m_chunkBase, rhs.m_chunkBase );
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterMatrix4::fromQuaternion( const ArrayQuaternion &q )
	{
		ArrayReal * RESTRICT_ALIAS chunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS qChunkBase = q.m_chunkBase;
		ArrayReal fTx  = _mm_add_ps( qChunkBase[1], qChunkBase[1] );		// 2 * x
		ArrayReal fTy  = _mm_add_ps( qChunkBase[2], qChunkBase[2] );		// 2 * y
		ArrayReal fTz  = _mm_add_ps( qChunkBase[3], qChunkBase[3] );		// 2 * z
        ArrayReal fTwx = _mm_mul_ps( fTx, qChunkBase[0] );					// fTx*w;
        ArrayReal fTwy = _mm_mul_ps( fTy, qChunkBase[0] );					// fTy*w;
        ArrayReal fTwz = _mm_mul_ps( fTz, qChunkBase[0] );					// fTz*w;
        ArrayReal fTxx = _mm_mul_ps( fTx, qChunkBase[1] );					// fTx*x;
        ArrayReal fTxy = _mm_mul_ps( fTy, qChunkBase[1] );					// fTy*x;
        ArrayReal fTxz = _mm_mul_ps( fTz, qChunkBase[1] );					// fTz*x;
        ArrayReal fTyy = _mm_mul_ps( fTy, qChunkBase[2] );					// fTy*y;
        ArrayReal fTyz = _mm_mul_ps( fTz, qChunkBase[2] );					// fTz*y;
        ArrayReal fTzz = _mm_mul_ps( fTz, qChunkBase[3] );					// fTz*z;

        chunkBase[0] = _mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTyy, fTzz ) );
        chunkBase[1] = _mm_sub_ps( fTxy, fTwz );
        chunkBase[2] = _mm_add_ps( fTxz, fTwy );
        chunkBase[4] = _mm_add_ps( fTxy, fTwz );
        chunkBase[5] = _mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTxx, fTzz ) );
        chunkBase[6] = _mm_sub_ps( fTyz, fTwx );
        chunkBase[8] = _mm_sub_ps( fTxz, fTwy );
        chunkBase[9] = _mm_add_ps( fTyz, fTwx );
        chunkBase[10]= _mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTxx, fTyy ) );
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterMatrix4::makeTransform( const ArrayVector3 &position, const ArrayVector3 &scale,
											 const ArrayQuaternion &orientation )
	{
		ArrayReal * RESTRICT_ALIAS chunkBase			= m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS posChunkBase	= position.m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS scaleChunkBase	= scale.m_chunkBase;
		this->fromQuaternion( orientation );
		chunkBase[0] = _mm_mul_ps( chunkBase[0], scaleChunkBase[0] );	//m00 * scale.x
		chunkBase[1] = _mm_mul_ps( chunkBase[1], scaleChunkBase[1] );	//m01 * scale.y
		chunkBase[2] = _mm_mul_ps( chunkBase[2], scaleChunkBase[2] );	//m02 * scale.z
		chunkBase[3] =  posChunkBase[0];								//m03 * pos.x

		chunkBase[4] = _mm_mul_ps( chunkBase[4], scaleChunkBase[0] );	//m10 * scale.x
		chunkBase[5] = _mm_mul_ps( chunkBase[5], scaleChunkBase[1] );	//m11 * scale.y
		chunkBase[6] = _mm_mul_ps( chunkBase[6], scaleChunkBase[2] );	//m12 * scale.z
		chunkBase[7] =  posChunkBase[1];								//m13 * pos.y

		chunkBase[8] = _mm_mul_ps( chunkBase[8], scaleChunkBase[0] );	//m20 * scale.x
		chunkBase[9] = _mm_mul_ps( chunkBase[9], scaleChunkBase[1] );	//m21 * scale.y
		chunkBase[10]= _mm_mul_ps( chunkBase[10],scaleChunkBase[2] );	//m22 * scale.z
		chunkBase[11]=  posChunkBase[2];								//m23 * pos.z

		// No projection term
		chunkBase[12] = m_chunkBase[13] = m_chunkBase[14] = _mm_setzero_ps();
		chunkBase[15] = MathlibSSE2::ONE;
	}
	//-----------------------------------------------------------------------------------
	// END CODE TO BE COPIED TO OgreArrayInterMatrix4.inl
	//-----------------------------------------------------------------------------------
	
}
