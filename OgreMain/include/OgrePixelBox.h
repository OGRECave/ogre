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
#ifndef _PixelBox_H__
#define _PixelBox_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgrePixelFormat.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Image
    *  @{
    */

    class _OgreExport PixelBox: public Box, public ImageAlloc {
    public:
        /// Parameter constructor for setting the members manually
        PixelBox() {}
        ~PixelBox() {}
        /** Constructor providing extents in the form of a Box object. This constructor
            assumes the pixel data is laid out consecutively in memory. (this
            means row after row, slice after slice, with no space in between)
            @param extents      Extents of the region defined by data
            @param pixelFormat  Format of this buffer
            @param pixelData    Pointer to the actual data
        */
        PixelBox(const Box &extents, PixelFormat pixelFormat, void *pixelData=0):
            Box(extents), data(pixelData), format(pixelFormat)
        {
            setConsecutive();
        }
        /** Constructor providing width, height and depth. This constructor
            assumes the pixel data is laid out consecutively in memory. (this
            means row after row, slice after slice, with no space in between)
            @param width        Width of the region
            @param height       Height of the region
            @param depth        Depth of the region
            @param pixelFormat  Format of this buffer
            @param pixelData    Pointer to the actual data
        */
        PixelBox(uint32 width, uint32 height, uint32 depth, PixelFormat pixelFormat, void *pixelData=0):
            Box(0, 0, 0, width, height, depth),
            data(pixelData), format(pixelFormat)
        {
            setConsecutive();
        }
        
        /// The data pointer 
        void *data;
        /// The pixel format 
        PixelFormat format;
        /** Number of elements between the leftmost pixel of one row and the left
            pixel of the next. This value must always be equal to getWidth() (consecutive) 
            for compressed formats.
        @remarks
            For compressed formats, this value is in bytes; not in elements.
        */
        size_t rowPitch;
        /** Number of elements between the top left pixel of one (depth) slice and 
            the top left pixel of the next. This can be a negative value. Must be a multiple of
            rowPitch. This value must always be equal to getWidth()*getHeight() (consecutive) 
            for compressed formats.
        @remarks
            For compressed formats, this value is in bytes; not in elements.
        */
        size_t slicePitch;

        /// Returns rowPitch, but always in bytes.
        size_t rowPitchAlwaysBytes(void) const;
        /// Returns slicePitch, but always in bytes.
        size_t slicePitchAlwaysBytes(void) const;
        
        /** Set the rowPitch and slicePitch so that the buffer is laid out consecutive 
            in memory.
        */        
        void setConsecutive();

        /** Get the number of elements between one past the rightmost pixel of 
            one row and the leftmost pixel of the next row. (IE this is zero if rows
            are consecutive).
        @remarks
            For compressed formats, this value is in bytes; not in elements.
        */
        size_t getRowSkip() const;

        /** Get the number of elements between one past the right bottom pixel of
            one slice and the left top pixel of the next slice. (IE this is zero if slices
            are consecutive).
        @remarks
            For compressed formats, this value is in bytes; not in elements.
        */
        size_t getSliceSkip() const { return slicePitch - (getHeight() * rowPitch); }

        /// @see getSliceSkip, but value is always in bytes.
        size_t getSliceSkipAlwaysBytes() const;

        /** Return whether this buffer is laid out consecutive in memory (ie the pitches
            are equal to the dimensions)
        */        
        bool isConsecutive() const;

        /** Return the size (in bytes) this image would take if it was
            laid out consecutive in memory
        */
        size_t getConsecutiveSize() const;
        /** Return a subvolume of this PixelBox.
            @param def  Defines the bounds of the subregion to return
            @return A pixel box describing the region and the data in it
            @remarks    This function does not copy any data, it just returns
                a PixelBox object with a data pointer pointing somewhere inside 
                the data of object.
            @throws Exception(ERR_INVALIDPARAMS) if def is not fully contained
        */
        PixelBox getSubVolume(const Box &def) const;
        
        /** Return a data pointer pointing to top left front pixel of the pixel box.
            @remarks Non consecutive pixel boxes are supported.
         */
        void* getTopLeftFrontPixelPtr() const;
        
        /**
         * Get colour value from a certain location in the PixelBox. The z coordinate
         * is only valid for cubemaps and volume textures. This uses the first (largest)
         * mipmap.
         */
        ColourValue getColourAt(size_t x, size_t y, size_t z);

        /**
         * Set colour value at a certain location in the PixelBox. The z coordinate
         * is only valid for cubemaps and volume textures. This uses the first (largest)
         * mipmap.
         */
        void setColourAt(ColourValue const &cv, size_t x, size_t y, size_t z);
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
