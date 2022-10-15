/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreStableHeaders.h"

namespace Ogre
{

    constexpr Real Math::POS_INFINITY;
    constexpr Real Math::NEG_INFINITY;
    constexpr Real Math::PI;
    constexpr Real Math::TWO_PI;
    constexpr Real Math::HALF_PI;
    constexpr float Math::fDeg2Rad;
    constexpr float Math::fRad2Deg;
    constexpr Real Math::LOG2;

    int Math::mTrigTableSize;
   Math::AngleUnit Math::msAngleUnit;

    float  Math::mTrigTableFactor;
    float *Math::mSinTable = NULL;
    float *Math::mTanTable = NULL;

    Math::RandomValueProvider* Math::mRandProvider = NULL;

    //-----------------------------------------------------------------------
    Math::Math( unsigned int trigTableSize )
    {
        msAngleUnit = AU_DEGREE;
        mTrigTableSize = trigTableSize;
        mTrigTableFactor = mTrigTableSize / Math::TWO_PI;

        mSinTable = OGRE_ALLOC_T(float, mTrigTableSize, MEMCATEGORY_GENERAL);
        mTanTable = OGRE_ALLOC_T(float, mTrigTableSize, MEMCATEGORY_GENERAL);

        buildTrigTables();
    }

    //-----------------------------------------------------------------------
    Math::~Math()
    {
        OGRE_FREE(mSinTable, MEMCATEGORY_GENERAL);
        OGRE_FREE(mTanTable, MEMCATEGORY_GENERAL);
    }

    //-----------------------------------------------------------------------
    void Math::buildTrigTables(void)
    {
        // Build trig lookup tables
        // Could get away with building only PI sized Sin table but simpler this 
        // way. Who cares, it'll ony use an extra 8k of memory anyway and I like 
        // simplicity.
        float angle;
        for (int i = 0; i < mTrigTableSize; ++i)
        {
            angle = Math::TWO_PI * i / Real(mTrigTableSize);
            mSinTable[i] = std::sin(angle);
            mTanTable[i] = std::tan(angle);
        }
    }
    //-----------------------------------------------------------------------   
    float Math::SinTable (float fValue)
    {
        // Convert range to index values, wrap if required
        int idx;
        if (fValue >= 0)
        {
            idx = int(fValue * mTrigTableFactor) % mTrigTableSize;
        }
        else
        {
            idx = mTrigTableSize - (int(-fValue * mTrigTableFactor) % mTrigTableSize) - 1;
        }

        return mSinTable[idx];
    }
    //-----------------------------------------------------------------------
    float Math::TanTable (float fValue)
    {
        // Convert range to index values, wrap if required
        int idx = int(fValue * mTrigTableFactor) % mTrigTableSize;
        return mTanTable[idx];
    }
    //-----------------------------------------------------------------------
    Radian Math::ACos (Real fValue)
    {
        if ( -1.0 < fValue )
        {
            if ( fValue < 1.0 )
                return Radian(std::acos(fValue));
            else
                return Radian(0.0);
        }
        else
        {
            return Radian(PI);
        }
    }
    //-----------------------------------------------------------------------
    Radian Math::ASin (Real fValue)
    {
        if ( -1.0 < fValue )
        {
            if ( fValue < 1.0 )
                return Radian(std::asin(fValue));
            else
                return Radian(HALF_PI);
        }
        else
        {
            return Radian(-HALF_PI);
        }
    }
    //-----------------------------------------------------------------------
    float Math::UnitRandom ()
    {
        if (mRandProvider)
            return mRandProvider->getRandomUnit();
        else return rand() / float(RAND_MAX);
    }
    
    //-----------------------------------------------------------------------
    void Math::SetRandomValueProvider(RandomValueProvider* provider)
    {
        mRandProvider = provider;
    }

