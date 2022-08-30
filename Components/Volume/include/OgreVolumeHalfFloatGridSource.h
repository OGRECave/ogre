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
#ifndef __Ogre_Volume_HalfFloatGridSource_H__
#define __Ogre_Volume_HalfFloatGridSource_H__


#include "OgreVolumeGridSource.h"

namespace Ogre {
namespace Volume {
    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Volume
    *  @{
    */
    /** A volume source from a 16 Bit float 3D grid capable of loading volume
    serializations.
    */
    class _OgreVolumeExport HalfFloatGridSource : public GridSource
    {
    protected:
        
        /// The raw volume data.
        uint16 *mData;
        
        /// To have a little bit faster data access.
        int mDepthTimesHeight;

        /// The maximum absolute density value to be written into the data when combining,
        /// influencing the compression rate on serialization.
        Real mMaxClampedAbsoluteDensity;
        
        /** Overridden from GridSource.
        */
        float getVolumeGridValue(size_t x, size_t y, size_t z) const override;

        /** Overridden from GridSource.
        */
        void setVolumeGridValue(int x, int y, int z, float value) override;

    public:

        /** Constructur.
        @param serializedVolumeFile
            Which volume serialization to get the data from.
        @param trilinearValue
            Whether to use trilinear filtering (true) or nearest neighbour (false) for the value.
        @param trilinearGradient
            Whether to use trilinear filtering (true) or nearest neighbour (false) for the gradient.
        @param sobelGradient
            Whether to add a bit of blur to the gradient like in a sobel filter.
        */
        explicit HalfFloatGridSource(const String &serializedVolumeFile, const bool trilinearValue = true, const bool trilinearGradient = false, const bool sobelGradient = false);
        
        /** Sets the maximum absolute density value to be written into the data when combining,
        influencing the compression rate on serialization. Set it to 0.0 to deactivate.
        @param maxClampedAbsoluteDensity
            The maximum absolute density value.
        */
        void setMaxClampedAbsoluteDensity(Real maxClampedAbsoluteDensity);
        
        /** Gets the maximum absolute density value to be written into the data when combining,
        influencing the compression rate on serialization.
        @return
            The maximum absolute density value, 0.0 when deactivated.
        */
        Real getMaxClampedAbsoluteDensity(void) const;

        /** Destructor.
        */
        ~HalfFloatGridSource(void);

    };
    /** @} */
    /** @} */
}
}

#endif
