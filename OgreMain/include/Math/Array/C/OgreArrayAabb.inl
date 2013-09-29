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

		m_center	= ( max + min ) * 0.5f;
		m_halfSize	= ( max - min ) * 0.5f;
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayAabb::merge( const ArrayVector3& points )
	{
		ArrayVector3 max( m_center + m_halfSize );
		max.makeCeil( points );

		ArrayVector3 min( m_center - m_halfSize );
		min.makeFloor( points );

		m_center	= ( max + min ) * 0.5f;
		m_halfSize	= ( max - min ) * 0.5f;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayMaskR ArrayAabb::intersects( const ArrayAabb& b2 ) const
	{
		ArrayVector3 distance( m_center - b2.m_center );
		ArrayVector3 sumHalfSizes( m_halfSize + b2.m_halfSize );

		// ( abs( center.x - center2.x ) <= halfSize.x + halfSize2.x &&
		//   abs( center.y - center2.y ) <= halfSize.y + halfSize2.y &&
		//   abs( center.z - center2.z ) <= halfSize.z + halfSize2.z )
		ArrayMaskR maskX = Math::Abs( distance.m_chunkBase[0] ) <= sumHalfSizes.m_chunkBase[0];
		ArrayMaskR maskY = Math::Abs( distance.m_chunkBase[1] ) <= sumHalfSizes.m_chunkBase[1];
		ArrayMaskR maskZ = Math::Abs( distance.m_chunkBase[2] ) <= sumHalfSizes.m_chunkBase[2];
		
		return maskX & maskY & maskZ;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayAabb::volume(void) const
	{
		ArrayReal w = m_halfSize.m_chunkBase[0] + m_halfSize.m_chunkBase[0]; // x * 2
		ArrayReal h = m_halfSize.m_chunkBase[1] + m_halfSize.m_chunkBase[1]; // y * 2
		ArrayReal d = m_halfSize.m_chunkBase[2] + m_halfSize.m_chunkBase[2]; // z * 2

		return w * h * d;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayMaskR ArrayAabb::contains( const ArrayAabb &other ) const
	{
		ArrayVector3 distance( m_center - other.m_center );

		// In theory, "abs( distance.x ) < m_halfSize - other.m_halfSize" should be more pipeline-
		// friendly because the processor can do the subtraction while the abs4() is being performed,
		// however that variation won't handle the case where both boxes are infinite (will produce
		// nan instead and return false, when it should return true)

		// ( abs( distance.x ) + other.m_halfSize.x <= m_halfSize.x &&
		//   abs( distance.y ) + other.m_halfSize.y <= m_halfSize.y &&
		//   abs( distance.z ) + other.m_halfSize.z <= m_halfSize.z )
		ArrayMaskR maskX = (Math::Abs( distance.m_chunkBase[0] ) + other.m_halfSize.m_chunkBase[0]) <= m_halfSize.m_chunkBase[0];
		ArrayMaskR maskY = (Math::Abs( distance.m_chunkBase[1] ) + other.m_halfSize.m_chunkBase[1]) <= m_halfSize.m_chunkBase[1];
		ArrayMaskR maskZ = (Math::Abs( distance.m_chunkBase[2] ) + other.m_halfSize.m_chunkBase[2]) <= m_halfSize.m_chunkBase[2];

		return maskX & maskY & maskZ;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayMaskR ArrayAabb::contains( const ArrayVector3 &v ) const
	{
		ArrayVector3 distance( m_center - v );

		// ( abs( distance.x ) <= m_halfSize.x &&
		//   abs( distance.y ) <= m_halfSize.y &&
		//   abs( distance.z ) <= m_halfSize.z )
		ArrayMaskR maskX = Math::Abs( distance.m_chunkBase[0] ) <= m_halfSize.m_chunkBase[0];
		ArrayMaskR maskY = Math::Abs( distance.m_chunkBase[1] ) <= m_halfSize.m_chunkBase[1];
		ArrayMaskR maskZ = Math::Abs( distance.m_chunkBase[2] ) <= m_halfSize.m_chunkBase[2];

		return maskX & maskY & maskZ;
	}
	//-----------------------------------------------------------------------------------
	inline ArrayReal ArrayAabb::distance( const ArrayVector3 &v ) const
	{
		ArrayVector3 distance( m_center - v );

		// x = abs( distance.x ) - halfSize.x
		// y = abs( distance.y ) - halfSize.y
		// z = abs( distance.z ) - halfSize.z
		// return max( min( x, y, z ), 0 ); //Return minimum between xyz, clamp to zero
		distance.m_chunkBase[0] = Math::Abs( distance.m_chunkBase[0] ) - m_halfSize.m_chunkBase[0];
		distance.m_chunkBase[1] = Math::Abs( distance.m_chunkBase[1] ) - m_halfSize.m_chunkBase[1];
		distance.m_chunkBase[2] = Math::Abs( distance.m_chunkBase[2] ) - m_halfSize.m_chunkBase[2];

		return Ogre::max( Ogre::min( Ogre::min(
				distance.m_chunkBase[0], distance.m_chunkBase[1] ), distance.m_chunkBase[2] ), 0.0f );
	}
	//-----------------------------------------------------------------------------------
	inline void ArrayAabb::transformAffine( const ArrayMatrix4 &m )
	{
		assert( m.isAffine() );

		m_center = m * m_center;

		ArrayReal x = Math::Abs( m.m_chunkBase[2] ) * m_halfSize.m_chunkBase[2];			// abs( m02 ) * z +
		x = ogre_madd( Math::Abs( m.m_chunkBase[1] ), m_halfSize.m_chunkBase[1], x );	// abs( m01 ) * y +
		x = ogre_madd( Math::Abs( m.m_chunkBase[0] ), m_halfSize.m_chunkBase[0], x );	// abs( m00 ) * x

		ArrayReal y = Math::Abs( m.m_chunkBase[6] ) * m_halfSize.m_chunkBase[2];			// abs( m12 ) * z +
		y = ogre_madd( Math::Abs( m.m_chunkBase[5] ), m_halfSize.m_chunkBase[1], y );	// abs( m11 ) * y +
		y = ogre_madd( Math::Abs( m.m_chunkBase[4] ), m_halfSize.m_chunkBase[0], y );	// abs( m10 ) * x

		ArrayReal z = Math::Abs( m.m_chunkBase[10] ) * m_halfSize.m_chunkBase[2];		// abs( m22 ) * z +
		z = ogre_madd( Math::Abs( m.m_chunkBase[9] ), m_halfSize.m_chunkBase[1], z );	// abs( m21 ) * y +
		z = ogre_madd( Math::Abs( m.m_chunkBase[8] ), m_halfSize.m_chunkBase[0], z );	// abs( m20 ) * x

		//Handle infinity boxes not becoming NaN. Null boxes containing -Inf will still have NaNs
		//(which is ok since we need them to say 'false' to intersection tests)
		x = MathlibC::CmovRobust( MathlibC::INFINITY, x, m_halfSize.m_chunkBase[0] == MathlibC::INFINITY );
		y = MathlibC::CmovRobust( MathlibC::INFINITY, y, m_halfSize.m_chunkBase[1] == MathlibC::INFINITY );
		z = MathlibC::CmovRobust( MathlibC::INFINITY, z, m_halfSize.m_chunkBase[2] == MathlibC::INFINITY );

		m_halfSize = ArrayVector3( x, y, z );
	}
	//-----------------------------------------------------------------------------------
}
