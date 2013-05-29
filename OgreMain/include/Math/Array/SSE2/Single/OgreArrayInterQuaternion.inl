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
	inline ArrayInterQuaternion::ArrayInterQuaternion( const ArrayQuaternion &copy )
	{
		m_chunkBase[0] = copy.m_chunkBase[0];
		m_chunkBase[1] = copy.m_chunkBase[1];
		m_chunkBase[2] = copy.m_chunkBase[2];
		m_chunkBase[3] = copy.m_chunkBase[3];
	}

	//-----------------------------------------------------------------------------------
	// START CODE TO BE COPIED TO OgreArrayInterQuaternion.inl
	//	Copy paste and replace "ArrayInterQuaternion::" for "ArrayInterQuaternion::"
	//-----------------------------------------------------------------------------------
	inline void ArrayInterQuaternion::FromAngleAxis( const ArrayRadian& rfAngle, const ArrayVector3& rkAxis )
	{
		// assert:  axis[] is unit length
        //
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

		ArrayReal fHalfAngle( _mm_mul_ps( rfAngle.valueRadians(), MathlibSSE2::HALF ) );

		ArrayReal fSin;
		MathlibSSE2::SinCos4( fHalfAngle, fSin, m_chunkBase[0] );

		ArrayReal * RESTRICT_ALIAS chunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS rkAxisChunkBase = rkAxis.m_chunkBase;

		chunkBase[1] = _mm_mul_ps( fSin, rkAxisChunkBase[0] ); //x = fSin*rkAxis.x;
		chunkBase[2] = _mm_mul_ps( fSin, rkAxisChunkBase[1] ); //y = fSin*rkAxis.y;
		chunkBase[3] = _mm_mul_ps( fSin, rkAxisChunkBase[2] ); //z = fSin*rkAxis.z;
	}
	inline void ArrayInterQuaternion::FromAngleAxis( const ArrayRadian& rfAngle, const ArrayInterVector3& rkAxis )
	{
		// assert:  axis[] is unit length
        //
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

		ArrayReal fHalfAngle( _mm_mul_ps( rfAngle.valueRadians(), MathlibSSE2::HALF ) );

		ArrayReal fSin;
		MathlibSSE2::SinCos4( fHalfAngle, fSin, m_chunkBase[0] );

		ArrayReal * RESTRICT_ALIAS chunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS rkAxisChunkBase = rkAxis.m_chunkBase;

		chunkBase[1] = _mm_mul_ps( fSin, rkAxisChunkBase[0] ); //x = fSin*rkAxis.x;
		chunkBase[2] = _mm_mul_ps( fSin, rkAxisChunkBase[1] ); //y = fSin*rkAxis.y;
		chunkBase[3] = _mm_mul_ps( fSin, rkAxisChunkBase[2] ); //z = fSin*rkAxis.z;
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterQuaternion::ToAngleAxis( ArrayRadian &rfAngle, ArrayVector3 &rkAxis ) const
	{
		// The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
		ArrayReal sqLength = _mm_add_ps( _mm_add_ps(
								_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ),	//(x * x +
								_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) ),	//y * y) +
								_mm_mul_ps( m_chunkBase[3], m_chunkBase[3] ) );	//z * z )

		ArrayReal mask		= _mm_cmpgt_ps( sqLength, _mm_setzero_ps() ); //mask = sqLength > 0

		//sqLength = sqLength > 0 ? sqLength : 1; so that invSqrt doesn't give NaNs or Infs
		//when 0 (to avoid using CmovRobust just to select the non-nan results)
		sqLength = MathlibSSE2::Cmov4( sqLength, MathlibSSE2::ONE,
										_mm_cmpgt_ps( sqLength, MathlibSSE2::FLOAT_MIN ) );
		ArrayReal fInvLength = MathlibSSE2::InvSqrtNonZero4( sqLength );

		const ArrayReal acosW = MathlibSSE2::ACos4( m_chunkBase[0] );
		rfAngle = MathlibSSE2::Cmov4( //sqLength > 0 ? (2 * ACos(w)) : 0
					_mm_add_ps( acosW, acosW ),
					_mm_setzero_ps(), mask );

		rkAxis.m_chunkBase[0] = MathlibSSE2::Cmov4(	//sqLength > 0 ? (x * fInvLength) : 1
									_mm_mul_ps( m_chunkBase[1], fInvLength ), MathlibSSE2::ONE, mask );
		rkAxis.m_chunkBase[1] = MathlibSSE2::Cmov4(	//sqLength > 0 ? (y * fInvLength) : 0
									_mm_mul_ps( m_chunkBase[2], fInvLength ), _mm_setzero_ps(), mask );
		rkAxis.m_chunkBase[2] = MathlibSSE2::Cmov4(	//sqLength > 0 ? (y * fInvLength) : 0
									_mm_mul_ps( m_chunkBase[3], fInvLength ), _mm_setzero_ps(), mask );
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterQuaternion::ToAngleAxis( ArrayRadian &rfAngle, ArrayInterVector3 &rkAxis ) const
	{
		// The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
		ArrayReal sqLength = _mm_add_ps( _mm_add_ps(
								_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ),	//(x * x +
								_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) ),	//y * y) +
								_mm_mul_ps( m_chunkBase[3], m_chunkBase[3] ) );	//z * z )

		ArrayReal mask = _mm_cmpgt_ps( sqLength, _mm_setzero_ps() ); //mask = sqLength > 0

		//sqLength = sqLength > 0 ? sqLength : 1; so that invSqrt doesn't give NaNs or Infs
		//when 0 (to avoid using CmovRobust just to select the non-nan results)
		sqLength = MathlibSSE2::Cmov4( sqLength, MathlibSSE2::ONE,
										_mm_cmpgt_ps( sqLength, MathlibSSE2::FLOAT_MIN ) );
		ArrayReal fInvLength = MathlibSSE2::InvSqrtNonZero4( sqLength );

		const ArrayReal acosW = MathlibSSE2::ACos4( m_chunkBase[0] );
		rfAngle = MathlibSSE2::Cmov4( //sqLength > 0 ? (2 * ACos(w)) : 0
					_mm_add_ps( acosW, acosW ),
					_mm_setzero_ps(), mask );

		rkAxis.m_chunkBase[0] = MathlibSSE2::Cmov4(	//sqLength > 0 ? (x * fInvLength) : 1
									_mm_mul_ps( m_chunkBase[1], fInvLength ), MathlibSSE2::ONE, mask );
		rkAxis.m_chunkBase[1] = MathlibSSE2::Cmov4(	//sqLength > 0 ? (y * fInvLength) : 0
									_mm_mul_ps( m_chunkBase[2], fInvLength ), _mm_setzero_ps(), mask );
		rkAxis.m_chunkBase[2] = MathlibSSE2::Cmov4(	//sqLength > 0 ? (y * fInvLength) : 0
									_mm_mul_ps( m_chunkBase[3], fInvLength ), _mm_setzero_ps(), mask );
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterQuaternion::xAxis( void ) const
	{
		ArrayReal fTy  = _mm_add_ps( m_chunkBase[2], m_chunkBase[2] );		// 2 * y
		ArrayReal fTz  = _mm_add_ps( m_chunkBase[3], m_chunkBase[3] );		// 2 * z
		ArrayReal fTwy = _mm_mul_ps( fTy, m_chunkBase[0] );					// fTy*w;
		ArrayReal fTwz = _mm_mul_ps( fTz, m_chunkBase[0] );					// fTz*w;
		ArrayReal fTxy = _mm_mul_ps( fTy, m_chunkBase[1] );					// fTy*x;
		ArrayReal fTxz = _mm_mul_ps( fTz, m_chunkBase[1] );					// fTz*x;
		ArrayReal fTyy = _mm_mul_ps( fTy, m_chunkBase[2] );					// fTy*y;
		ArrayReal fTzz = _mm_mul_ps( fTz, m_chunkBase[3] );					// fTz*z;

		return ArrayInterVector3(
				_mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTyy, fTzz ) ),
				_mm_add_ps( fTxy, fTwz ),
				_mm_sub_ps( fTxz, fTwy ) );
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterQuaternion::yAxis( void ) const
	{
		ArrayReal fTx  = _mm_add_ps( m_chunkBase[1], m_chunkBase[1] );		// 2 * x
		ArrayReal fTy  = _mm_add_ps( m_chunkBase[2], m_chunkBase[2] );		// 2 * y
		ArrayReal fTz  = _mm_add_ps( m_chunkBase[3], m_chunkBase[3] );		// 2 * z
		ArrayReal fTwx = _mm_mul_ps( fTx, m_chunkBase[0] );					// fTx*w;
		ArrayReal fTwz = _mm_mul_ps( fTz, m_chunkBase[0] );					// fTz*w;
		ArrayReal fTxx = _mm_mul_ps( fTx, m_chunkBase[1] );					// fTx*x;
		ArrayReal fTxy = _mm_mul_ps( fTy, m_chunkBase[1] );					// fTy*x;
		ArrayReal fTyz = _mm_mul_ps( fTz, m_chunkBase[2] );					// fTz*y;
		ArrayReal fTzz = _mm_mul_ps( fTz, m_chunkBase[3] );					// fTz*z;

		return ArrayInterVector3(
				_mm_sub_ps( fTxy, fTwz ),
				_mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTxx, fTzz ) ),
				_mm_add_ps( fTyz, fTwx ) );
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterQuaternion::zAxis( void ) const
	{
		ArrayReal fTx  = _mm_add_ps( m_chunkBase[1], m_chunkBase[1] );		// 2 * x
		ArrayReal fTy  = _mm_add_ps( m_chunkBase[2], m_chunkBase[2] );		// 2 * y
		ArrayReal fTz  = _mm_add_ps( m_chunkBase[3], m_chunkBase[3] );		// 2 * z
		ArrayReal fTwx = _mm_mul_ps( fTx, m_chunkBase[0] );					// fTx*w;
		ArrayReal fTwy = _mm_mul_ps( fTy, m_chunkBase[0] );					// fTy*w;
		ArrayReal fTxx = _mm_mul_ps( fTx, m_chunkBase[1] );					// fTx*x;
		ArrayReal fTxz = _mm_mul_ps( fTz, m_chunkBase[1] );					// fTz*x;
		ArrayReal fTyy = _mm_mul_ps( fTy, m_chunkBase[2] );					// fTy*y;
		ArrayReal fTyz = _mm_mul_ps( fTz, m_chunkBase[2] );					// fTz*y;

		return ArrayInterVector3(
				_mm_add_ps( fTxz, fTwy ),
				_mm_sub_ps( fTyz, fTwx ),
				_mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTxx, fTyy ) ) );
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayInterQuaternion::Dot( const ArrayQuaternion& rkQ ) const
	{
		return
		_mm_add_ps( _mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], rkQ.m_chunkBase[0] ) ,	//((w * vec.w   +
			_mm_mul_ps( m_chunkBase[1], rkQ.m_chunkBase[1] ) ),	//  x * vec.x ) +
			_mm_mul_ps( m_chunkBase[2], rkQ.m_chunkBase[2] ) ), //  y * vec.y ) +
			_mm_mul_ps( m_chunkBase[3], rkQ.m_chunkBase[3] ) );	//  z * vec.z
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayInterQuaternion::Dot( const ArrayInterQuaternion& rkQ ) const
	{
		return
		_mm_add_ps( _mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], rkQ.m_chunkBase[0] ) ,	//((w * vec.w   +
			_mm_mul_ps( m_chunkBase[1], rkQ.m_chunkBase[1] ) ),	//  x * vec.x ) +
			_mm_mul_ps( m_chunkBase[2], rkQ.m_chunkBase[2] ) ), //  y * vec.y ) +
			_mm_mul_ps( m_chunkBase[3], rkQ.m_chunkBase[3] ) );	//  z * vec.z
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayInterQuaternion::Norm( void ) const
	{
		return
		_mm_add_ps( _mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], m_chunkBase[0] ) ,	//((w * w   +
			_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ) ),	//  x * x ) +
			_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) ), //  y * y ) +
			_mm_mul_ps( m_chunkBase[3], m_chunkBase[3] ) );	//  z * z
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterQuaternion::normalise( void )
	{
		ArrayReal sqLength = _mm_add_ps( _mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], m_chunkBase[0] ) ,	//((w * w   +
			_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ) ),	//  x * x ) +
			_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) ), //  y * y ) +
			_mm_mul_ps( m_chunkBase[3], m_chunkBase[3] ) );	//  z * z

		//Convert sqLength's 0s into 1, so that zero vectors remain as zero
		//Denormals are treated as 0 during the check.
		//Note: We could create a mask now and nuke nans after InvSqrt, however
		//generating the nans could impact performance in some architectures
		sqLength = MathlibSSE2::Cmov4( sqLength, MathlibSSE2::ONE,
										_mm_cmpgt_ps( sqLength, MathlibSSE2::FLOAT_MIN ) );
		ArrayReal invLength = MathlibSSE2::InvSqrtNonZero4( sqLength );
		m_chunkBase[0] = _mm_mul_ps( m_chunkBase[0], invLength ); //w * invLength
		m_chunkBase[1] = _mm_mul_ps( m_chunkBase[1], invLength ); //x * invLength
		m_chunkBase[2] = _mm_mul_ps( m_chunkBase[2], invLength ); //y * invLength
		m_chunkBase[3] = _mm_mul_ps( m_chunkBase[3], invLength ); //z * invLength
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterQuaternion ArrayInterQuaternion::Inverse( void ) const
	{
		ArrayReal fNorm = _mm_add_ps( _mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], m_chunkBase[0] ) ,	//((w * w   +
			_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ) ),	//  x * x ) +
			_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) ), //  y * y ) +
			_mm_mul_ps( m_chunkBase[3], m_chunkBase[3] ) );	//  z * z;

		//Will return a zero Quaternion if original is zero length (Quaternion's behavior)
		fNorm = MathlibSSE2::Cmov4( fNorm, MathlibSSE2::ONE,
									_mm_cmpgt_ps( fNorm, MathlibSSE2::fEpsilon ) );
		ArrayReal invNorm	 = MathlibSSE2::Inv4( fNorm );
		ArrayReal negInvNorm = _mm_mul_ps( invNorm, MathlibSSE2::NEG_ONE );

		return ArrayInterQuaternion(
			_mm_mul_ps( m_chunkBase[0], invNorm ),		//w * invNorm
			_mm_mul_ps( m_chunkBase[1], negInvNorm ),	//x * -invNorm
			_mm_mul_ps( m_chunkBase[2], negInvNorm ),	//y * -invNorm
			_mm_mul_ps( m_chunkBase[3], negInvNorm ) );	//z * -invNorm
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterQuaternion ArrayInterQuaternion::UnitInverse( void ) const
	{
		return ArrayInterQuaternion(
			m_chunkBase[0],											//w
			_mm_mul_ps( m_chunkBase[1], MathlibSSE2::NEG_ONE ),		//-x
			_mm_mul_ps( m_chunkBase[2], MathlibSSE2::NEG_ONE ),		//-y
			_mm_mul_ps( m_chunkBase[3], MathlibSSE2::NEG_ONE ) );	//-z
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterQuaternion ArrayInterQuaternion::Exp( void ) const
	{
		// If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
        // exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
        // use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

		ArrayReal fAngle = _mm_sqrt_ps( _mm_add_ps( _mm_add_ps(						//sqrt(
								_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ),		//(x * x +
								_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) ),		//y * y) +
								_mm_mul_ps( m_chunkBase[3], m_chunkBase[3] ) ) );	//z * z )

		ArrayReal w, fSin;
		MathlibSSE2::SinCos4( fAngle, fSin, w );

		//coeff = Abs(fSin) >= msEpsilon ? (fSin / fAngle) : 1.0f;
		ArrayReal coeff = MathlibSSE2::CmovRobust( _mm_div_ps( fSin, fAngle ), MathlibSSE2::ONE,
								_mm_cmpge_ps( MathlibSSE2::Abs4( fSin ), MathlibSSE2::fEpsilon ) );
		return ArrayInterQuaternion(
			w,											//cos( fAngle )
			_mm_mul_ps( m_chunkBase[1], coeff ),		//x * coeff
			_mm_mul_ps( m_chunkBase[2], coeff ),		//y * coeff
			_mm_mul_ps( m_chunkBase[3], coeff ) );		//z * coeff
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterQuaternion ArrayInterQuaternion::Log( void ) const
	{
		// If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
        // log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
        // sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

		ArrayReal fAngle	= MathlibSSE2::ACos4( m_chunkBase[0] );
		ArrayReal fSin		= MathlibSSE2::Sin4( fAngle );

		//mask = Math::Abs(w) < 1.0 && Math::Abs(fSin) >= msEpsilon
		ArrayReal mask = _mm_and_ps(
							_mm_cmplt_ps( MathlibSSE2::Abs4( m_chunkBase[0] ), MathlibSSE2::ONE ),
							_mm_cmpge_ps( MathlibSSE2::Abs4( fSin ), MathlibSSE2::fEpsilon ) );

		//coeff = mask ? (fAngle / fSin) : 1.0
		//Unlike Exp(), we can use InvNonZero4 (which is faster) instead of div because we know for
		//sure CMov will copy the 1 instead of the NaN when fSin is close to zero, guarantee we might
		//not have in Exp()
		ArrayReal coeff = MathlibSSE2::CmovRobust( _mm_mul_ps( fAngle, MathlibSSE2::InvNonZero4( fSin ) ),
													MathlibSSE2::ONE, mask );

		return ArrayInterQuaternion(
			_mm_setzero_ps(),							//w = 0
			_mm_mul_ps( m_chunkBase[1], coeff ),		//x * coeff
			_mm_mul_ps( m_chunkBase[2], coeff ),		//y * coeff
			_mm_mul_ps( m_chunkBase[3], coeff ) );		//z * coeff
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterQuaternion::operator * ( const ArrayVector3 &v ) const
	{
		// nVidia SDK implementation
		ArrayInterVector3 qVec( m_chunkBase[1], m_chunkBase[2], m_chunkBase[3] );

		ArrayInterVector3 uv	= qVec.crossProduct( v );
		ArrayInterVector3 uuv	= qVec.crossProduct( uv );

		// uv = uv * (2.0f * w)
		ArrayReal w2 = _mm_add_ps( m_chunkBase[0], m_chunkBase[0] );
		uv.m_chunkBase[0] = _mm_mul_ps( uv.m_chunkBase[0], w2 );
		uv.m_chunkBase[1] = _mm_mul_ps( uv.m_chunkBase[1], w2 );
		uv.m_chunkBase[2] = _mm_mul_ps( uv.m_chunkBase[2], w2 );

		// uuv = uuv * 2.0f
		uuv.m_chunkBase[0] = _mm_add_ps( uuv.m_chunkBase[0], uuv.m_chunkBase[0] );
		uuv.m_chunkBase[1] = _mm_add_ps( uuv.m_chunkBase[1], uuv.m_chunkBase[1] );
		uuv.m_chunkBase[2] = _mm_add_ps( uuv.m_chunkBase[2], uuv.m_chunkBase[2] );

		//uv = v + uv + uuv
		uv.m_chunkBase[0] = _mm_add_ps( v.m_chunkBase[0],
								_mm_add_ps( uv.m_chunkBase[0], uuv.m_chunkBase[0] ) );
		uv.m_chunkBase[1] = _mm_add_ps( v.m_chunkBase[1],
								_mm_add_ps( uv.m_chunkBase[1], uuv.m_chunkBase[1] ) );
		uv.m_chunkBase[2] = _mm_add_ps( v.m_chunkBase[2],
								_mm_add_ps( uv.m_chunkBase[2], uuv.m_chunkBase[2] ) );

		return uv;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterQuaternion::operator * ( const ArrayInterVector3 &v ) const
	{
		// nVidia SDK implementation
		ArrayInterVector3 qVec( m_chunkBase[1], m_chunkBase[2], m_chunkBase[3] );

		ArrayInterVector3 uv	= qVec.crossProduct( v );
		ArrayInterVector3 uuv	= qVec.crossProduct( uv );

		// uv = uv * (2.0f * w)
		ArrayReal w2 = _mm_add_ps( m_chunkBase[0], m_chunkBase[0] );
		uv.m_chunkBase[0] = _mm_mul_ps( uv.m_chunkBase[0], w2 );
		uv.m_chunkBase[1] = _mm_mul_ps( uv.m_chunkBase[1], w2 );
		uv.m_chunkBase[2] = _mm_mul_ps( uv.m_chunkBase[2], w2 );

		// uuv = uuv * 2.0f
		uuv.m_chunkBase[0] = _mm_add_ps( uuv.m_chunkBase[0], uuv.m_chunkBase[0] );
		uuv.m_chunkBase[1] = _mm_add_ps( uuv.m_chunkBase[1], uuv.m_chunkBase[1] );
		uuv.m_chunkBase[2] = _mm_add_ps( uuv.m_chunkBase[2], uuv.m_chunkBase[2] );

		//uv = v + uv + uuv
		uv.m_chunkBase[0] = _mm_add_ps( v.m_chunkBase[0],
								_mm_add_ps( uv.m_chunkBase[0], uuv.m_chunkBase[0] ) );
		uv.m_chunkBase[1] = _mm_add_ps( v.m_chunkBase[1],
								_mm_add_ps( uv.m_chunkBase[1], uuv.m_chunkBase[1] ) );
		uv.m_chunkBase[2] = _mm_add_ps( v.m_chunkBase[2],
								_mm_add_ps( uv.m_chunkBase[2], uuv.m_chunkBase[2] ) );

		return uv;
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterQuaternion::Cmov4( ArrayReal mask, const ArrayQuaternion &replacement )
	{
		ArrayReal * RESTRICT_ALIAS aChunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS bChunkBase = replacement.m_chunkBase;
		aChunkBase[0] = MathlibSSE2::Cmov4( aChunkBase[0], replacement.m_chunkBase[0], mask );
		aChunkBase[1] = MathlibSSE2::Cmov4( aChunkBase[1], replacement.m_chunkBase[1], mask );
		aChunkBase[2] = MathlibSSE2::Cmov4( aChunkBase[2], replacement.m_chunkBase[2], mask );
		aChunkBase[3] = MathlibSSE2::Cmov4( aChunkBase[3], replacement.m_chunkBase[3], mask );
	}
	inline void ArrayInterQuaternion::Cmov4( ArrayReal mask, const ArrayInterQuaternion &replacement )
	{
		ArrayReal * RESTRICT_ALIAS aChunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS bChunkBase = replacement.m_chunkBase;
		aChunkBase[0] = MathlibSSE2::Cmov4( aChunkBase[0], bChunkBase[0], mask );
		aChunkBase[1] = MathlibSSE2::Cmov4( aChunkBase[1], bChunkBase[1], mask );
		aChunkBase[2] = MathlibSSE2::Cmov4( aChunkBase[2], bChunkBase[2], mask );
		aChunkBase[3] = MathlibSSE2::Cmov4( aChunkBase[3], bChunkBase[3], mask );
	}

	//-----------------------------------------------------------------------------------
	// END CODE TO BE COPIED TO OgreArrayInterQuaternion.inl
	//-----------------------------------------------------------------------------------
	
}
