/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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

#include "OgreMetalMultiRenderTarget.h"
#include "OgreMetalRenderTexture.h"

#include "OgreException.h"

namespace Ogre
{
    MetalMultiRenderTarget::MetalMultiRenderTarget( const String &name ) :
        MultiRenderTarget( name ),
        mNumMRTs( 0 )
    {
        memset( mMetalRenderTargetCommon, 0, sizeof(mMetalRenderTargetCommon) );

        mWidth  = 0;
        mHeight = 0;
        mFSAA = 1u;
    }
    //-----------------------------------------------------------------------------------
    MetalMultiRenderTarget::~MetalMultiRenderTarget()
    {
    }
    //-----------------------------------------------------------------------------------
    void MetalMultiRenderTarget::bindSurfaceImpl( size_t attachment, RenderTexture *target )
    {
        assert( attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS );
        assert( mNumMRTs < OGRE_MAX_MULTIPLE_RENDER_TARGETS &&
                "Reached max number of attachments!" );
        assert( !mMetalRenderTargetCommon[attachment] &&
                "RTT already attached at this slot! "
                "You must call MultiRenderTarget::unbindSurface first!" );

        if( mWidth )
        {
            if( mWidth != target->getWidth() && mHeight != target->getHeight() &&
                mFSAA != target->getFSAA() )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "MultiRenderTarget surfaces are not of same size",
                             "MetalMultiRenderTarget::bindSurface" );
            }
        }

        mWidth  = target->getWidth();
        mHeight = target->getHeight();
        mFSAA   = target->getFSAA();

        assert( dynamic_cast<MetalRenderTexture*>( target ) );
        MetalRenderTexture *metalRtt = static_cast<MetalRenderTexture*>( target );

        mMetalRenderTargetCommon[attachment] = metalRtt;
        ++mNumMRTs;
    }
    //-----------------------------------------------------------------------------------
    void MetalMultiRenderTarget::unbindSurfaceImpl( size_t attachment )
    {
        assert( attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS );

        if( mMetalRenderTargetCommon[attachment] )
        {
            mMetalRenderTargetCommon[attachment] = 0;
            --mNumMRTs;

            if( !mNumMRTs )
            {
                mWidth  = 0;
                mHeight = 0;
                mFSAA   = 1u;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalMultiRenderTarget::getCustomAttribute( const String& name, void *pData )
    {
        if( name == "MetalRenderTargetCommon" )
        {
            MetalRenderTargetCommon **metalRttCommon = static_cast<MetalRenderTargetCommon**>(pData);
            for( size_t i=0; i<mNumMRTs; ++i )
                metalRttCommon[i] = mMetalRenderTargetCommon[i];
        }
        else if( name == "mNumMRTs" )
        {
            *static_cast<uint8*>(pData) = mNumMRTs;
        }
        else if( name == "MetalDevice" )
        {
            if( mMetalRenderTargetCommon[0] )
            {
                *static_cast<MetalDevice**>(pData) = mMetalRenderTargetCommon[0]->getOwnerDevice();
            }
            else
            {
                *static_cast<MetalDevice**>(pData) = 0;
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Cannot get 'MetalDevice' while no RTT is attached to this MRT",
                             "MetalMultiRenderTarget::getCustomAttribute" );
            }
        }
        else
        {
            MultiRenderTarget::getCustomAttribute( name, pData );
        }
    }
    //-----------------------------------------------------------------------------------
}
