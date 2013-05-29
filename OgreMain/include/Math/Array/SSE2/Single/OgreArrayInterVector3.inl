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
	inline ArrayInterVector3::ArrayInterVector3( const ArrayVector3 &copy )
	{
		m_chunkBase[0] = copy.m_chunkBase[0];
		m_chunkBase[1] = copy.m_chunkBase[1];
		m_chunkBase[2] = copy.m_chunkBase[2];
	}

	// Update operations
#define DEFINE_UPDATE_OPERATION( leftClass, op, op_func )\
	inline void ArrayInterVector3::operator op ( const leftClass &a )\
	{\
		ArrayReal * RESTRICT_ALIAS chunkBase = m_chunkBase;\
		const ArrayReal * RESTRICT_ALIAS aChunkBase = a.m_chunkBase;\
		chunkBase[0] = op_func( chunkBase[0], aChunkBase[0] );\
		chunkBase[1] = op_func( chunkBase[1], aChunkBase[1] );\
		chunkBase[2] = op_func( chunkBase[2], aChunkBase[2] );\
	}
#define DEFINE_UPDATE_R_SCALAR_OPERATION( rightType, op, op_func )\
	inline void ArrayInterVector3::operator op ( const rightType fScalar )\
	{\
		ArrayReal a = _mm_set1_ps( fScalar );\
		m_chunkBase[0] = op_func( m_chunkBase[0], a );\
		m_chunkBase[1] = op_func( m_chunkBase[1], a );\
		m_chunkBase[2] = op_func( m_chunkBase[2], a );\
	}
#define DEFINE_UPDATE_R_OPERATION( rightType, op, op_func )\
	inline void ArrayInterVector3::operator op ( const rightType a )\
	{\
		m_chunkBase[0] = op_func( m_chunkBase[0], a );\
		m_chunkBase[1] = op_func( m_chunkBase[1], a );\
		m_chunkBase[2] = op_func( m_chunkBase[2], a );\
	}
#define DEFINE_UPDATE_DIVISION( leftClass, op, op_func )\
	inline void ArrayInterVector3::operator op ( const leftClass &a )\
	{\
		ArrayReal * RESTRICT_ALIAS chunkBase = m_chunkBase;\
		const ArrayReal * RESTRICT_ALIAS aChunkBase = a.m_chunkBase;\
		chunkBase[0] = op_func( chunkBase[0], aChunkBase[0] );\
		chunkBase[1] = op_func( chunkBase[1], aChunkBase[1] );\
		chunkBase[2] = op_func( chunkBase[2], aChunkBase[2] );\
	}
#define DEFINE_UPDATE_R_SCALAR_DIVISION( rightType, op, op_func )\
	inline void ArrayInterVector3::operator op ( const rightType fScalar )\
	{\
		assert( fScalar != 0.0 );\
		Real fInv = 1.0f / fScalar;\
		ArrayReal a = _mm_set1_ps( fInv );\
		m_chunkBase[0] = op_func( m_chunkBase[0], a );\
		m_chunkBase[1] = op_func( m_chunkBase[1], a );\
		m_chunkBase[2] = op_func( m_chunkBase[2], a );\
	}
