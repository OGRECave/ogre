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
	//-----------------------------------------------------------------------------------
	inline ArrayVector3 ArrayAabb::getMinimum() const
	{
		return m_center - m_halfSize;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayVector3 ArrayAabb::getMaximum() const
	{
		return m_center + m_halfSize;
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayAabb::merge( const ArrayAabb& rhs )
	{
		ArrayVector3 max( m_center + m_halfSize );
		max.makeCeil( rhs.m_center + rhs.m_halfSize );

		ArrayVector3 min( m_center - m_halfSize );
		min.makeFloor( rhs.m_center - rhs.m_halfSize );

		m_center	= ( max + min ) * Mathlib::HALF;
		m_halfSize	= ( max - min ) * Mathlib::HALF;
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayAabb::merge( const ArrayVector3& points )
	{
		ArrayVector3 max( m_center + m_halfSize );
		max.makeCeil( points );

		ArrayVector3 min( m_center - m_halfSize );
		min.makeFloor( points );

		m_center	= ( max + min ) * Mathlib::HALF;
		m_halfSize	= ( max - min ) * Mathlib::HALF;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayMaskR ArrayAabb::intersects( const ArrayAabb& b2 ) const
	{
		ArrayVector3 distance( m_center - b2.m_center );
		ArrayVector3 sumHalfSizes( m_halfSize + b2.m_halfSize );

		// ( abs( center.x - center2.x ) <= halfSize.x + halfSize2.x &&
		//   abs( center.y - center2.y ) <= halfSize.y + halfSize2.y &&
		//   abs( center.z - center2.z ) <= halfSize.z + halfSize2.z )
		ArrayReal maskX = _mm_cmple_ps( MathlibSSE2::Abs4( distance.m_chunkBase[0] ),
										sumHalfSizes.m_chunkBase[0] );
		ArrayReal maskY = _mm_cmple_ps( MathlibSSE2::Abs4( distance.m_chunkBase[1] ),
										sumHalfSizes.m_chunkBase[1] );
		ArrayReal maskZ = _mm_cmple_ps( MathlibSSE2::Abs4( distance.m_chunkBase[2] ),
										sumHalfSizes.m_chunkBase[2] );
		
		return _mm_and_ps( _mm_and_ps( maskX, maskY ), maskZ );
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayAabb::volume(void) const
	{
		ArrayReal w = _mm_add_ps( m_halfSize.m_chunkBase[0], m_halfSize.m_chunkBase[0] ); // x * 2
		ArrayReal h = _mm_add_ps( m_halfSize.m_chunkBase[1], m_halfSize.m_chunkBase[1] ); // y * 2
		ArrayReal d = _mm_add_ps( m_halfSize.m_chunkBase[2], m_halfSize.m_chunkBase[2] ); // z * 2

		return _mm_mul_ps( _mm_mul_ps( w, h ), d ); // w * h * d
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayAabb::contains( const ArrayAabb &other ) const
	{
		ArrayVector3 distance( m_center - other.m_center );

		// In theory, "abs( distance.x ) < m_halfSize - other.m_halfSize" should be more pipeline-
		// friendly because the processor can do the subtraction while the abs4() is being performed,
		// however that variation won't handle the case where both boxes are infinite (will produce
		// nan instead and return false, when it should return true)

		// ( abs( distance.x ) + other.m_halfSize.x <= m_halfSize.x &&
		//   abs( distance.y ) + other.m_halfSize.y <= m_halfSize.y &&
		//   abs( distance.z ) + other.m_halfSize.z <= m_halfSize.z )
		ArrayReal maskX = _mm_cmple_ps( _mm_add_ps( MathlibSSE2::Abs4( distance.m_chunkBase[0] ),
										other.m_halfSize.m_chunkBase[0] ), m_halfSize.m_chunkBase[0] );
		ArrayReal maskY = _mm_cmple_ps( _mm_add_ps( MathlibSSE2::Abs4( distance.m_chunkBase[1] ),
										other.m_halfSize.m_chunkBase[1] ), m_halfSize.m_chunkBase[1] );
		ArrayReal maskZ = _mm_cmple_ps( _mm_add_ps( MathlibSSE2::Abs4( distance.m_chunkBase[2] ),
										other.m_halfSize.m_chunkBase[2] ), m_halfSize.m_chunkBase[2] );

		return _mm_and_ps( _mm_and_ps( maskX, maskY ), maskZ );
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayAabb::contains( const ArrayVector3 &v ) const
	{
		ArrayVector3 distance( m_center - v );

		// ( abs( distance.x ) <= m_halfSize.x &&
		//   abs( distance.y ) <= m_halfSize.y &&
		//   abs( distance.z ) <= m_halfSize.z )
		ArrayReal maskX = _mm_cmple_ps( MathlibSSE2::Abs4( distance.m_chunkBase[0] ),
										m_halfSize.m_chunkBase[0] );
		ArrayReal maskY = _mm_cmple_ps( MathlibSSE2::Abs4( distance.m_chunkBase[1] ),
										m_halfSize.m_chunkBase[1] );
		ArrayReal maskZ = _mm_cmple_ps( MathlibSSE2::Abs4( distance.m_chunkBase[2] ),
										m_halfSize.m_chunkBase[2] );

		return _mm_and_ps( _mm_and_ps( maskX, maskY ), maskZ );
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayAabb::distance( const ArrayVector3 &v ) const
	{
		ArrayVector3 distance( m_center - v );

		// x = abs( distance.x ) - halfSize.x
		// y = abs( distance.y ) - halfSize.y
		// z = abs( distance.z ) - halfSize.z
		// return max( min( x, y, z ), 0 ); //Return minimum between xyz, clamp to zero
		distance.m_chunkBase[0] = _mm_sub_ps( MathlibSSE2::Abs4( distance.m_chunkBase[0] ),
												m_halfSize.m_chunkBase[0] );
		distance.m_chunkBase[1] = _mm_sub_ps( MathlibSSE2::Abs4( distance.m_chunkBase[1] ),
												m_halfSize.m_chunkBase[1] );
		distance.m_chunkBase[2] = _mm_sub_ps( MathlibSSE2::Abs4( distance.m_chunkBase[2] ),
												m_halfSize.m_chunkBase[2] );

		return _mm_max_ps( _mm_min_ps( _mm_min_ps( distance.m_chunkBase[0],
					distance.m_chunkBase[1] ), distance.m_chunkBase[2] ), _mm_setzero_ps() );
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayAabb::transformAffine( const ArrayMatrix4 &m )
	{
		assert( m.isAffine() );

		m_center = m * m_center;

		ArrayReal x = _mm_mul_ps( Mathlib::Abs4( m.m_chunkBase[2] ), m_halfSize.m_chunkBase[2] );	// abs( m02 ) * z +
		x = _mm_madd_ps( Mathlib::Abs4( m.m_chunkBase[1] ), m_halfSize.m_chunkBase[1], x );			// abs( m01 ) * y +
		x = _mm_madd_ps( Mathlib::Abs4( m.m_chunkBase[0] ), m_halfSize.m_chunkBase[0], x );			// abs( m00 ) * x

		ArrayReal y = _mm_mul_ps( Mathlib::Abs4( m.m_chunkBase[6] ), m_halfSize.m_chunkBase[2] );	// abs( m12 ) * z +
		y = _mm_madd_ps( Mathlib::Abs4( m.m_chunkBase[5] ), m_halfSize.m_chunkBase[1], y );			// abs( m11 ) * y +
		y = _mm_madd_ps( Mathlib::Abs4( m.m_chunkBase[4] ), m_halfSize.m_chunkBase[0], y );			// abs( m10 ) * x

		ArrayReal z = _mm_mul_ps( Mathlib::Abs4( m.m_chunkBase[10] ), m_halfSize.m_chunkBase[2] );	// abs( m22 ) * z +
		z = _mm_madd_ps( Mathlib::Abs4( m.m_chunkBase[9] ), m_halfSize.m_chunkBase[1], z );			// abs( m21 ) * y +
		z = _mm_madd_ps( Mathlib::Abs4( m.m_chunkBase[8] ), m_halfSize.m_chunkBase[0], z );			// abs( m20 ) * x

		//Handle infinity boxes not becoming NaN. Null boxes containing -Inf will still have NaNs
		//(which is ok since we need them to say 'false' to intersection tests)
		x = MathlibSSE2::CmovRobust( MathlibSSE2::INFINITEA, x,
									_mm_cmpeq_ps( m_halfSize.m_chunkBase[0], MathlibSSE2::INFINITEA ) );
		y = MathlibSSE2::CmovRobust( MathlibSSE2::INFINITEA, y,
									_mm_cmpeq_ps( m_halfSize.m_chunkBase[1], MathlibSSE2::INFINITEA ) );
		z = MathlibSSE2::CmovRobust( MathlibSSE2::INFINITEA, z,
									_mm_cmpeq_ps( m_halfSize.m_chunkBase[2], MathlibSSE2::INFINITEA ) );

		m_halfSize = ArrayVector3( x, y, z );
	}
	//-----------------------------------------------------------------------------------
}
