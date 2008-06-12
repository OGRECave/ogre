#ifndef _VECTOR3_H_
#define _VECTOR3_H_

#include "math.h"

class Vector3
{
public:
	float x, y, z;        

	inline Vector3() {}
	
	inline Vector3( float nx, float ny, float nz ) : x(nx), y(ny), z(nz) {}
	
	inline Vector3( float v[3] ) : x(v[0]), y(v[1]), z(v[2]) {}
	
	inline Vector3( int v[3] ): x((float)v[0]), y((float)v[1]), z((float)v[2]) {}
	
	inline Vector3( const float* const v ) : x(v[0]), y(v[1]), z(v[2]) {}
	
	inline Vector3( const Vector3& v ) : x(v.x), y(v.y), z(v.z) {}
	
	inline float operator [] ( unsigned i ) const
	{
		return *(&x+i);
	}
	
	inline float& operator [] ( unsigned i )
	{
		return *(&x+i);
	}
	
	inline Vector3& operator = ( const Vector3& v )
	{
		x = v.x;
		y = v.y;
		z = v.z;            
		
		return *this;
	}
	
	inline bool operator == ( const Vector3& v ) const
	{
		return ( x == v.x && y == v.y && z == v.z );
	}
	
	inline bool operator != ( const Vector3& v ) const
	{
		return ( x != v.x || y != v.y || z != v.z );
	}
	
	// arithmetic operations
	inline Vector3 operator + ( const Vector3& v ) const
	{		
		return Vector3(x + v.x, y + v.y, z + v.z);
	}
	
	inline Vector3 operator - ( const Vector3& v ) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}
	
	inline Vector3 operator * ( float f ) const
	{
		return Vector3(x * f, y * f, z * f);
	}
	
	inline Vector3 operator * ( const Vector3& v) const
	{
		return Vector3(x * v.x, y * v.y, z * v.z);
	}
	
	inline Vector3 operator / ( float f ) const
	{
		f = 1.0f / f;		
		return Vector3(x * f, y * f, z * f);
	}
	
	inline Vector3 operator - () const
	{
		return Vector3( -x, -y, -z);
	}
	
	inline friend Vector3 operator * ( float f, const Vector3& v )
	{
		return Vector3(f * v.x, f * v.y, f * v.z);
	}
	
	// arithmetic updates
	inline Vector3& operator += ( const Vector3& v )
	{
		x += v.x;
		y += v.y;
		z += v.z;
		
		return *this;
	}
	
	inline Vector3& operator -= ( const Vector3& v )
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		
		return *this;
	}
	
	inline Vector3& operator *= ( float f )
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}
	
	inline Vector3& operator /= ( float f )
	{
		f = 1.0f / f;
		
		x *= f;
		y *= f;
		z *= f;
		
		return *this;
	}
	
	inline float length () const
	{
		return (float)sqrt( x * x + y * y + z * z );
	}
	
	inline float squaredLength () const
	{
		return x * x + y * y + z * z;
	}
	
	inline float dotProduct(const Vector3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}
	
	inline Vector3 & normalise()
	{
		float f = (float)sqrt( x * x + y * y + z * z );
		
		// Will also work for zero-sized vectors, but will change nothing
		if ( f > 1e-06f )
		{
			f = 1.0f / f;
			x *= f;
			y *= f;
			z *= f;
		}
		
		return *this;
	}
	
	inline Vector3 crossProduct( const Vector3& v ) const
	{
		return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
	
	inline Vector3 midPoint( const Vector3& v ) const
	{
		return Vector3( ( x + v.x ) * 0.5f, ( y + v.y ) * 0.5f, ( z + v.z ) * 0.5f );
	}
	
	inline bool operator < ( const Vector3& v ) const
	{
		return ( x < v.x && y < v.y && z < v.z );
	}
	
	inline bool operator > ( const Vector3& v ) const
	{
		return ( x > v.x && y > v.y && z > v.z );
	}
	
	inline void makeFloor( const Vector3& v )
	{
		if( v.x < x ) x = v.x;
		if( v.y < y ) y = v.y;
		if( v.z < z ) z = v.z;
	}
	
	inline void makeCeil( const Vector3& v )
	{
		if( v.x > x ) x = v.x;
		if( v.y > y ) y = v.y;
		if( v.z > z ) z = v.z;
	}
	
	inline Vector3 perpendicular(void)
	{
		static float fSquareZero = 1e-06f * 1e-06f;
		
		Vector3 perp = this->crossProduct( Vector3::UNIT_X );
		
		// Check length
		if( perp.squaredLength() < fSquareZero )
		{
		/* This vector is the Y axis multiplied by a scalar, so we have 
		to use another axis.
			*/
			perp = this->crossProduct( Vector3::UNIT_Y );
		}
		
		return perp;
	}
	
	// special points
	static const Vector3 ZERO;
	static const Vector3 UNIT_X;
	static const Vector3 UNIT_Y;
	static const Vector3 UNIT_Z;
	static const Vector3 UNIT_SCALE;
};

#endif // _VECTOR3_H_