#define DEFINE_UPDATE_R_DIVISION( rightType, op, op_func )\
	inline void ArrayInterVector3::operator op ( const rightType _a )\
	{\
		ASSERT_DIV_BY_ZERO( _a );\
		ArrayReal a = MathlibSSE2::Inv4( _a );\
		m_chunkBase[0] = op_func( m_chunkBase[0], a );\
		m_chunkBase[1] = op_func( m_chunkBase[1], a );\
		m_chunkBase[2] = op_func( m_chunkBase[2], a );\
	}

	inline const ArrayInterVector3& ArrayInterVector3::operator + () const
	{
		return *this;
	};
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterVector3::operator - () const
	{
		return ArrayInterVector3(
			_mm_xor_ps( m_chunkBase[0], MathlibSSE2::SIGN_MASK ),	//-x
			_mm_xor_ps( m_chunkBase[1], MathlibSSE2::SIGN_MASK ),	//-y
			_mm_xor_ps( m_chunkBase[2], MathlibSSE2::SIGN_MASK ) );	//-z
	}
	//-----------------------------------------------------------------------------------

	// Update operations
	// +=
	DEFINE_UPDATE_OPERATION(			ArrayVector3,		+=, _mm_add_ps );
	DEFINE_UPDATE_OPERATION(			ArrayInterVector3,	+=, _mm_add_ps );
	DEFINE_UPDATE_R_SCALAR_OPERATION(	Real,				+=, _mm_add_ps );
	DEFINE_UPDATE_R_OPERATION(			ArrayReal,			+=, _mm_add_ps );

	// -=
	DEFINE_UPDATE_OPERATION(			ArrayVector3,		-=, _mm_sub_ps );
	DEFINE_UPDATE_OPERATION(			ArrayInterVector3,	-=, _mm_sub_ps );
	DEFINE_UPDATE_R_SCALAR_OPERATION(	Real,				-=, _mm_sub_ps );
	DEFINE_UPDATE_R_OPERATION(			ArrayReal,			-=, _mm_sub_ps );

	// *=
	DEFINE_UPDATE_OPERATION(			ArrayVector3,		*=, _mm_mul_ps );
	DEFINE_UPDATE_OPERATION(			ArrayInterVector3,	*=, _mm_mul_ps );
	DEFINE_UPDATE_R_SCALAR_OPERATION(	Real,				*=, _mm_mul_ps );
	DEFINE_UPDATE_R_OPERATION(			ArrayReal,			*=, _mm_mul_ps );

	// /=
	DEFINE_UPDATE_DIVISION(				ArrayVector3,		/=, _mm_div_ps );
	DEFINE_UPDATE_DIVISION(				ArrayInterVector3,	/=, _mm_div_ps );
	DEFINE_UPDATE_R_SCALAR_DIVISION(	Real,				/=, _mm_mul_ps );
	DEFINE_UPDATE_R_DIVISION(			ArrayReal,			/=, _mm_mul_ps );

	//Functions
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayInterVector3::length() const
    {
		return
		_mm_sqrt_ps( _mm_add_ps( _mm_add_ps(					//sqrt(
				_mm_mul_ps( m_chunkBase[0], m_chunkBase[0] ),	//(x * x +
				_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ) ),	//y * y) +
			_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) ) );	//z * z )
    }
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayInterVector3::squaredLength() const
    {
        return
		_mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], m_chunkBase[0] ),	//(x * x +
			_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ) ),	//y * y) +
		_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) );		//z * z )
    }
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayInterVector3::distance( const ArrayVector3& rhs ) const
    {
        return (*this - rhs).length();
    }
	inline ArrayReal ArrayInterVector3::distance( const ArrayInterVector3& rhs ) const
    {
        return (*this - rhs).length();
    }
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayInterVector3::squaredDistance( const ArrayVector3& rhs ) const
    {
        return (*this - rhs).squaredLength();
    }
	inline ArrayReal ArrayInterVector3::squaredDistance( const ArrayInterVector3& rhs ) const
    {
        return (*this - rhs).squaredLength();
    }
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayInterVector3::dotProduct( const ArrayVector3& vec ) const
	{
		return
		_mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], vec.m_chunkBase[0] ) ,	//( x * vec.x   +
			_mm_mul_ps( m_chunkBase[1], vec.m_chunkBase[1] ) ),	//  y * vec.y ) +
			_mm_mul_ps( m_chunkBase[2], vec.m_chunkBase[2] ) );	//  z * vec.z
	}
	inline ArrayReal ArrayInterVector3::dotProduct( const ArrayInterVector3& vec ) const
	{
		return
		_mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], vec.m_chunkBase[0] ) ,	//( x * vec.x   +
			_mm_mul_ps( m_chunkBase[1], vec.m_chunkBase[1] ) ),	//  y * vec.y ) +
			_mm_mul_ps( m_chunkBase[2], vec.m_chunkBase[2] ) );	//  z * vec.z
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayInterVector3::absDotProduct( const ArrayVector3& vec ) const
	{
		return
		_mm_add_ps( _mm_add_ps(
			MathlibSSE2::Abs4( _mm_mul_ps( m_chunkBase[0], vec.m_chunkBase[0] ) ),	//( abs( x * vec.x )   +
			MathlibSSE2::Abs4( _mm_mul_ps( m_chunkBase[1], vec.m_chunkBase[1] ) ) ),//  abs( y * vec.y ) ) +
			MathlibSSE2::Abs4( _mm_mul_ps( m_chunkBase[2], vec.m_chunkBase[2] ) ) );//  abs( z * vec.z )
	}
	inline ArrayReal ArrayInterVector3::absDotProduct( const ArrayInterVector3& vec ) const
	{
		return
		_mm_add_ps( _mm_add_ps(
			MathlibSSE2::Abs4( _mm_mul_ps( m_chunkBase[0], vec.m_chunkBase[0] ) ),	//( abs( x * vec.x )   +
			MathlibSSE2::Abs4( _mm_mul_ps( m_chunkBase[1], vec.m_chunkBase[1] ) ) ),//  abs( y * vec.y ) ) +
			MathlibSSE2::Abs4( _mm_mul_ps( m_chunkBase[2], vec.m_chunkBase[2] ) ) );//  abs( z * vec.z )
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterVector3::normalise( void )
	{
		ArrayReal sqLength = _mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], m_chunkBase[0] ),	//(x * x +
			_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ) ),	//y * y) +
		_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) );		//z * z )

		//Convert sqLength's 0s into 1, so that zero vectors remain as zero
		//Denormals are treated as 0 during the check.
		//Note: We could create a mask now and nuke nans after InvSqrt, however
		//generating the nans could impact performance in some architectures
		sqLength = MathlibSSE2::Cmov4( sqLength, MathlibSSE2::ONE,
										_mm_cmpgt_ps( sqLength, MathlibSSE2::FLOAT_MIN ) );
		ArrayReal invLength = MathlibSSE2::InvSqrtNonZero4( sqLength );
		m_chunkBase[0] = _mm_mul_ps( m_chunkBase[0], invLength ); //x * invLength
		m_chunkBase[1] = _mm_mul_ps( m_chunkBase[1], invLength ); //y * invLength
		m_chunkBase[2] = _mm_mul_ps( m_chunkBase[2], invLength ); //z * invLength
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterVector3::crossProduct( const ArrayVector3& rkVec ) const
    {
        return ArrayInterVector3(
			_mm_sub_ps(
				_mm_mul_ps( m_chunkBase[1], rkVec.m_chunkBase[2] ),
				_mm_mul_ps( m_chunkBase[2], rkVec.m_chunkBase[1] ) ),	//y * rkVec.z - z * rkVec.y
			_mm_sub_ps(
				_mm_mul_ps( m_chunkBase[2], rkVec.m_chunkBase[0] ),
				_mm_mul_ps( m_chunkBase[0], rkVec.m_chunkBase[2] ) ),	//z * rkVec.x - x * rkVec.z
			_mm_sub_ps(
				_mm_mul_ps( m_chunkBase[0], rkVec.m_chunkBase[1] ),
				_mm_mul_ps( m_chunkBase[1], rkVec.m_chunkBase[0] ) ) );	//x * rkVec.y - y * rkVec.x
    }
	inline ArrayInterVector3 ArrayInterVector3::crossProduct( const ArrayInterVector3& rkVec ) const
    {
        return ArrayInterVector3(
			_mm_sub_ps(
				_mm_mul_ps( m_chunkBase[1], rkVec.m_chunkBase[2] ),
				_mm_mul_ps( m_chunkBase[2], rkVec.m_chunkBase[1] ) ),	//y * rkVec.z - z * rkVec.y
			_mm_sub_ps(
				_mm_mul_ps( m_chunkBase[2], rkVec.m_chunkBase[0] ),
				_mm_mul_ps( m_chunkBase[0], rkVec.m_chunkBase[2] ) ),	//z * rkVec.x - x * rkVec.z
			_mm_sub_ps(
				_mm_mul_ps( m_chunkBase[0], rkVec.m_chunkBase[1] ),
				_mm_mul_ps( m_chunkBase[1], rkVec.m_chunkBase[0] ) ) );	//x * rkVec.y - y * rkVec.x
    }
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterVector3::midPoint( const ArrayVector3& rkVec ) const
    {
		return ArrayInterVector3(
			_mm_mul_ps( _mm_add_ps( m_chunkBase[0], rkVec.m_chunkBase[0] ), MathlibSSE2::HALF ),
			_mm_mul_ps( _mm_add_ps( m_chunkBase[1], rkVec.m_chunkBase[1] ), MathlibSSE2::HALF ),
			_mm_mul_ps( _mm_add_ps( m_chunkBase[2], rkVec.m_chunkBase[2] ), MathlibSSE2::HALF ) );
	}
	inline ArrayInterVector3 ArrayInterVector3::midPoint( const ArrayInterVector3& rkVec ) const
    {
		return ArrayInterVector3(
			_mm_mul_ps( _mm_add_ps( m_chunkBase[0], rkVec.m_chunkBase[0] ), MathlibSSE2::HALF ),
			_mm_mul_ps( _mm_add_ps( m_chunkBase[1], rkVec.m_chunkBase[1] ), MathlibSSE2::HALF ),
			_mm_mul_ps( _mm_add_ps( m_chunkBase[2], rkVec.m_chunkBase[2] ), MathlibSSE2::HALF ) );
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterVector3::makeFloor( const ArrayVector3& cmp )
    {
		ArrayReal * RESTRICT_ALIAS aChunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS bChunkBase = cmp.m_chunkBase;
		aChunkBase[0] = _mm_min_ps( aChunkBase[0], bChunkBase[0] );
		aChunkBase[1] = _mm_min_ps( aChunkBase[1], bChunkBase[1] );
		aChunkBase[2] = _mm_min_ps( aChunkBase[2], bChunkBase[2] );
	}
	inline void ArrayInterVector3::makeFloor( const ArrayInterVector3& cmp )
    {
		ArrayReal * RESTRICT_ALIAS aChunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS bChunkBase = cmp.m_chunkBase;
		aChunkBase[0] = _mm_min_ps( aChunkBase[0], bChunkBase[0] );
		aChunkBase[1] = _mm_min_ps( aChunkBase[1], bChunkBase[1] );
		aChunkBase[2] = _mm_min_ps( aChunkBase[2], bChunkBase[2] );
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterVector3::makeCeil( const ArrayVector3& cmp )
    {
		ArrayReal * RESTRICT_ALIAS aChunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS bChunkBase = cmp.m_chunkBase;
		aChunkBase[0] = _mm_max_ps( aChunkBase[0], bChunkBase[0] );
		aChunkBase[1] = _mm_max_ps( aChunkBase[1], bChunkBase[1] );
		aChunkBase[2] = _mm_max_ps( aChunkBase[2], bChunkBase[2] );
	}
	inline void ArrayInterVector3::makeCeil( const ArrayInterVector3& cmp )
    {
		ArrayReal * RESTRICT_ALIAS aChunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS bChunkBase = cmp.m_chunkBase;
		aChunkBase[0] = _mm_max_ps( aChunkBase[0], bChunkBase[0] );
		aChunkBase[1] = _mm_max_ps( aChunkBase[1], bChunkBase[1] );
		aChunkBase[2] = _mm_max_ps( aChunkBase[2], bChunkBase[2] );
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterVector3::perpendicular( void ) const
    {
        ArrayInterVector3 perp = this->crossProduct( ArrayVector3::UNIT_X );

		const ArrayReal mask = _mm_cmple_ps( perp.squaredLength(), MathlibSSE2::fSqEpsilon );
        // Check length
        if( _mm_movemask_ps( mask ) )
        {
            /* One or more of these vectors are the X axis multiplied by a scalar,
			   so we have to use another axis for those.
            */
            ArrayInterVector3 perp1 = this->crossProduct( ArrayVector3::UNIT_Y );
			perp.m_chunkBase[0] = MathlibSSE2::Cmov4( perp1.m_chunkBase[0], perp.m_chunkBase[0], mask );
			perp.m_chunkBase[1] = MathlibSSE2::Cmov4( perp1.m_chunkBase[1], perp.m_chunkBase[1], mask );
			perp.m_chunkBase[2] = MathlibSSE2::Cmov4( perp1.m_chunkBase[2], perp.m_chunkBase[2], mask );
        }
		perp.normalise();

        return perp;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterVector3::normalisedCopy( void ) const
	{
		ArrayReal sqLength = _mm_add_ps( _mm_add_ps(
			_mm_mul_ps( m_chunkBase[0], m_chunkBase[0] ),	//(x * x +
			_mm_mul_ps( m_chunkBase[1], m_chunkBase[1] ) ),	//y * y) +
		_mm_mul_ps( m_chunkBase[2], m_chunkBase[2] ) );		//z * z )

		//Convert sqLength's 0s into 1, so that zero vectors remain as zero
		//Denormals are treated as 0 during the check.
		//Note: We could create a mask now and nuke nans after InvSqrt, however
		//generating the nans could impact performance in some architectures
		sqLength = MathlibSSE2::Cmov4( sqLength, MathlibSSE2::ONE,
										_mm_cmpgt_ps( sqLength, MathlibSSE2::FLOAT_MIN ) );
		ArrayReal invLength = MathlibSSE2::InvSqrtNonZero4( sqLength );

		return ArrayInterVector3(
			_mm_mul_ps( m_chunkBase[0], invLength ),	//x * invLength
			_mm_mul_ps( m_chunkBase[1], invLength ),	//y * invLength
			_mm_mul_ps( m_chunkBase[2], invLength ) );	//z * invLength
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterVector3::reflect( const ArrayVector3& normal ) const
	{
		return ( *this - ( 2 * this->dotProduct( normal ) * normal ) );
	}
	inline ArrayInterVector3 ArrayInterVector3::reflect( const ArrayInterVector3& normal ) const
	{
		return ( *this - ( 2 * this->dotProduct( normal ) * normal ) );
	}
	//-----------------------------------------------------------------------------------
	inline int ArrayInterVector3::isNaN( void ) const
	{
		ArrayReal mask = _mm_and_ps( _mm_and_ps( 
			_mm_cmpeq_ps( m_chunkBase[0], m_chunkBase[0] ),
			_mm_cmpeq_ps( m_chunkBase[1], m_chunkBase[1] ) ),
			_mm_cmpeq_ps( m_chunkBase[2], m_chunkBase[2] ) );

		return _mm_movemask_ps( mask ) ^ 0x0000000f;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayInterVector3 ArrayInterVector3::primaryAxis( void ) const
	{
		// We could've used some operators, i.e.
		// xVec = MathlibSSE2::Cmov( ArrayInterVector3::UNIT_X, ArrayInterVector3::NEGATIVE_UNIT_X )
		// and so forth, which would've increased readability considerably. However,
		// MSVC's compiler ability to do constant propagation & remove dead code sucks,
		// which means it would try to cmov the Y & Z component even though we already
		// know it's always zero for both +x & -x. Therefore, we do it the manual
		// way. Doing this the "human readable way" results in massive amounts of wasted
		// instructions and stack memory abuse.
		// See Vector3::primaryAxis() to understand what's actually going on.
		ArrayReal absx = MathlibSSE2::Abs4( m_chunkBase[0] );
		ArrayReal absy = MathlibSSE2::Abs4( m_chunkBase[1] );
		ArrayReal absz = MathlibSSE2::Abs4( m_chunkBase[2] );

		//xVec = x > 0 ? Vector3::UNIT_X : Vector3::NEGATIVE_UNIT_X;
		ArrayReal sign = MathlibSSE2::Cmov4( _mm_set1_ps( 1.0f ), _mm_set1_ps( -1.0f ),
											_mm_cmpgt_ps( m_chunkBase[0], _mm_setzero_ps() ) );
		ArrayInterVector3 xVec( _mm_mul_ps( _mm_set1_ps( 1.0f ), sign ),
								_mm_setzero_ps(), _mm_setzero_ps() );

		//yVec = y > 0 ? Vector3::UNIT_Y : Vector3::NEGATIVE_UNIT_Y;
		sign = MathlibSSE2::Cmov4( _mm_set1_ps( 1.0f ), _mm_set1_ps( -1.0f ),
									_mm_cmpgt_ps( m_chunkBase[1], _mm_setzero_ps() ) );
		ArrayInterVector3 yVec( _mm_setzero_ps(), _mm_mul_ps( _mm_set1_ps( 1.0f ), sign ),
								_mm_setzero_ps() );

		//zVec = z > 0 ? Vector3::UNIT_Z : Vector3::NEGATIVE_UNIT_Z;
		sign = MathlibSSE2::Cmov4( _mm_set1_ps( 1.0f ), _mm_set1_ps( -1.0f ),
									_mm_cmpgt_ps( m_chunkBase[2], _mm_setzero_ps() ) );
		ArrayInterVector3 zVec( _mm_setzero_ps(), _mm_setzero_ps(),
								_mm_mul_ps( _mm_set1_ps( 1.0f ), sign ) );

		//xVec = absx > absz ? xVec : zVec
		ArrayReal mask = _mm_cmpgt_ps( absx, absz );
		xVec.m_chunkBase[0] = MathlibSSE2::Cmov4( xVec.m_chunkBase[0], zVec.m_chunkBase[0], mask );
		xVec.m_chunkBase[2] = MathlibSSE2::Cmov4( xVec.m_chunkBase[2], zVec.m_chunkBase[2], mask );

		//yVec = absy > absz ? yVec : zVec
		mask = _mm_cmpgt_ps( absy, absz );
		yVec.m_chunkBase[1] = MathlibSSE2::Cmov4( yVec.m_chunkBase[1], zVec.m_chunkBase[1], mask );
		yVec.m_chunkBase[2] = MathlibSSE2::Cmov4( yVec.m_chunkBase[2], zVec.m_chunkBase[2], mask );

		xVec.Cmov4( _mm_cmpgt_ps( absx, absy ), yVec );
		return xVec;
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayInterVector3::Cmov4( ArrayReal mask, const ArrayVector3 &replacement )
	{
		ArrayReal * RESTRICT_ALIAS aChunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS bChunkBase = replacement.m_chunkBase;
		aChunkBase[0] = MathlibSSE2::Cmov4( aChunkBase[0], bChunkBase[0], mask );
		aChunkBase[1] = MathlibSSE2::Cmov4( aChunkBase[1], bChunkBase[1], mask );
		aChunkBase[2] = MathlibSSE2::Cmov4( aChunkBase[2], bChunkBase[2], mask );
	}
	inline void ArrayInterVector3::Cmov4( ArrayReal mask, const ArrayInterVector3 &replacement )
	{
		ArrayReal * RESTRICT_ALIAS aChunkBase = m_chunkBase;
		const ArrayReal * RESTRICT_ALIAS bChunkBase = replacement.m_chunkBase;
		aChunkBase[0] = MathlibSSE2::Cmov4( aChunkBase[0], bChunkBase[0], mask );
		aChunkBase[1] = MathlibSSE2::Cmov4( aChunkBase[1], bChunkBase[1], mask );
		aChunkBase[2] = MathlibSSE2::Cmov4( aChunkBase[2], bChunkBase[2], mask );
	}
	//-----------------------------------------------------------------------------------

#undef DEFINE_OPERATION
#undef DEFINE_R_SCALAR_OPERATION
#undef DEFINE_R_OPERATION
#undef DEFINE_DIVISION
#undef DEFINE_R_SCALAR_DIVISION
#undef DEFINE_R_DIVISION

#undef DEFINE_UPDATE_OPERATION
#undef DEFINE_UPDATE_R_SCALAR_OPERATION
#undef DEFINE_UPDATE_R_OPERATION
#undef DEFINE_UPDATE_DIVISION
#undef DEFINE_UPDATE_R_SCALAR_DIVISION
#undef DEFINE_UPDATE_R_DIVISION
}
