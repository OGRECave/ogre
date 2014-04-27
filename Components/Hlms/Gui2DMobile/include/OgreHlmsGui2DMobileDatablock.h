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
#ifndef _OgreHlmsGui2DMobileDatablock_H_
#define _OgreHlmsGui2DMobileDatablock_H_

#include "OgreHlmsDatablock.h"
#include "OgreMatrix4.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    /// Contains information needed by the UI (2D) for OpenGL ES 2.0
    class _OgreExport HlmsGui2DMobileDatablock : public HlmsDatablock
    {
    public:
        /// Up to 8 matrices; RS APIs don't let us to pass through
        /// more than 8 UVs to the pixel shader anyway
        uint32  mNumTextureMatrices;
        float   mTextureMatrices[16*8];

        bool    mHasColour;         /// When false; mR, mG, mB & mA aren't passed to the pixel shader
        uint8   mNumDiffuseTextures;
        float   mR, mG, mB, mA;

        /// Up to 16 diffuse textures (they can re use UVs); which is the limit for a lot of HW
        /// Must be contiguous (i.e. if mDiffuseTextures[1] isn't used, mDiffuseTextures[2] can't be)
        TexturePtr mDiffuseTextures[16];

        HlmsGui2DMobileDatablock( IdString name, Hlms *creator,
                              const HlmsMacroblock *macroblock,
                              const HlmsBlendblock *blendblock,
                              const HlmsParamVec &params ) :
            HlmsDatablock( name, creator, macroblock, blendblock, params ),
            mNumTextureMatrices( 0 ),
            mHasColour( false ),
            mNumDiffuseTextures( 0 ),
            mR( 1.0f ), mG( 1.0f ), mB( 1.0f ), mA( 1.0f )
        {
            for( size_t i=0; i<sizeof(mTextureMatrices) / sizeof(Matrix4); ++i )
            {
                mTextureMatrices[i+ 0] = 1.0f;
                mTextureMatrices[i+ 1] = 0.0f;
                mTextureMatrices[i+ 2] = 0.0f;
                mTextureMatrices[i+ 3] = 0.0f;

                mTextureMatrices[i+ 4] = 0.0f;
                mTextureMatrices[i+ 5] = 1.0f;
                mTextureMatrices[i+ 6] = 0.0f;
                mTextureMatrices[i+ 7] = 0.0f;

                mTextureMatrices[i+ 8] = 0.0f;
                mTextureMatrices[i+ 9] = 0.0f;
                mTextureMatrices[i+10] = 1.0f;
                mTextureMatrices[i+11] = 0.0f;

                mTextureMatrices[i+12] = 0.0f;
                mTextureMatrices[i+13] = 0.0f;
                mTextureMatrices[i+14] = 0.0f;
                mTextureMatrices[i+15] = 1.0f;
            }
        }

        virtual void calculateHash();
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
