/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd
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
#include "OgreVolumeCSGSource.h"
#include <algorithm>

namespace Ogre {
namespace Volume {

    Vector3 CSGCubeSource::mBoxNormals[6] = {
        Vector3::UNIT_X,
        Vector3::UNIT_Y,
        Vector3::UNIT_Z,
        Vector3::NEGATIVE_UNIT_X,
        Vector3::NEGATIVE_UNIT_Y,
        Vector3::NEGATIVE_UNIT_Z
    };
    
    //-----------------------------------------------------------------------

    CSGSphereSource::CSGSphereSource(const Real r, const Vector3 &center) : mR(r), mCenter(center)
    {
    }
    
    //-----------------------------------------------------------------------

    Real CSGSphereSource::getValueAndGradient(const Vector3 &position, Vector3 &gradient) const
    {
        Vector3 pMinCenter = position- mCenter;
        gradient = pMinCenter;
        return mR - gradient.normalise();
    }
    
    //-----------------------------------------------------------------------

    Real CSGSphereSource::getValue(const Vector3 &position) const
    {
        Vector3 pMinCenter = position - mCenter;
        return mR - pMinCenter.length();
    }
    
    //-----------------------------------------------------------------------

    CSGPlaneSource::CSGPlaneSource(const Real d, const Vector3 &normal) : mD(d)
    {
        mNormal = normal.normalisedCopy();
    }
    
    //-----------------------------------------------------------------------

    Real CSGPlaneSource::getValueAndGradient(const Vector3 &position, Vector3 &gradient) const
    {
        gradient = mNormal;
        // Lineare Algebra: Ein geometrischer Zugang, S.180-181
        return mD - mNormal.dotProduct(position);
    }
    
    //-----------------------------------------------------------------------

    Real CSGPlaneSource::getValue(const Vector3 &position) const
    {
        // Lineare Algebra: Ein geometrischer Zugang, S.180-181
        return mD - mNormal.dotProduct(position);
    }
    
    //-----------------------------------------------------------------------

    CSGCubeSource::CSGCubeSource(const Vector3 &min, const Vector3 &max)
    {
        mBox.setExtents(min, max);
    }
    
    //-----------------------------------------------------------------------

    Real CSGCubeSource::getValueAndGradient(const Vector3 &position, Vector3 &gradient) const
    { 
        // Just approximate the normal via a simple Prewitt. To get the real normal, 26 cases had to be evaluated...
        gradient.x = getValue(Vector3(position.x + (Real)1.0, position.y, position.z)) - getValue(Vector3(position.x - (Real)1.0, position.y, position.z));
        gradient.y = getValue(Vector3(position.x, position.y + (Real)1.0, position.z)) - getValue(Vector3(position.x, position.y - (Real)1.0, position.z));
        gradient.z = getValue(Vector3(position.x, position.y, position.z + (Real)1.0)) - getValue(Vector3(position.x, position.y, position.z - (Real)1.0));
        gradient.normalise();
        gradient *= (Real)-1.0;
        return distanceTo(position);
    }
    
    //-----------------------------------------------------------------------

    Real CSGCubeSource::getValue(const Vector3 &position) const
    {
        return distanceTo(position);
    }
    
    //-----------------------------------------------------------------------

    CSGOperationSource::CSGOperationSource(const Source *a, const Source *b) : mA(a), mB(b)
    {
    }
    
    //-----------------------------------------------------------------------

    CSGIntersectionSource::CSGIntersectionSource(const Source *a, const Source *b) : CSGOperationSource(a, b)
    {
    }
    
    //-----------------------------------------------------------------------

    Real CSGIntersectionSource::getValueAndGradient(const Vector3 &position, Vector3 &gradient) const
    {
        Vector3 gradientA;
        Vector3 gradientB;
        Real valueA = mA->getValueAndGradient(position, gradientA);
        Real valueB = mB->getValueAndGradient(position, gradientB);
        if (valueA < valueB)
        {
            gradient = gradientA;
            return valueA;
        }
        gradient = gradientB;
        return valueB;
    }
    
