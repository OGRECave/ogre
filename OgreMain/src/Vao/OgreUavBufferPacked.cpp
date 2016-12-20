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

#include "OgreStableHeaders.h"
#include "Vao/OgreUavBufferPacked.h"
#include "Vao/OgreTexBufferPacked.h"
#include "Vao/OgreVaoManager.h"

namespace Ogre
{
    UavBufferPacked::UavBufferPacked(
                size_t internalBufferStartBytes, size_t numElements, uint32 bytesPerElement,
                uint32 bindFlags, void *initialData, bool keepAsShadow,
                VaoManager *vaoManager, BufferInterface *bufferInterface ) :
        BufferPacked( internalBufferStartBytes, numElements, bytesPerElement, 0, BT_DEFAULT,
                      initialData, keepAsShadow, vaoManager, bufferInterface ),
        mBindFlags( bindFlags )
    {
    }
    //-----------------------------------------------------------------------------------
    UavBufferPacked::~UavBufferPacked()
    {
        destroyAllTexBufferViews();
    }
    //-----------------------------------------------------------------------------------
    TexBufferPacked* UavBufferPacked::getAsTexBufferView( PixelFormat pixelFormat )
    {
        assert( mBindFlags & BB_FLAG_TEX && "Buffer must've been created with BB_FLAG_TEX" );

        TexBufferPacked *retVal = 0;
        vector<TexBufferPacked*>::type::const_iterator itor = mTexBufferViews.begin();
        vector<TexBufferPacked*>::type::const_iterator end  = mTexBufferViews.end();
        while( itor != end && !retVal )
        {
            if( (*itor)->getPixelFormat() == pixelFormat )
                retVal = *itor;
            ++itor;
        }

        if( !retVal )
            getAsTexBufferImpl( pixelFormat );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void UavBufferPacked::destroyTexBufferView( PixelFormat pixelFormat )
    {
        vector<TexBufferPacked*>::type::const_iterator itor = mTexBufferViews.begin();
        vector<TexBufferPacked*>::type::const_iterator end  = mTexBufferViews.end();

        while( itor != end && (*itor)->getPixelFormat() != pixelFormat )
            ++itor;

        if( itor != end )
        {
            (*itor)->_setBufferInterface( (BufferInterface*)0 );
            delete *itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void UavBufferPacked::destroyAllTexBufferViews(void)
    {
        vector<TexBufferPacked*>::type::const_iterator itor = mTexBufferViews.begin();
        vector<TexBufferPacked*>::type::const_iterator end  = mTexBufferViews.end();

        while( itor != end )
        {
            (*itor)->_setBufferInterface( (BufferInterface*)0 );
            delete *itor;
            ++itor;
        }
    }
}
