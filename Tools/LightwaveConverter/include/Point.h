#ifndef _POINT_H_
#define _POINT_H_

#include "lwo.h"
#include "Vector3.h"

using namespace std;

class Point3 {
	public:
		inline Point3() {}
		
		inline Point3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}
		
		inline Point3& operator =(const Vector3& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			return (*this);
		}
		
		inline Point3& operator *=(float t)
		{
			x *= t;
			y *= t;
			z *= t;
			return (*this);
		}
		
		inline Point3& operator /=(float t)
		{
			float f = 1.0F / t;
			x *= f;
			y *= f;
			z *= f;
			return (*this);
		}
		
        inline bool operator == ( const Point3& p ) const
        {
            return ( x == p.x && y == p.y && z == p.z );
        }

        inline bool operator != ( const Point3& p ) const
        {
            return ( x != p.x || y != p.y || z != p.z );
        }

		inline Point3 operator -(void) const
		{
			return (Point3(-x, -y, -z));
		}
		
		// Sum of point and vector (direction) is a point
		inline Point3 operator +(const Vector3& v) const
		{
			return (Point3(x + v.x, y + v.y, z + v.z));
		}
		
		// Difference of point and vector (direction) is a point
		inline Point3 operator -(const Vector3& v) const
		{
			return (Point3(x - v.x, y - v.y, z - v.z));
		}
		
		// Difference between to points is a vector (direction)
		inline Vector3 operator -(const Point3& p) const
		{
			return (Vector3(x - p.x, y - p.y, z - p.z));
		}
		
		inline Point3 operator *(float t) const
		{
			return (Point3(x * t, y * t, z * t));
		}
		
		inline Point3 operator /(float t) const
		{
			float f = 1.0F / t;
			return (Point3(x * f, y * f, z * f));
		}
		
		// Dot product
		inline float operator *(const Vector3& v) const
		{
			return (x * v.x + y * v.y + z * v.z);
		}

		float x, y, z;
};

#endif // _POINT_H_

