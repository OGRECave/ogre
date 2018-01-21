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
#ifndef _OgreForward3D_H_
#define _OgreForward3D_H_

#include "OgrePrerequisites.h"
#include "OgreForwardPlusBase.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** Forward3D */
    class _OgreExport Forward3D : public ForwardPlusBase
    {
        struct Resolution
        {
            uint32 width;
            uint32 height;
            Real   zEnd; /// Depth at which this slice ends, in view space.
            Resolution() : width( 0 ), height( 0 ), zEnd( 0 ) {}
            Resolution( uint32 w, uint32 h, Real _zEnd ) :
                width( w ), height( h ), zEnd( _zEnd ) {}
        };

        uint32  mWidth;
        uint32  mHeight;
        uint32  mNumSlices;
        uint32  mLightsPerCell;
        uint32  mTableSize; /// Automatically calculated, size of the first table, elements.

        FastArray<Resolution>   mResolutionAtSlice;

        float   mMinDistance;
        float   mMaxDistance;
        float   mInvMaxDistance;

        /// Performs the reverse of getSliceAtDepth. @see getSliceAtDepth.
        inline Real getDepthAtSlice( uint32 slice ) const;

        /** Returns the slice index at the given depth
        @param depth
            Depth, in view space.
        @return
            Slice index, in range [0; mNumSlices)
        */
        inline uint32 getSliceAtDepth( Real depth ) const;

        /** Converts a vector in projection space to grid space from the specified slice index.
        @param projSpace
            projSpace.x & projSpace.y should be in range [0; 1]
        @param slice
            Slice index.
        @param outX [out]
            The column of table in the given slice.
            Will be in range [0; mResolutionAtSlice[slice].width)
        @param outY [out]
            The row of the table in the given slice.
            Will be in range [0; mResolutionAtSlice[slice].height)
        */
        inline void projectionSpaceToGridSpace( const Vector2 &projSpace, uint32 slice,
                                                uint32 &outX, uint32 &outY ) const;

    public:
        Forward3D( uint32 width, uint32 height, uint32 numSlices, uint32 lightsPerCell,
                   float minDistance, float maxDistance, SceneManager *sceneManager );
        virtual ~Forward3D();

        virtual ForwardPlusMethods getForwardPlusMethod(void) const     { return MethodForward3D; }

        virtual void collectLights( Camera *camera );

        uint32 getWidth(void) const                                     { return mWidth; }
        uint32 getHeight(void) const                                    { return mHeight; }
        uint32 getNumSlices(void) const                                 { return mNumSlices; }
        uint32 getLightsPerCell(void) const                             { return mLightsPerCell; }
        float getMinDistance(void) const                                { return mMinDistance; }
        float getMaxDistance(void) const                                { return mMaxDistance; }

        /// Returns the amount of bytes that fillConstBufferData is going to fill.
        virtual size_t getConstBufferSize(void) const;

        /** Fills 'passBufferPtr' with the necessary data for Forward3D rendering.
            @see getConstBufferSize
        @remarks
            Assumes 'passBufferPtr' is aligned to a vec4/float4 boundary.
        */
        virtual void fillConstBufferData( Viewport *viewport, RenderTarget* renderTarget,
                                          IdString shaderSyntax,
                                          float * RESTRICT_ALIAS passBufferPtr ) const;

        virtual void setHlmsPassProperties( Hlms *hlms );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
