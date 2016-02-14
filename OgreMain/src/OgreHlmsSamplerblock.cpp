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

#include "OgreStableHeaders.h"

#include "OgreHlmsSamplerblock.h"
#include <float.h>

namespace Ogre
{
    HlmsSamplerblock::HlmsSamplerblock() :
        BasicBlock( BLOCK_SAMPLER ),
        mMinFilter( FO_LINEAR ),
        mMagFilter( FO_LINEAR ),
        mMipFilter( FO_LINEAR ),
        mU( TAM_CLAMP ),
        mV( TAM_CLAMP ),
        mW( TAM_CLAMP ),
        mMipLodBias( 0.0f ),
        mMaxAnisotropy( 1.0f ),
        mCompareFunction( NUM_COMPARE_FUNCTIONS ),
        mBorderColour( ColourValue::White ),
        mMinLod( -FLT_MAX ),
        mMaxLod( FLT_MAX )
    {
    }
    //-----------------------------------------------------------------------------------
    void HlmsSamplerblock::setFiltering( TextureFilterOptions filterType )
    {
        switch( filterType )
        {
        case TFO_NONE:
            mMinFilter = FO_POINT;
            mMagFilter = FO_POINT;
            mMipFilter = FO_NONE;
            break;
        case TFO_BILINEAR:
            mMinFilter = FO_LINEAR;
            mMagFilter = FO_LINEAR;
            mMipFilter = FO_POINT;
            break;
        case TFO_TRILINEAR:
            mMinFilter = FO_LINEAR;
            mMagFilter = FO_LINEAR;
            mMipFilter = FO_LINEAR;
            break;
        case TFO_ANISOTROPIC:
            mMinFilter = FO_ANISOTROPIC;
            mMagFilter = FO_ANISOTROPIC;
            // OpenGL doesn't support anisotropic on mip filter, linear will be used automatically.
            mMipFilter = FO_ANISOTROPIC;
            break;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsSamplerblock::setAddressingMode( TextureAddressingMode addressingMode )
    {
        mU = addressingMode;
        mV = addressingMode;
        mW = addressingMode;
    }
}
