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
#ifndef __Ogre_Volume_TextureSource_H__
#define __Ogre_Volume_TextureSource_H__

#include "OgreVolumeSource.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    /** A volume source from a 3D texture.
    */
    class _OgreVolumeExport TextureSource : public Source
    {
    protected:

        /// The raw volume data.
        float *mData;

        /// The texture width.
        int mWidth;

        /// The texture height.
        int mHeight;

        /// To have a little bit faster data access.
        int mWidthTimesHeight;

        /// The scale of the position based on the world width.
        Real mPosXScale;
        
        /// The scale of the position based on the world height.
        Real mPosYScale;
        
        /// The scale of the position based on the world depth.
        Real mPosZScale;

        /// The texture depth.
        int mDepth;

        // Whether to use trilinear filtering or not for the value.
        const bool mTrilinearValue;
        
        // Whether to use trilinear filtering or not for the normal.
        const bool mTrilinearNormal;

        /// Whether to blur the normal a bit Sobel like.
        const bool mSobelNormal;

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
        inline float getVolumeArrayValue(int x, int y, int z) const
        {
            if (x >= mWidth)
            {
                x = mWidth - 1;
            }
            else if (x < 0)
            {
                x = 0;
            }

            if (y >= mHeight)
            {
                y = mHeight - 1;
            } else if (y < 0)
            {
                y = 0;
            }

            if (z >= mDepth)
            {
                z = mDepth - 1;
            } else if (z < 0)
            {
                z = 0;
            }

            return mData[(mDepth - z - 1) * mWidthTimesHeight + y * mWidth + x];
        }

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
            if (mSobelNormal)
            {
                // Calculate gradient like in the original MC paper but mix a bit of Sobel in
                return Vector3(
                (getVolumeArrayValue(x + 1, y - 1, z) - getVolumeArrayValue(x - 1, y - 1, z))
                        + (Real)2.0 * (getVolumeArrayValue(x + 1, y, z) - getVolumeArrayValue(x - 1, y, z))
                        + (getVolumeArrayValue(x + 1, y + 1, z) - getVolumeArrayValue(x - 1, y + 1, z)),
                (getVolumeArrayValue(x, y + 1, z - 1) - getVolumeArrayValue(x, y - 1, z - 1))
                    + (Real)2.0 * (getVolumeArrayValue(x, y + 1, z) - getVolumeArrayValue(x, y - 1, z))
                    + (getVolumeArrayValue(x, y + 1, z + 1) - getVolumeArrayValue(x, y - 1, z + 1)),
                (getVolumeArrayValue(x - 1, y, z + 1) - getVolumeArrayValue(x - 1, y, z - 1))
                    + (Real)2.0 * (getVolumeArrayValue(x, y, z + 1) - getVolumeArrayValue(x, y, z - 1))
                    + (getVolumeArrayValue(x + 1, y, z + 1) - getVolumeArrayValue(x + 1, y, z - 1))) / (Real)4.0;
            }
            // Calculate gradient like in the original MC paper
            return Vector3(
                getVolumeArrayValue(x + 1, y, z) - getVolumeArrayValue(x - 1, y, z),
                getVolumeArrayValue(x, y + 1, z) - getVolumeArrayValue(x, y - 1, z),
                getVolumeArrayValue(x, y, z + 1) - getVolumeArrayValue(x, y, z - 1));
        }

    public:

        /** Constructur.
        @param volumeTextureName
            Which volume texture to get the data from.
        @param worldWidth
            The world width.
        @param worldHeight
            The world height.
        @param worldDepth
            The world depth.
        @param trilinearValue
            Whether to use trilinear filtering (true) or nearest neighbour (false) for the value.
        @param trilinearNormal
            Whether to use trilinear filtering (true) or nearest neighbour (false) for the normal.
        @param sobelNormal
            Whether to add a bit of blur to the normal like in a sobel filter.
        */
        explicit TextureSource(const String &volumeTextureName, const Real worldWidth, const Real worldHeight, const Real worldDepth, const bool trilinearValue = true, const bool trilinearNormal = true, const bool sobelNormal = false);
        
        /** Destructor.
        */
        ~TextureSource(void);

        /** Overridden from VolumeSource.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;
        
        /** Overridden from VolumeSource.
        */
        virtual Real getValue(const Vector3 &position) const;

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
    };

}
}

#endif