    //-----------------------------------------------------------------------

    Real CSGIntersectionSource::getValue(const Vector3 &position) const
    {
        Real valueA = mA->getValue(position);
        Real valueB = mB->getValue(position);
        if (valueA < valueB)
        {
            return valueA;
        }
        return valueB;
    }
    
    //-----------------------------------------------------------------------

    CSGUnionSource::CSGUnionSource(const Source *a, const Source *b) : CSGOperationSource(a, b)
    {
    }
    
    //-----------------------------------------------------------------------

    Real CSGUnionSource::getValueAndGradient(const Vector3 &position, Vector3 &gradient) const
    {
        Vector3 gradientA;
        Vector3 gradientB;
        Real valueA = mA->getValueAndGradient(position, gradientA);
        Real valueB = mB->getValueAndGradient(position, gradientB);
        if (valueA > valueB)
        {
            gradient = gradientA;
            return valueA;
        }
        gradient = gradientB;
        return valueB;
    }
    
    //-----------------------------------------------------------------------

    Real CSGUnionSource::getValue(const Vector3 &position) const
    {
        Real valueA = mA->getValue(position);
        Real valueB = mB->getValue(position);
        if (valueA > valueB)
        {
            return valueA;
        }
        return valueB;
    }
    
    //-----------------------------------------------------------------------

    CSGDifferenceSource::CSGDifferenceSource(const Source *a, const Source *b) : CSGOperationSource(a, b)
    {
    }
    
    //-----------------------------------------------------------------------

    Real CSGDifferenceSource::getValueAndGradient(const Vector3 &position, Vector3 &gradient) const
    {
        // Like in the isosurfe project
        Vector3 gradientA;
        Vector3 gradientB;
        Real valueA = mA->getValueAndGradient(position, gradientA);
        Real valueB = (Real)-1.0 * mB->getValueAndGradient(position, gradientB);
        gradientB *= (Real)-1.0;
        if (valueA < valueB)
        {
            gradient = gradientA;
            return valueA;
        }
        gradient = gradientB;
        return valueB;
    }
    
    //-----------------------------------------------------------------------

    Real CSGDifferenceSource::getValue(const Vector3 &position) const
    {
        // Like in the isosurfe project
        Real valueA = mA->getValue(position);
        Real valueB = (Real)-1.0 * mB->getValue(position);
        if (valueA < valueB)
        {
            return valueA;
        }
        return valueB;
    }
    
    //-----------------------------------------------------------------------

    CSGUnarySource::CSGUnarySource(const Source *src) : mSrc(src)
    {
    }
    
    //-----------------------------------------------------------------------

    CSGNegateSource::CSGNegateSource(const Source *src) : CSGUnarySource(src)
    {
    }
    
    //-----------------------------------------------------------------------

    Real CSGNegateSource::getValueAndGradient(const Vector3 &position, Vector3 &gradient) const
    {
        Real result = (Real)-1.0 * mSrc->getValueAndGradient(position, gradient);
        gradient *= (Real)-1.0;
        return result;
    }
    
    //-----------------------------------------------------------------------

    Real CSGNegateSource::getValue(const Vector3 &position) const
    {
        return (Real)-1.0 * mSrc->getValue(position);
    }
    
    //-----------------------------------------------------------------------

    CSGScaleSource::CSGScaleSource(const Source *src, const Real scale) : CSGUnarySource(src), mScale(scale)
    {
    }
    
    //-----------------------------------------------------------------------

    Real CSGScaleSource::getValueAndGradient(const Vector3 &position, Vector3 &gradient) const
    {
        Real density = mSrc->getValueAndGradient(position / mScale, gradient) * mScale;
        gradient *= mScale;
        return density;
    }
    
    //-----------------------------------------------------------------------

    Real CSGScaleSource::getValue(const Vector3 &position) const
    {
        return mSrc->getValue(position / mScale) * mScale;
    }

}
}