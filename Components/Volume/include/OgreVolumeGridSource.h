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
#ifndef __Ogre_Volume_GridSource_H__
#define __Ogre_Volume_GridSource_H__

#include "OgreVector.h"

#include "OgreVolumePrerequisites.h"
#include "OgreVolumeSource.h"

namespace Ogre {
namespace Volume {
    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Volume
    *  @{
    */
    class CSGOperationSource;

    /** A volume source from a discrete 3d grid.
    */
    class _OgreVolumeExport GridSource : public Source
    {
    protected:

        /// The texture width.
        size_t mWidth;

        /// The texture height.
        size_t mHeight;
        
        /// The texture depth.
        size_t mDepth;
        
        /// The scale of the position based on the world width.
        Real mPosXScale;
        
        /// The scale of the position based on the world height.
        Real mPosYScale;
        
        /// The scale of the position based on the world depth.
        Real mPosZScale;

        /// Whether to use trilinear filtering or not for the value.
        bool mTrilinearValue;
        
        /// Whether to use trilinear filtering or not for the gradient.
        const bool mTrilinearGradient;

        /// Whether to blur the gradient a bit Sobel like.
        const bool mSobelGradient;

        /// Factor to come from volume coordinate to world coordinate.
        Real mVolumeSpaceToWorldSpaceFactor;
        
        /** Overridden from VolumeSource.
        */
        Vector3 getIntersectionStart(const Ray &ray, Real maxDistance) const override;
        
        /** Overridden from VolumeSource.
        */
        Vector3 getIntersectionEnd(const Ray &ray, Real maxDistance) const override;

        /** Gets the volume value of a position.
        @param x
            The x position.
        @param y
            The y position.
        @param z
            The z position.
        @return
            The density.
        */
        virtual float getVolumeGridValue(size_t x, size_t y, size_t z) const = 0;
        
        /** Sets the volume value of a position.
        @param x
            The x position.
        @param y
            The y position.
        @param z
            The z position.
        @param value
            The density to be set.
        */
        virtual void setVolumeGridValue(int x, int y, int z, float value) = 0;

        /** Gets a gradient of a point with optional sobel blurring.
        @param x
            The x coordinate of the point.
        @param y
            The x coordinate of the point.
        @param z
            The x coordinate of the point.
        */
        inline const Vector3 getGradient(size_t x, size_t y, size_t z) const
        {
            if (mSobelGradient)
            {
                // Calculate gradient like in the original MC paper but mix a bit of Sobel in
                return Vector3(
                (getVolumeGridValue(x + 1, y - 1, z) - getVolumeGridValue(x - 1, y - 1, z))
                        + (Real)2.0 * (getVolumeGridValue(x + 1, y, z) - getVolumeGridValue(x - 1, y, z))
                        + (getVolumeGridValue(x + 1, y + 1, z) - getVolumeGridValue(x - 1, y + 1, z)),
                (getVolumeGridValue(x, y + 1, z - 1) - getVolumeGridValue(x, y - 1, z - 1))
                    + (Real)2.0 * (getVolumeGridValue(x, y + 1, z) - getVolumeGridValue(x, y - 1, z))
                    + (getVolumeGridValue(x, y + 1, z + 1) - getVolumeGridValue(x, y - 1, z + 1)),
                (getVolumeGridValue(x - 1, y, z + 1) - getVolumeGridValue(x - 1, y, z - 1))
                    + (Real)2.0 * (getVolumeGridValue(x, y, z + 1) - getVolumeGridValue(x, y, z - 1))
                    + (getVolumeGridValue(x + 1, y, z + 1) - getVolumeGridValue(x + 1, y, z - 1))) / (Real)4.0;
            }
            // Calculate gradient like in the original MC paper
            return Vector3(
                getVolumeGridValue(x + 1, y, z) - getVolumeGridValue(x - 1, y, z),
                getVolumeGridValue(x, y + 1, z) - getVolumeGridValue(x, y - 1, z),
                getVolumeGridValue(x, y, z + 1) - getVolumeGridValue(x, y, z - 1));
        }

    public:

        GridSource(bool trilinearValue, bool trilinearGradient, bool sobelGradient);

        /** Destructor.
        */
        virtual ~GridSource(void);

        /** Overridden from VolumeSource.
        */
        Vector4 getValueAndGradient(const Vector3 &position) const override;
        
        /** Overridden from VolumeSource.
        */
        Real getValue(const Vector3 &position) const override;

        /** Gets the width of the texture.
        @return
            The width of the texture.
        */
        size_t getWidth(void) const;
        
        /** Gets the height of the texture.
        @return
            The height of the texture.
        */
        size_t getHeight(void) const;
        
        /** Gets the depth of the texture.
        @return
            The depth of the texture.
        */
        size_t getDepth(void) const;

        /** Updates this grid with another source in a certain area. Use
        it for example to add spheres as a brush.
        @param operation
            The operation to use, will use this source and the other given one as operands. Beware that
            this function overrides the maybe existing sources in the operation.
        @param source
            The other source to combine this one with.
        @param center
            The rough center of the affected area by the operation. If the other source is a sphere, take
            its center for example.
        @param radius
            The radius of the affected area. For the example sphere, you might use its radius times two
            because the density outside of the sphere is needed, too.
        */
        virtual void combineWithSource(CSGOperationSource *operation, Source *source, const Vector3 &center, Real radius);
    
        
        /** Overridden from VolumeSource.
        */
        Real getVolumeSpaceToWorldSpaceFactor(void) const;

    };
    /** @} */
    /** @} */
}
}

#endif
