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

#include "Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopy.h"
#include "Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopyDef.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"

#include "OgreDepthBuffer.h"
#include "OgreRenderTarget.h"

namespace Ogre
{
    void CompositorPassDepthCopyDef::setDepthTextureCopy( const String &srcTextureName,
                                                          const String &dstTextureName )
    {
        if( srcTextureName.find( "global_" ) == 0 )
        {
            mParentNodeDef->addTextureSourceName( srcTextureName, 0,
                                                  TextureDefinitionBase::TEXTURE_GLOBAL );
        }
        if( dstTextureName.find( "global_" ) == 0 )
        {
            mParentNodeDef->addTextureSourceName( dstTextureName, 0,
                                                  TextureDefinitionBase::TEXTURE_GLOBAL );
        }

        mSrcDepthTextureName = srcTextureName;
        mDstDepthTextureName = dstTextureName;
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    CompositorPassDepthCopy::CompositorPassDepthCopy( const CompositorPassDepthCopyDef *definition,
                                                      const CompositorChannel &target,
                                                      CompositorNode *parentNode ) :
                CompositorPass( definition, target, parentNode ),
                mDefinition( definition ),
                mCopyFailed( false )
    {
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassDepthCopy::execute( const Camera *lodCamera )
    {
        //Execute a limited number of times?
        if( mNumPassesLeft != std::numeric_limits<uint32>::max() )
        {
            if( !mNumPassesLeft )
                return;
            --mNumPassesLeft;
        }

        //Call beginUpdate if we're the first to use this RT
        if( mDefinition->mBeginRtUpdate )
            mTarget->_beginUpdate();

        //Fire the listener in case it wants to change anything
        CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if( listener )
            listener->passPreExecute( this );

        //Should we retrieve every update, or cache the return values
        //and listen to notifyRecreated and family of funtions?
        const CompositorChannel *srcChannel = mParentNode->_getDefinedTexture(
                                                mDefinition->mSrcDepthTextureName );
        const CompositorChannel *dstChannel = mParentNode->_getDefinedTexture(
                                                mDefinition->mDstDepthTextureName );

        DepthBuffer *srcDepthBuffer = srcChannel->target->getDepthBuffer();
        DepthBuffer *dstDepthBuffer = dstChannel->target->getDepthBuffer();
        if( !mCopyFailed )
        {
            mCopyFailed = !srcDepthBuffer->copyTo( dstDepthBuffer );
        }

        if( mCopyFailed && srcDepthBuffer != dstDepthBuffer &&
            mDefinition->mAliasDepthBufferOnCopyFailure )
        {
            dstChannel->target->attachDepthBuffer( srcDepthBuffer, false );
        }

        if( listener )
            listener->passPosExecute( this );

        //Call endUpdate if we're the last pass in a row to use this RT
        if( mDefinition->mEndRtUpdate )
            mTarget->_endUpdate();
    }
}
