/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

    Vector4 CSGSphereSource::getValueAndGradient(const Vector3 &position) const
    {
        Vector3 pMinCenter = position- mCenter;
        Vector3 gradient = pMinCenter;
        return Vector4(
            gradient.x,
            gradient.y,
            gradient.z,
            mR - gradient.normalise()
            );
    }
    
    //-----------------------------------------------------------------------

    Real CSGSphereSource::getValue(const Vector3 &position) const
    {
        Vector3 pMinCenter = position - mCenter;
        return mR - pMinCenter.length();
    }
    
    //-----------------------------------------------------------------------

    CSGPlaneSource::CSGPlaneSource(const Real d, const Vector3 &normal) : mD(d), mNormal(normal.normalisedCopy())
    {
    }
    
    //-----------------------------------------------------------------------

    Vector4 CSGPlaneSource::getValueAndGradient(const Vector3 &position) const
    {
        // Lineare Algebra: Ein geometrischer Zugang, S.180-181
        return Vector4(
            mNormal.x,
            mNormal.y,
            mNormal.z,
            mD - mNormal.dotProduct(position)
            );
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

    Vector4 CSGCubeSource::getValueAndGradient(const Vector3 &position) const
    { 
        // Just approximate the normal via a simple Prewitt. To get the real normal, 26 cases had to be evaluated...
        Vector3 gradient(getValue(Vector3(position.x + (Real)1.0, position.y, position.z)) - getValue(Vector3(position.x - (Real)1.0, position.y, position.z)),
            getValue(Vector3(position.x, position.y + (Real)1.0, position.z)) - getValue(Vector3(position.x, position.y - (Real)1.0, position.z)),
            getValue(Vector3(position.x, position.y, position.z + (Real)1.0)) - getValue(Vector3(position.x, position.y, position.z - (Real)1.0)));
        gradient.normalise();
        gradient *= (Real)-1.0;
        return Vector4(
            gradient.x,
            gradient.y,
            gradient.z,
            distanceTo(position));
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

    CSGOperationSource::CSGOperationSource(void) : mA(0), mB(0)
    {
    }
    
    //-----------------------------------------------------------------------

    const Source* CSGOperationSource::getSourceA(void) const
    {
        return mA;
    }

    //-----------------------------------------------------------------------

    void CSGOperationSource::setSourceA(Source *a)
    {
        mA = a;
    }

    //-----------------------------------------------------------------------

    const Source* CSGOperationSource::getSourceB(void) const
    {
        return mB;
    }

    //-----------------------------------------------------------------------

    void CSGOperationSource::setSourceB(Source *b)
    {
        mB = b;
    }

    //-----------------------------------------------------------------------

    CSGIntersectionSource::CSGIntersectionSource(const Source *a, const Source *b) : CSGOperationSource(a, b)
    {
    }
    
    //-----------------------------------------------------------------------

    CSGIntersectionSource::CSGIntersectionSource(void) : CSGOperationSource()
    {
    }
    
    //-----------------------------------------------------------------------

    Vector4 CSGIntersectionSource::getValueAndGradient(const Vector3 &position) const
    {
        Vector4 valueA = mA->getValueAndGradient(position);
        Vector4 valueB = mB->getValueAndGradient(position);
        if (valueA.w < valueB.w)
        {
            return valueA;
        }
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

    CSGUnionSource::CSGUnionSource(void) : CSGOperationSource()
    {
    }
    
    //-----------------------------------------------------------------------

    Vector4 CSGUnionSource::getValueAndGradient(const Vector3 &position) const
    {
        Vector4 valueA = mA->getValueAndGradient(position);
        Vector4 valueB = mB->getValueAndGradient(position);
        if (valueA.w > valueB.w)
        {
            return valueA;
        }
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

    CSGDifferenceSource::CSGDifferenceSource(void) : CSGOperationSource()
    {
    }
    
    //-----------------------------------------------------------------------

    Vector4 CSGDifferenceSource::getValueAndGradient(const Vector3 &position) const
    {
        Vector4 valueA = mA->getValueAndGradient(position);
        Vector4 valueB = (Real)-1.0 * mB->getValueAndGradient(position);
        if (valueA.w < valueB.w)
        {
            return valueA;
        }
        return valueB;
    }
    
    //-----------------------------------------------------------------------

    Real CSGDifferenceSource::getValue(const Vector3 &position) const
    {
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

    CSGUnarySource::CSGUnarySource(void) : mSrc(0)
    {
    }
    
    //-----------------------------------------------------------------------

    const Source* CSGUnarySource::getSource(void) const
    {
        return mSrc;
    }
    
    //-----------------------------------------------------------------------

    void CSGUnarySource::setSource(Source *a)
    {
        mSrc = a;
    }
    
    //-----------------------------------------------------------------------

    CSGNegateSource::CSGNegateSource(const Source *src) : CSGUnarySource(src)
    {
    }
    
    //-----------------------------------------------------------------------

    CSGNegateSource::CSGNegateSource(void) : CSGUnarySource()
    {
    }
    
    //-----------------------------------------------------------------------

    Vector4 CSGNegateSource::getValueAndGradient(const Vector3 &position) const
    {
        return (Real)-1.0 * mSrc->getValueAndGradient(position);
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

    Vector4 CSGScaleSource::getValueAndGradient(const Vector3 &position) const
    {
        return mSrc->getValueAndGradient(position / mScale) * mScale;
    }
    
    //-----------------------------------------------------------------------

    Real CSGScaleSource::getValue(const Vector3 &position) const
    {
        return mSrc->getValue(position / mScale) * mScale;
    }
    
    //-----------------------------------------------------------------------

    void CSGNoiseSource::setData(void)
    {
        mGradientOff = fabs(mFrequencies[0]);
        for (size_t i = 1; i < mNumOctaves; ++i)
        {
            if (fabs(mFrequencies[i]) < mGradientOff)
            {
                mGradientOff = mFrequencies[i];
            }
        }
        mGradientOff /= (Real)4.0;
        mSeed = mNoise.getSeed();
    }
    
    //-----------------------------------------------------------------------

    CSGNoiseSource::CSGNoiseSource(const Source *src, Real *frequencies, Real *amplitudes, size_t numOctaves, long seed) :
        CSGUnarySource(src), mFrequencies(frequencies), mAmplitudes(amplitudes), mNumOctaves(numOctaves), mNoise(seed)
    {
        setData();
    }

    //-----------------------------------------------------------------------

    CSGNoiseSource::CSGNoiseSource(const Source *src, Real *frequencies, Real *amplitudes, size_t numOctaves) :
        CSGUnarySource(src), mFrequencies(frequencies), mAmplitudes(amplitudes), mNumOctaves(numOctaves)
    {
        setData();
    }

    //-----------------------------------------------------------------------

    Vector4 CSGNoiseSource::getValueAndGradient(const Vector3 &position) const
    {
        return Vector4(
            -(getInternalValue(Vector3(position.x + mGradientOff, position.y, position.z)) - getInternalValue(Vector3(position.x - mGradientOff, position.y, position.z))),
            -(getInternalValue(Vector3(position.x, position.y + mGradientOff, position.z)) - getInternalValue(Vector3(position.x, position.y - mGradientOff, position.z))),
            -(getInternalValue(Vector3(position.x, position.y, position.z + mGradientOff)) - getInternalValue(Vector3(position.x, position.y, position.z - mGradientOff))),
            getInternalValue(position));
    }
    
    //-----------------------------------------------------------------------

    Real CSGNoiseSource::getValue(const Vector3 &position) const
    {
        return getInternalValue(position);
    }
    
    //-----------------------------------------------------------------------

    long CSGNoiseSource::getSeed(void) const
    {
        return mSeed;
    }
}
}