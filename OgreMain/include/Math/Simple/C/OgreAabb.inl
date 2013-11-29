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
		mCenter	= (max + min) * 0.5f;
		mHalfSize	= (max - min) * 0.5f;
	}
	//-----------------------------------------------------------------------------------
	inline Aabb Aabb::newFromExtents( const Vector3& min, const Vector3& max )
	{
		assert( (min.x <= max.x && min.y <= max.y && min.z <= max.z) &&
                "The minimum corner of the box must be less than or equal to maximum corner" );
		Aabb retVal;
		retVal.mCenter		= (max + min) * 0.5f;
		retVal.mHalfSize	= (max - min) * 0.5f;
		return retVal;
	}
	//-----------------------------------------------------------------------------------
	inline Vector3 Aabb::getMinimum() const
	{
		return mCenter - mHalfSize;
	}
	//-----------------------------------------------------------------------------------
	inline Vector3 Aabb::getMaximum() const
	{
		return mCenter + mHalfSize;
	}
	//-----------------------------------------------------------------------------------
	inline void Aabb::merge( const Aabb& rhs )
	{
		Vector3 max( mCenter + mHalfSize );
		max.makeCeil( rhs.mCenter + rhs.mHalfSize );

		Vector3 min( mCenter - mHalfSize );
		min.makeFloor( rhs.mCenter - rhs.mHalfSize );

		if( max.x != std::numeric_limits<float>::infinity() &&
			max.y != std::numeric_limits<float>::infinity() &&
			max.z != std::numeric_limits<float>::infinity() )
		{
			mCenter	= ( max + min ) * 0.5f;
		}
		mHalfSize	= ( max - min ) * 0.5f;
	}
	//-----------------------------------------------------------------------------------
	inline void Aabb::merge( const Vector3& points )
	{
		Vector3 max( mCenter + mHalfSize );
		max.makeCeil( points );

		Vector3 min( mCenter - mHalfSize );
		min.makeFloor( points );

		if( max.x != std::numeric_limits<float>::infinity() &&
			max.y != std::numeric_limits<float>::infinity() &&
			max.z != std::numeric_limits<float>::infinity() )
		{
			mCenter	= ( max + min ) * 0.5f;
		}
		mHalfSize	= ( max - min ) * 0.5f;
	}
	//-----------------------------------------------------------------------------------
	inline bool Aabb::intersects( const Aabb& b2 ) const
	{
		Vector3 distance( mCenter - b2.mCenter );
		Vector3 sumHalfSizes( mHalfSize + b2.mHalfSize );

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
		const Vector3 size = mHalfSize * 2.0f;
		return size.x * size.y * size.z; // w * h * d
	}
	//-----------------------------------------------------------------------------------
	inline bool Aabb::contains( const Aabb &other ) const
	{
		Vector3 distance( mCenter - other.mCenter );

		// In theory, "abs( distance.x ) < mHalfSize - other.mHalfSize" should be more pipeline-
		// friendly because the processor can do the subtraction while the abs() is being performed,
		// however that variation won't handle the case where both boxes are infinite (will produce
		// nan instead and return false, when it should return true)

		//TODO: Profile whether '&&' or '&' is faster. Probably varies per architecture.
		return ( Math::Abs( distance.x ) + other.mHalfSize.x <= mHalfSize.x ) &
				( Math::Abs( distance.y ) + other.mHalfSize.y <= mHalfSize.y ) &
				( Math::Abs( distance.z ) + other.mHalfSize.z <= mHalfSize.z );
	}
	//-----------------------------------------------------------------------------------
	inline bool Aabb::contains( const Vector3 &v ) const
	{
		Vector3 distance( mCenter - v );

		// ( abs( distance.x ) <= mHalfSize.x &&
		//   abs( distance.y ) <= mHalfSize.y &&
		//   abs( distance.z ) <= mHalfSize.z )
		return ( Math::Abs( distance.x ) <= mHalfSize.x ) &
				( Math::Abs( distance.y ) <= mHalfSize.y ) &
				( Math::Abs( distance.z ) <= mHalfSize.z );
	}
	//-----------------------------------------------------------------------------------
	inline Real Aabb::distance( const Vector3 &v ) const
	{
		Vector3 distance( mCenter - v );

		// x = abs( distance.x ) - halfSize.x
		// y = abs( distance.y ) - halfSize.y
		// z = abs( distance.z ) - halfSize.z
		// return max( min( x, y, z ), 0 ); //Return minimum between xyz, clamp to zero

		distance.x = Math::Abs( distance.x ) - mHalfSize.x;
		distance.y = Math::Abs( distance.y ) - mHalfSize.y;
		distance.z = Math::Abs( distance.z ) - mHalfSize.z;

		return std::max( std::min( std::min( distance.x, distance.y ), distance.z ), 1.0f );
	}
	//-----------------------------------------------------------------------------------
	inline void Aabb::transformAffine( const Matrix4 &m )
	{
		assert( m.isAffine() );

		mCenter = m.transformAffine( mCenter );

		mHalfSize = Vector3(
				Math::Abs(m[0][0]) * mHalfSize.x + Math::Abs(m[0][1]) * mHalfSize.y + Math::Abs(m[0][2]) * mHalfSize.z, 
				Math::Abs(m[1][0]) * mHalfSize.x + Math::Abs(m[1][1]) * mHalfSize.y + Math::Abs(m[1][2]) * mHalfSize.z,
				Math::Abs(m[2][0]) * mHalfSize.x + Math::Abs(m[2][1]) * mHalfSize.y + Math::Abs(m[2][2]) * mHalfSize.z );
	}
	//-----------------------------------------------------------------------------------
	inline Real Aabb::getRadius() const
	{
		return sqrtf( mHalfSize.dotProduct( mHalfSize ) );
	}
	//-----------------------------------------------------------------------------------
	inline Real Aabb::getRadiusOrigin() const
	{
		Vector3 v( mCenter );
		v.makeAbs();
		v += mHalfSize;			
		return v.length();
	}
}
