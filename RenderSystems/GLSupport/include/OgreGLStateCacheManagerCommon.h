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

#ifndef __GLStateCacheManagerCommon_H__
#define __GLStateCacheManagerCommon_H__

#include "OgrePrerequisites.h"
#include "OgreGLSupportPrerequisites.h"
#include "OgreCommon.h"

namespace Ogre
{
    typedef GeneralAllocatedObject StateCacheAlloc;

    /** An in memory cache of the OpenGL state.

     State changes can be particularly expensive time wise. This is because
     a change requires OpenGL to re-evaluate and update the state machine.
     Because of the general purpose nature of OGRE we often set the state for
     a specific texture, material, buffer, etc. But this may be the same as the
     current status of the state machine and is therefore redundant and causes
     unnecessary work to be performed by OpenGL.

     Instead we are caching the state so that we can check whether it actually
     does need to be updated. This leads to improved performance all around and 
     can be somewhat dramatic in some cases.

     @warning caching does not work with multiple windows, sharing the same context.
     They will erroneously get different state caches.
     */
    class _OgreGLExport GLStateCacheManagerCommon : public StateCacheAlloc
    {
    protected:
        typedef std::unordered_map<uint32, uint32> BindBufferMap;
        typedef std::unordered_map<uint32, int> TexParameteriMap;
        typedef std::unordered_map<uint32, TexParameteriMap> TexUnitsMap;

        /* These variables are used for caching OpenGL state.
         They are cached because state changes can be quite expensive,
         which is especially important on mobile or embedded systems.
         */

        /// Array of each OpenGL feature that is enabled i.e. blending, depth test, etc.
        std::vector<uint32> mEnableVector;
        /// Stores the current clear colour
        float mClearColour[4];
        /// Stores the current depth clearing colour
        float mClearDepth;
        /// Stores the current colour write mask
        uchar mColourMask[4];
        /// Stores the current depth write mask
        uchar mDepthMask;
        /// Stores the current stencil mask
        uint32 mStencilMask;
        /// Viewport origin and size
        Rect mViewport;
        /// A map of different buffer types and the currently bound buffer for each type
        BindBufferMap mActiveBufferMap;
        /// Stores the current face culling setting
        uint32 mCullFace;
        /// Stores the current depth test function
        uint32 mDepthFunc;
        /// Stores the current blend equation
        uint32 mBlendEquationRGB;
        uint32 mBlendEquationAlpha;
        uint32 mBlendFuncSource;
        uint32 mBlendFuncDest;
        uint32 mBlendFuncSourceAlpha;
        uint32 mBlendFuncDestAlpha;
        /// Stores the currently active texture unit
        size_t mActiveTextureUnit;
        /// A map of texture parameters for each texture unit
        TexUnitsMap mTexUnitsMap;
    public:
        virtual ~GLStateCacheManagerCommon() {}

        /** Gets the current colour mask setting.
         @return An array containing the mask in RGBA order.
         */
        uchar* getColourMask() { return mColourMask; }

        /** Gets the current depth mask setting.
         @return The current depth mask.
         */
        uchar getDepthMask() const { return mDepthMask; }

        /** Gets the current stencil mask.
         @return The stencil mask.
         */
        uint32 getStencilMask(void) const { return mStencilMask; }
    };
}

#endif
