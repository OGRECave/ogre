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
	inline void Aabb::setExtents( const Vector3& min, const Vector3& max )
	{
		assert( (min.x <= max.x && min.y <= max.y && min.z <= max.z) &&
                "The minimum corner of the box must be less than or equal to maximum corner" );
		m_center	= (max + min) * 0.5f;
		m_halfSize	= (max - min) * 0.5f;
	}
	//-----------------------------------------------------------------------------------
	inline Aabb Aabb::newFromExtents( const Vector3& min, const Vector3& max )
	{
		assert( (min.x <= max.x && min.y <= max.y && min.z <= max.z) &&
                "The minimum corner of the box must be less than or equal to maximum corner" );
		Aabb retVal;
		retVal.m_center		= (max + min) * 0.5f;
		retVal.m_halfSize	= (max - min) * 0.5f;
		return retVal;
	}
	//-----------------------------------------------------------------------------------
	inline Vector3 Aabb::getMinimum() const
	{
		return m_center - m_halfSize;
	}
	//-----------------------------------------------------------------------------------
	inline Vector3 Aabb::getMaximum() const
	{
		return m_center + m_halfSize;
	}
	//-----------------------------------------------------------------------------------
	inline void Aabb::merge( const Aabb& rhs )
	{
		Vector3 max( m_center + m_halfSize );
		max.makeCeil( rhs.m_center + rhs.m_halfSize );

		Vector3 min( m_center - m_halfSize );
		min.makeFloor( rhs.m_center - rhs.m_halfSize );

		if( max.x != std::numeric_limits<float>::infinity() &&
			max.y != std::numeric_limits<float>::infinity() &&
			max.z != std::numeric_limits<float>::infinity() )
		{
			m_center	= ( max + min ) * 0.5f;
		}
		m_halfSize	= ( max - min ) * 0.5f;
	}
	//-----------------------------------------------------------------------------------
	inline void Aabb::merge( const Vector3& points )
	{
		Vector3 max( m_center + m_halfSize );
		max.makeCeil( points );

		Vector3 min( m_center - m_halfSize );
		min.makeFloor( points );

		if( max.x != std::numeric_limits<float>::infinity() &&
			max.y != std::numeric_limits<float>::infinity() &&
			max.z != std::numeric_limits<float>::infinity() )
		{
			m_center	= ( max + min ) * 0.5f;
		}
		m_halfSize	= ( max - min ) * 0.5f;
	}
	//-----------------------------------------------------------------------------------
	inline bool Aabb::intersects( const Aabb& b2 ) const
	{
		Vector3 distance( m_center - b2.m_center );
		Vector3 sumHalfSizes( m_halfSize + b2.m_halfSize );

		// ( abs( center.x - center2.x ) <= halfSize.x + halfSize2.x &&
		//   abs( center.y - center2.y ) <= halfSize.y + halfSize2.y &&
		//   abs( center.z - center2.z ) <= halfSize.z + halfSize2.z )
		//TODO: Profile whether '&&' or '&' is faster. Probably varies per architecture.
		return ( Math::Abs( distance.x ) <= sumHalfSizes.x ) &
				( Math::Abs( distance.y ) <= sumHalfSizes.y ) &
				( Math::Abs( distance.z ) <= sumHalfSizes.z );
	}
	//-----------------------------------------------------------------------------------
	inline Real Aabb::volume(void) const
	{
		const Vector3 size = m_halfSize * 2.0f;
		return size.x * size.y * size.z; // w * h * d
	}
	//-----------------------------------------------------------------------------------
	inline bool Aabb::contains( const Aabb &other ) const
	{
		Vector3 distance( m_center - other.m_center );

		// In theory, "abs( distance.x ) < m_halfSize - other.m_halfSize" should be more pipeline-
		// friendly because the processor can do the subtraction while the abs() is being performed,
		// however that variation won't handle the case where both boxes are infinite (will produce
		// nan instead and return false, when it should return true)

		//TODO: Profile whether '&&' or '&' is faster. Probably varies per architecture.
		return ( Math::Abs( distance.x ) + other.m_halfSize.x <= m_halfSize.x ) &
				( Math::Abs( distance.y ) + other.m_halfSize.y <= m_halfSize.y ) &
				( Math::Abs( distance.z ) + other.m_halfSize.z <= m_halfSize.z );
	}
	//-----------------------------------------------------------------------------------
	inline bool Aabb::contains( const Vector3 &v ) const
	{
		Vector3 distance( m_center - v );

		// ( abs( distance.x ) <= m_halfSize.x &&
		//   abs( distance.y ) <= m_halfSize.y &&
		//   abs( distance.z ) <= m_halfSize.z )
		return ( Math::Abs( distance.x ) <= m_halfSize.x ) &
				( Math::Abs( distance.y ) <= m_halfSize.y ) &
				( Math::Abs( distance.z ) <= m_halfSize.z );
	}
	//-----------------------------------------------------------------------------------
	inline Real Aabb::distance( const Vector3 &v ) const
	{
		Vector3 distance( m_center - v );

		// x = abs( distance.x ) - halfSize.x
		// y = abs( distance.y ) - halfSize.y
		// z = abs( distance.z ) - halfSize.z
		// return max( min( x, y, z ), 0 ); //Return minimum between xyz, clamp to zero

		distance.x = Math::Abs( distance.x ) - m_halfSize.x;
		distance.y = Math::Abs( distance.y ) - m_halfSize.y;
		distance.z = Math::Abs( distance.z ) - m_halfSize.z;

		return std::max( std::min( std::min( distance.x, distance.y ), distance.z ), 1.0f );
	}
	//-----------------------------------------------------------------------------------
	inline void Aabb::transformAffine( const Matrix4 &m )
	{
		assert( m.isAffine() );

		m_center = m.transformAffine( m_center );

		m_halfSize = Vector3(
				Math::Abs(m[0][0]) * m_halfSize.x + Math::Abs(m[0][1]) * m_halfSize.y + Math::Abs(m[0][2]) * m_halfSize.z, 
				Math::Abs(m[1][0]) * m_halfSize.x + Math::Abs(m[1][1]) * m_halfSize.y + Math::Abs(m[1][2]) * m_halfSize.z,
				Math::Abs(m[2][0]) * m_halfSize.x + Math::Abs(m[2][1]) * m_halfSize.y + Math::Abs(m[2][2]) * m_halfSize.z );
	}
	//-----------------------------------------------------------------------------------
	inline Real Aabb::getRadius() const
	{
		return sqrtf( m_halfSize.dotProduct( m_halfSize ) );
	}
	//-----------------------------------------------------------------------------------
	inline Real Aabb::getRadiusOrigin() const
	{
		Vector3 v( m_center );
		v.makeAbs();
		v += m_halfSize;			
		return v.length();
	}
}
