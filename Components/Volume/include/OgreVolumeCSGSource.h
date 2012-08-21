/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __Ogre_Volume_CSGSource_H__
#define __Ogre_Volume_CSGSource_H__

#include "OgreVolumeSource.h"
#include "OgreVector3.h"
#include "OgreAxisAlignedBox.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    /** A sphere.
    */
    class _OgreVolumeExport CSGSphereSource : public Source
    {
    protected:
        /** The radius.
        */
        const Real mR;
        
        /** The center.
        */
        const Vector3 mCenter;
    public:
    
        /** Constructor.
        @param r
            The radius.
        @param center
            The center.
        */
        CSGSphereSource(const Real r, const Vector3 &center);
        
        /** Overridden from Source.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;

        /** Overridden from VolumeSource.
        */
        virtual Real getValue(const Vector3 &position) const;
    };

    /** A plane.
    */
    class _OgreVolumeExport CSGPlaneSource : public Source
    {
    protected:
        
        /// The distance to zero of the plane.
        const Real mD;
        
        /// The normal of the plane.
        Vector3 mNormal;

    public:
        
        /** Constructor.
        @param d
            The distance to zero of the plane.
        @param normal
            The planes normal.
        */
        CSGPlaneSource(const Real d, const Vector3 &normal);
        
        /** Overridden from Source.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;

        /** Overridden from VolumeSource.
        */
        virtual Real getValue(const Vector3 &position) const;
    };

    /** A not rotated cube.
    */
    class _OgreVolumeExport CSGCubeSource : public Source
    {
    protected:
    
        /// The six possible normals of the cube.
        static Vector3 mBoxNormals[6];
        
        /// The box.
        AxisAlignedBox mBox;

        inline Real distanceTo(const Vector3 &position) const
        {
            Real distance;
            const Vector3 dMin = position - mBox.getMinimum();
            const Vector3 dMax = mBox.getMaximum() - position;

            // Check if inside of the box
            if (dMin.x >= (Real)0.0 && dMin.y >= (Real)0.0 && dMin.z >= (Real)0.0 &&
                dMax.x >= (Real)0.0 && dMax.y >= (Real)0.0 && dMax.z >= (Real)0.0)
            {
                const Real d[6] = {dMin.x, dMin.y, dMin.z, dMax.x, dMax.y, dMax.z};
                distance = Math::POS_INFINITY;
                for (size_t i = 0; i < 6; ++i)
                {
                    if (d[i] < distance)
                    {
                        distance = d[i];
                    }
                }
            }
            else
            {
                distance = -mBox.distance(position);
            }
            return distance;
        }

    public:
    
        /** Constructor.
        @param min
            The lower back left corner.
        @param max
            The upper front right corner.
        */
        CSGCubeSource(const Vector3 &min, const Vector3 &max);
        
        /** Overridden from Source.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;

        /** Overridden from VolumeSource.
        */
        virtual Real getValue(const Vector3 &position) const;
    };

    /** Abstract operation volume source holding two sources as operants.
    */
    class _OgreVolumeExport CSGOperationSource : public Source
    {
    protected:
    
        /// The first operant.
        const Source *mA;
        
        /// The second operant.
        const Source *mB;
        
        /** Constructor. Protected to be callable from child classes.
        @param a
            The first operator.
        @param b
            The second operator.
        */
        CSGOperationSource(const Source *a, const Source *b);
    };

    /** Builds the intersection between two sources.
    */
    class _OgreVolumeExport CSGIntersectionSource : public CSGOperationSource
    {
    public:
    
        /** Constructor.
        @param a
            The first operator.
        @param b
            The second operator.
        */
        CSGIntersectionSource(const Source *a, const Source *b);
        
        /** Overridden from Source.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;

        /** Overridden from VolumeSource.
        */
        virtual Real getValue(const Vector3 &position) const;
    };

    /** Builds the union between two sources.
    */
    class _OgreVolumeExport CSGUnionSource : public CSGOperationSource
    {
    public:
    
        /** Constructor.
        @param a
            The first operator.
        @param b
            The second operator.
        */
        CSGUnionSource(const Source *a, const Source *b);
        
        /** Overridden from Source.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;

        /** Overridden from VolumeSource.
        */
        virtual Real getValue(const Vector3 &position) const;
    };

    /** Builds the difference between two sources.
    */
    class _OgreVolumeExport CSGDifferenceSource : public CSGOperationSource
    {
    public:
    
    
        /** Constructor.
        @param a
            The first operator.
        @param b
            The second operator.
        */
        CSGDifferenceSource(const Source *a, const Source *b);
        
        /** Overridden from Source.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;

        /** Overridden from VolumeSource.
        */
        virtual Real getValue(const Vector3 &position) const;
    };

    /** Source which does a unary operation to another one.
    */
    class _OgreVolumeExport CSGUnarySource : public Source
    {
    protected:
    
        /// The first operant.
        const Source *mSrc;
        
        /** Constructor. Protected to be callable from child classes.
        @param src
            The operator.
        */
        CSGUnarySource(const Source *src);
    };

    /** Negates the given volume.
    */
    class _OgreVolumeExport CSGNegateSource : public CSGUnarySource
    {
    public:
    
        /** Constructor.
        @param src
            The source to negate.
        */
        explicit CSGNegateSource(const Source *src);
        
        /** Overridden from Source.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;

        /** Overridden from VolumeSource.
        */
        virtual Real getValue(const Vector3 &position) const;
    };

    /** Scales the given volume source.
    */
    class _OgreVolumeExport CSGScaleSource : public CSGUnarySource
    {
    protected:
        
        /// Holds the dimensions of the volume.
        Real mScale;
    public:

        /** Constructor.
        @param src
            The source to scale.
        @param scale
            The scale of the source.
        */
        CSGScaleSource(const Source *src, const Real scale);
        
        /** Overridden from Source.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;

        /** Overridden from VolumeSource.
        */
        virtual Real getValue(const Vector3 &position) const;
    };

}
}

#endif