   //-----------------------------------------------------------------------
    void Math::setAngleUnit(Math::AngleUnit unit)
   {
       msAngleUnit = unit;
   }
   //-----------------------------------------------------------------------
   Math::AngleUnit Math::getAngleUnit(void)
   {
       return msAngleUnit;
   }
    //-----------------------------------------------------------------------
    float Math::AngleUnitsToRadians(float angleunits)
    {
       if (msAngleUnit == AU_DEGREE)
           return angleunits * fDeg2Rad;
       else
           return angleunits;
    }

    //-----------------------------------------------------------------------
    float Math::RadiansToAngleUnits(float radians)
    {
       if (msAngleUnit == AU_DEGREE)
           return radians * fRad2Deg;
       else
           return radians;
    }

    //-----------------------------------------------------------------------
    float Math::AngleUnitsToDegrees(float angleunits)
    {
       if (msAngleUnit == AU_RADIAN)
           return angleunits * fRad2Deg;
       else
           return angleunits;
    }

    //-----------------------------------------------------------------------
    float Math::DegreesToAngleUnits(float degrees)
    {
       if (msAngleUnit == AU_RADIAN)
           return degrees * fDeg2Rad;
       else
           return degrees;
    }

    //-----------------------------------------------------------------------
    bool Math::pointInTri2D(const Vector2& p, const Vector2& a, 
        const Vector2& b, const Vector2& c)
    {
        // Winding must be consistent from all edges for point to be inside
        Vector2 v1, v2;
        Real dot[3];
        bool zeroDot[3];

        v1 = b - a;
        v2 = p - a;

        // Note we don't care about normalisation here since sign is all we need
        // It means we don't have to worry about magnitude of cross products either
        dot[0] = v1.crossProduct(v2);
        zeroDot[0] = Math::RealEqual(dot[0], 0.0f, 1e-3);


        v1 = c - b;
        v2 = p - b;

        dot[1] = v1.crossProduct(v2);
        zeroDot[1] = Math::RealEqual(dot[1], 0.0f, 1e-3);

        // Compare signs (ignore colinear / coincident points)
        if(!zeroDot[0] && !zeroDot[1] 
        && Math::Sign(dot[0]) != Math::Sign(dot[1]))
        {
            return false;
        }

        v1 = a - c;
        v2 = p - c;

        dot[2] = v1.crossProduct(v2);
        zeroDot[2] = Math::RealEqual(dot[2], 0.0f, 1e-3);
        // Compare signs (ignore colinear / coincident points)
        if((!zeroDot[0] && !zeroDot[2] 
            && Math::Sign(dot[0]) != Math::Sign(dot[2])) ||
            (!zeroDot[1] && !zeroDot[2] 
            && Math::Sign(dot[1]) != Math::Sign(dot[2])))
        {
            return false;
        }


        return true;
    }
    //-----------------------------------------------------------------------
    bool Math::pointInTri3D(const Vector3& p, const Vector3& a, 
        const Vector3& b, const Vector3& c, const Vector3& normal)
    {
        // Winding must be consistent from all edges for point to be inside
        Vector3 v1, v2;
        Real dot[3];
        bool zeroDot[3];

        v1 = b - a;
        v2 = p - a;

        // Note we don't care about normalisation here since sign is all we need
        // It means we don't have to worry about magnitude of cross products either
        dot[0] = v1.crossProduct(v2).dotProduct(normal);
        zeroDot[0] = Math::RealEqual(dot[0], 0.0f, 1e-3);


        v1 = c - b;
        v2 = p - b;

        dot[1] = v1.crossProduct(v2).dotProduct(normal);
        zeroDot[1] = Math::RealEqual(dot[1], 0.0f, 1e-3);

        // Compare signs (ignore colinear / coincident points)
        if(!zeroDot[0] && !zeroDot[1] 
            && Math::Sign(dot[0]) != Math::Sign(dot[1]))
        {
            return false;
        }

        v1 = a - c;
        v2 = p - c;

        dot[2] = v1.crossProduct(v2).dotProduct(normal);
        zeroDot[2] = Math::RealEqual(dot[2], 0.0f, 1e-3);
        // Compare signs (ignore colinear / coincident points)
        if((!zeroDot[0] && !zeroDot[2] 
            && Math::Sign(dot[0]) != Math::Sign(dot[2])) ||
            (!zeroDot[1] && !zeroDot[2] 
            && Math::Sign(dot[1]) != Math::Sign(dot[2])))
        {
            return false;
        }


        return true;
    }
    //-----------------------------------------------------------------------
    std::pair<bool, Real> Math::intersects(const Ray& ray, 
        const std::list<Plane>& planes, bool normalIsOutside)
    {
        std::vector<Plane> planesVec;
        planesVec.reserve(planes.size());
        for (auto plane : planes)
        {
            planesVec.push_back(plane);
        }
        return intersects(ray, planesVec, normalIsOutside);
    }
    //-----------------------------------------------------------------------
    std::pair<bool, Real> Math::intersects(const Ray& ray, 
        const std::vector<Plane>& planes, bool normalIsOutside)
    {
        bool allInside = true;
        std::pair<bool, Real> ret;
        std::pair<bool, Real> end;
        ret.first = false;
        ret.second = 0.0f;
        end.first = false;
        end.second = 0;

        // derive side
        // NB we don't pass directly since that would require Plane::Side in 
        // interface, which results in recursive includes since Math is so fundamental
        Plane::Side outside = normalIsOutside ? Plane::POSITIVE_SIDE : Plane::NEGATIVE_SIDE;
        for (auto& plane : planes)
        {
            // is origin outside?
            if (plane.getSide(ray.getOrigin()) == outside)
            {
                allInside = false;
                // Test single plane
                std::pair<bool, Real> planeRes = 
                    ray.intersects(plane);
                if (planeRes.first)
                {
                    // Ok, we intersected
                    ret.first = true;
                    // Use the most distant result since convex volume
                    ret.second = std::max(ret.second, planeRes.second);
                }
                else
                {
                    ret.first =false;
                    ret.second=0.0f;
                    return ret;
                }
            }
            else
            {
                std::pair<bool, Real> planeRes = 
                    ray.intersects(plane);
                if (planeRes.first)
                {
                    if( !end.first )
                    {
                        end.first = true;
                        end.second = planeRes.second;
                    }
                    else
                    {
                        end.second = std::min( planeRes.second, end.second );
                    }

                }

            }
        }

        if (allInside)
        {
            // Intersecting at 0 distance since inside the volume!
            ret.first = true;
            ret.second = 0.0f;
            return ret;
        }

        if( end.first )
        {
            if( end.second < ret.second )
            {
                ret.first = false;
                return ret;
            }
        }
        return ret;
    }
    //-----------------------------------------------------------------------
    std::pair<bool, Real> Math::intersects(const Ray& ray, const AxisAlignedBox& box)
    {
        if (box.isNull()) return std::pair<bool, Real>(false, (Real)0);
        if (box.isInfinite()) return std::pair<bool, Real>(true, (Real)0);

        Real lowt = 0.0f;
        Real t;
        bool hit = false;
        Vector3 hitpoint;
        const Vector3& min = box.getMinimum();
        const Vector3& max = box.getMaximum();
        const Vector3& rayorig = ray.getOrigin();
        const Vector3& raydir = ray.getDirection();

        // Check origin inside first
        if ( rayorig > min && rayorig < max )
        {
            return std::pair<bool, Real>(true, (Real)0);
        }

        // Check each face in turn, only check closest 3
        // Min x
        if (rayorig.x <= min.x && raydir.x > 0)
        {
            t = (min.x - rayorig.x) / raydir.x;

            // Substitute t back into ray and check bounds and dist
            hitpoint = rayorig + raydir * t;
            if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
                hitpoint.z >= min.z && hitpoint.z <= max.z &&
                (!hit || t < lowt))
            {
                hit = true;
                lowt = t;
            }
        }
        // Max x
        if (rayorig.x >= max.x && raydir.x < 0)
        {
            t = (max.x - rayorig.x) / raydir.x;

            // Substitute t back into ray and check bounds and dist
            hitpoint = rayorig + raydir * t;
            if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
                hitpoint.z >= min.z && hitpoint.z <= max.z &&
                (!hit || t < lowt))
            {
                hit = true;
                lowt = t;
            }
        }
        // Min y
        if (rayorig.y <= min.y && raydir.y > 0)
        {
            t = (min.y - rayorig.y) / raydir.y;

            // Substitute t back into ray and check bounds and dist
            hitpoint = rayorig + raydir * t;
            if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
                hitpoint.z >= min.z && hitpoint.z <= max.z &&
                (!hit || t < lowt))
            {
                hit = true;
                lowt = t;
            }
        }
        // Max y
        if (rayorig.y >= max.y && raydir.y < 0)
        {
            t = (max.y - rayorig.y) / raydir.y;

            // Substitute t back into ray and check bounds and dist
            hitpoint = rayorig + raydir * t;
            if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
                hitpoint.z >= min.z && hitpoint.z <= max.z &&
                (!hit || t < lowt))
            {
                hit = true;
                lowt = t;
            }
        }
        // Min z
        if (rayorig.z <= min.z && raydir.z > 0)
        {
            t = (min.z - rayorig.z) / raydir.z;

            // Substitute t back into ray and check bounds and dist
            hitpoint = rayorig + raydir * t;
            if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
                hitpoint.y >= min.y && hitpoint.y <= max.y &&
                (!hit || t < lowt))
            {
                hit = true;
                lowt = t;
            }
        }
        // Max z
        if (rayorig.z >= max.z && raydir.z < 0)
        {
            t = (max.z - rayorig.z) / raydir.z;

            // Substitute t back into ray and check bounds and dist
            hitpoint = rayorig + raydir * t;
            if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
                hitpoint.y >= min.y && hitpoint.y <= max.y &&
                (!hit || t < lowt))
            {
                hit = true;
                lowt = t;
            }
        }

        return std::pair<bool, Real>(hit, (Real)lowt);

    } 
    //-----------------------------------------------------------------------
    bool Math::intersects(const Ray& ray, const AxisAlignedBox& box,
        Real* d1, Real* d2)
    {
        if (box.isNull())
            return false;

        if (box.isInfinite())
        {
            if (d1) *d1 = 0;
            if (d2) *d2 = Math::POS_INFINITY;
            return true;
        }

        const Vector3& min = box.getMinimum();
        const Vector3& max = box.getMaximum();
        const Vector3& rayorig = ray.getOrigin();
        const Vector3& raydir = ray.getDirection();

        Vector3 absDir;
        absDir[0] = Math::Abs(raydir[0]);
        absDir[1] = Math::Abs(raydir[1]);
        absDir[2] = Math::Abs(raydir[2]);

        // Sort the axis, ensure check minimise floating error axis first
        int imax = 0, imid = 1, imin = 2;
        if (absDir[0] < absDir[2])
        {
            imax = 2;
            imin = 0;
        }
        if (absDir[1] < absDir[imin])
        {
            imid = imin;
            imin = 1;
        }
        else if (absDir[1] > absDir[imax])
        {
            imid = imax;
            imax = 1;
        }

        Real start = 0, end = Math::POS_INFINITY;

#define _CALC_AXIS(i)                                       \
    do {                                                    \
        Real denom = 1 / raydir[i];                         \
        Real newstart = (min[i] - rayorig[i]) * denom;      \
        Real newend = (max[i] - rayorig[i]) * denom;        \
        if (newstart > newend) std::swap(newstart, newend); \
        if (newstart > end || newend < start) return false; \
        if (newstart > start) start = newstart;             \
        if (newend < end) end = newend;                     \
    } while(0)

        // Check each axis in turn

        _CALC_AXIS(imax);

        if (absDir[imid] < std::numeric_limits<Real>::epsilon())
        {
            // Parallel with middle and minimise axis, check bounds only
            if (rayorig[imid] < min[imid] || rayorig[imid] > max[imid] ||
                rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
                return false;
        }
        else
        {
            _CALC_AXIS(imid);

            if (absDir[imin] < std::numeric_limits<Real>::epsilon())
            {
                // Parallel with minimise axis, check bounds only
                if (rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
                    return false;
            }
            else
            {
                _CALC_AXIS(imin);
            }
        }
#undef _CALC_AXIS

        if (d1) *d1 = start;
        if (d2) *d2 = end;

        return true;
    }
    //-----------------------------------------------------------------------
    std::pair<bool, Real> Math::intersects(const Ray& ray, const Vector3& a, const Vector3& b,
                                           const Vector3& c, bool positiveSide, bool negativeSide)
    {
        const Real EPSILON = 1e-6f;
        Vector3 E1 = b - a;
        Vector3 E2 = c - a;
        Vector3 P = ray.getDirection().crossProduct(E2);
        Real det = E1.dotProduct(P);

        // if determinant is near zero, ray lies in plane of triangle
        if((!positiveSide || det <= EPSILON) && (!negativeSide || det >= -EPSILON))
            return {false, (Real)0};
        Real inv_det = 1.0f / det;

        // calculate u parameter and test bounds
        Vector3 T = ray.getOrigin() - a;
        Real u = T.dotProduct(P) * inv_det;
        if(u < 0.0f || u > 1.0f)
            return {false, (Real)0};

        // calculate v parameter and test bounds
        Vector3 Q = T.crossProduct(E1);
        Real v = ray.getDirection().dotProduct(Q) * inv_det;
        if (v < 0.0f || u + v > 1.0f)
            return {false, (Real)0};

        // calculate t, ray intersects triangle
        Real t = E2.dotProduct(Q) * inv_det;
        if (t < 0.0f)
            return {false, (Real)0};

        return {true, t};
    }
    //-----------------------------------------------------------------------
    bool Math::intersects(const Sphere& sphere, const AxisAlignedBox& box)
    {
        if (box.isNull()) return false;
        if (box.isInfinite()) return true;

        // Use splitting planes
        const Vector3& center = sphere.getCenter();
        Real radius = sphere.getRadius();
        const Vector3& min = box.getMinimum();
        const Vector3& max = box.getMaximum();

        // Arvo's algorithm
        Real s, d = 0;
        for (int i = 0; i < 3; ++i)
        {
            if (center.ptr()[i] < min.ptr()[i])
            {
                s = center.ptr()[i] - min.ptr()[i];
                d += s * s; 
            }
            else if(center.ptr()[i] > max.ptr()[i])
            {
                s = center.ptr()[i] - max.ptr()[i];
                d += s * s; 
            }
        }
        return d <= radius * radius;

    }
    //-----------------------------------------------------------------------
    Vector3 Math::calculateTangentSpaceVector(
        const Vector3& position1, const Vector3& position2, const Vector3& position3,
        Real u1, Real v1, Real u2, Real v2, Real u3, Real v3)
    {
        //side0 is the vector along one side of the triangle of vertices passed in, 
        //and side1 is the vector along another side. Taking the cross product of these returns the normal.
        Vector3 side0 = position1 - position2;
        Vector3 side1 = position3 - position1;
        //Calculate face normal
        Vector3 normal = side1.crossProduct(side0);
        normal.normalise();
        //Now we use a formula to calculate the tangent. 
        Real deltaV0 = v1 - v2;
        Real deltaV1 = v3 - v1;
        Vector3 tangent = deltaV1 * side0 - deltaV0 * side1;
        tangent.normalise();
        //Calculate binormal
        Real deltaU0 = u1 - u2;
        Real deltaU1 = u3 - u1;
        Vector3 binormal = deltaU1 * side0 - deltaU0 * side1;
        binormal.normalise();
        //Now, we take the cross product of the tangents to get a vector which 
        //should point in the same direction as our normal calculated above. 
        //If it points in the opposite direction (the dot product between the normals is less than zero), 
        //then we need to reverse the s and t tangents. 
        //This is because the triangle has been mirrored when going from tangent space to object space.
        //reverse tangents if necessary
        Vector3 tangentCross = tangent.crossProduct(binormal);
        if (tangentCross.dotProduct(normal) < 0.0f)
        {
            tangent = -tangent;
            binormal = -binormal;
        }

        return tangent;

    }
    //-----------------------------------------------------------------------
    Affine3 Math::buildReflectionMatrix(const Plane& p)
    {
        return Affine3(
            -2 * p.normal.x * p.normal.x + 1,   -2 * p.normal.x * p.normal.y,       -2 * p.normal.x * p.normal.z,       -2 * p.normal.x * p.d, 
            -2 * p.normal.y * p.normal.x,       -2 * p.normal.y * p.normal.y + 1,   -2 * p.normal.y * p.normal.z,       -2 * p.normal.y * p.d, 
            -2 * p.normal.z * p.normal.x,       -2 * p.normal.z * p.normal.y,       -2 * p.normal.z * p.normal.z + 1,   -2 * p.normal.z * p.d);
    }
    //-----------------------------------------------------------------------
    Real Math::gaussianDistribution(Real x, Real offset, Real scale)
    {
        Real nom = Math::Exp(
            -Math::Sqr(x - offset) / (2 * Math::Sqr(scale)));
        Real denom = scale * Math::Sqrt(2 * Math::PI);

        return nom / denom;

    }
    //---------------------------------------------------------------------
    Affine3 Math::makeViewMatrix(const Vector3& position, const Quaternion& orientation,
        const Affine3* reflectMatrix)
    {
        // This is most efficiently done using 3x3 Matrices
        Matrix3 rot;
        orientation.ToRotationMatrix(rot);

        // Make the translation relative to new axes
        Matrix3 rotT = rot.Transpose();
        Vector3 trans = -rotT * position;

        // Make final matrix
        Affine3 viewMatrix = Affine3::IDENTITY;
        viewMatrix = rotT; // fills upper 3x3
        viewMatrix[0][3] = trans.x;
        viewMatrix[1][3] = trans.y;
        viewMatrix[2][3] = trans.z;

        // Deal with reflections
        if (reflectMatrix)
        {
            viewMatrix = viewMatrix * (*reflectMatrix);
        }

        return viewMatrix;

    }

    Matrix4 Math::makePerspectiveMatrix(Real left, Real right, Real bottom, Real top, Real zNear, Real zFar)
    {
        // The code below will dealing with general projection
        // parameters, similar glFrustum.
        // Doesn't optimise manually except division operator, so the
        // code more self-explaining.

        Real inv_w = 1 / (right - left);
        Real inv_h = 1 / (top - bottom);
        Real inv_d = 1 / (zFar - zNear);

        // Calc matrix elements
        Real A = 2 * zNear * inv_w;
        Real B = 2 * zNear * inv_h;
        Real C = (right + left) * inv_w;
        Real D = (top + bottom) * inv_h;
        Real q, qn;

        if (zFar == 0)
        {
            // Infinite far plane
            q = Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
            qn = zNear * (Frustum::INFINITE_FAR_PLANE_ADJUST - 2);
        }
        else
        {
            q = - (zFar + zNear) * inv_d;
            qn = -2 * (zFar * zNear) * inv_d;
        }

        Matrix4 ret = Matrix4::ZERO;
        ret[0][0] = A;
        ret[0][2] = C;
        ret[1][1] = B;
        ret[1][2] = D;
        ret[2][2] = q;
        ret[2][3] = qn;
        ret[3][2] = -1;

        return ret;
    }
    //---------------------------------------------------------------------
    Real Math::boundingRadiusFromAABB(const AxisAlignedBox& aabb)
    {
        const Vector3& max = aabb.getMaximum();
        const Vector3& min = aabb.getMinimum();

        Vector3 magnitude = max;
        magnitude.makeCeil(-max);
        magnitude.makeCeil(min);
        magnitude.makeCeil(-min);

        return magnitude.length();
    }

    Real Math::boundingRadiusFromAABBCentered(const AxisAlignedBox& aabb)
    {
        const Vector3& max = aabb.getMaximum();
        const Vector3& min = aabb.getMinimum();

        return ((min - max) * 0.5f).length();
    }
}
