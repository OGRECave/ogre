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

#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/OgreCompositorChannel.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"
#include "OgreViewport.h"

#include "OgreStringConverter.h"

namespace Ogre
{
    CompositorPass::CompositorPass( const CompositorPassDef *definition, const CompositorChannel &target,
                                    CompositorNode *parentNode ) :
            mDefinition( definition ),
            mTarget( 0 ),
            mViewport( 0 ),
            mNumPassesLeft( definition->mNumInitialPasses ),
            mParentNode( parentNode )
    {
        assert( definition->mNumInitialPasses && "Definition is broken, pass will never execute!" );

        mTarget = calculateRenderTarget( mDefinition->getRtIndex(), target );

        const Real EPSILON = 1e-6f;

        CompositorWorkspace *workspace = mParentNode->getWorkspace();
        uint8 workspaceVpMask = workspace->getViewportModifierMask();

        bool applyModifier = (workspaceVpMask & mDefinition->mViewportModifierMask) != 0;
        Vector4 vpModifier = applyModifier ? workspace->getViewportModifier() : Vector4( 0, 0, 1, 1 );

        Real left   = mDefinition->mVpLeft      + vpModifier.x;
        Real top    = mDefinition->mVpTop       + vpModifier.y;
        Real width  = mDefinition->mVpWidth     * vpModifier.z;
        Real height = mDefinition->mVpHeight    * vpModifier.w;

        Real scLeft   = mDefinition->mVpScissorLeft     + vpModifier.x;
        Real scTop    = mDefinition->mVpScissorTop      + vpModifier.y;
        Real scWidth  = mDefinition->mVpScissorWidth    * vpModifier.z;
        Real scHeight = mDefinition->mVpScissorHeight   * vpModifier.w;

        const unsigned short numViewports = mTarget->getNumViewports();
        for( unsigned short i=0; i<numViewports && !mViewport; ++i )
        {
            Viewport *vp = mTarget->getViewport(i);
            if( Math::Abs( vp->getLeft() - left )   < EPSILON &&
                Math::Abs( vp->getTop() - top )     < EPSILON &&
                Math::Abs( vp->getWidth() - width ) < EPSILON &&
                Math::Abs( vp->getHeight() - height )<EPSILON &&
                Math::Abs( vp->getScissorLeft() - scLeft )   < EPSILON &&
                Math::Abs( vp->getScissorTop() - scTop )     < EPSILON &&
                Math::Abs( vp->getScissorWidth() - scWidth ) < EPSILON &&
                Math::Abs( vp->getScissorHeight() - scHeight )<EPSILON &&
                vp->getOverlaysEnabled() == mDefinition->mIncludeOverlays )
            {
                mViewport = vp;
            }
        }

        if( !mViewport )
        {
            mViewport = mTarget->addViewport( left, top, width, height );
            mViewport->setScissors( scLeft, scTop, scWidth, scHeight );
            mViewport->setOverlaysEnabled( mDefinition->mIncludeOverlays );
        }
    }
    //-----------------------------------------------------------------------------------
    CompositorPass::~CompositorPass()
    {
    }
    //-----------------------------------------------------------------------------------
    RenderTarget* CompositorPass::calculateRenderTarget( size_t rtIndex,
                                                         const CompositorChannel &source )
    {
        RenderTarget *retVal;

        if( !source.isMrt() && !source.textures.empty() &&
            source.textures[0]->getTextureType() > TEX_TYPE_2D )
        {
            //Who had the bright idea of handling Cubemaps differently
            //than 3D textures is a mystery. Anyway, deal with it.
            TexturePtr texturePtr = source.textures[0];

            if( rtIndex >= texturePtr->getDepth() && rtIndex >= texturePtr->getNumFaces() )
            {
                size_t maxRTs = std::max<size_t>( source.textures[0]->getDepth(),
                                                    source.textures[0]->getNumFaces() );
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                        "Compositor pass is asking for a 3D/Cubemap/2D_array texture with "
                        "more faces/depth/slices than what's been supplied (Asked for slice '" +
                        StringConverter::toString( rtIndex ) + "', RT has '" +
                        StringConverter::toString( maxRTs ) + "')",
                        "CompositorPass::calculateRenderTarget" );
            }

            /*//If goes out bounds, will reference the last slice/face
            rtIndex = std::min( rtIndex, std::max( source.textures[0]->getDepth(),
                                                    source.textures[0]->getNumFaces() ) - 1 );*/

            TextureType textureType = texturePtr->getTextureType();
            size_t face = textureType == TEX_TYPE_CUBE_MAP ? rtIndex : 0;
            size_t slice= textureType != TEX_TYPE_CUBE_MAP ? rtIndex : 0;
            retVal = texturePtr->getBuffer( face )->getRenderTarget( slice );
        }
        else
        {
            retVal = source.target;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::notifyRecreated( const CompositorChannel &oldChannel,
                                            const CompositorChannel &newChannel )
    {
        const Real EPSILON = 1e-6f;

        if( mTarget == calculateRenderTarget( mDefinition->getRtIndex(), oldChannel ) )
        {
            mTarget = calculateRenderTarget( mDefinition->getRtIndex(), newChannel );

            mNumPassesLeft = mDefinition->mNumInitialPasses;

            mViewport = 0;

            CompositorWorkspace *workspace = mParentNode->getWorkspace();
            uint8 workspaceVpMask = workspace->getViewportModifierMask();

            bool applyModifier = (workspaceVpMask & mDefinition->mViewportModifierMask) != 0;
            Vector4 vpModifier = applyModifier ? workspace->getViewportModifier() : Vector4( 0, 0, 1, 1 );

            Real left   = mDefinition->mVpLeft      + vpModifier.x;
            Real top    = mDefinition->mVpTop       + vpModifier.y;
            Real width  = mDefinition->mVpWidth     * vpModifier.z;
            Real height = mDefinition->mVpHeight    * vpModifier.w;

            Real scLeft   = mDefinition->mVpScissorLeft     + vpModifier.x;
            Real scTop    = mDefinition->mVpScissorTop      + vpModifier.y;
            Real scWidth  = mDefinition->mVpScissorWidth    * vpModifier.z;
            Real scHeight = mDefinition->mVpScissorHeight   * vpModifier.w;

            const unsigned short numViewports = mTarget->getNumViewports();
            for( unsigned short i=0; i<numViewports && !mViewport; ++i )
            {
                Viewport *vp = mTarget->getViewport(i);
                if( Math::Abs( vp->getLeft() - left )   < EPSILON &&
                    Math::Abs( vp->getTop() - top )     < EPSILON &&
                    Math::Abs( vp->getWidth() - width ) < EPSILON &&
                    Math::Abs( vp->getHeight() - height )<EPSILON &&
                    Math::Abs( vp->getScissorLeft() - scLeft )   < EPSILON &&
                    Math::Abs( vp->getScissorTop() - scTop )     < EPSILON &&
                    Math::Abs( vp->getScissorWidth() - scWidth ) < EPSILON &&
                    Math::Abs( vp->getScissorHeight() - scHeight )<EPSILON &&
                    vp->getOverlaysEnabled() == mDefinition->mIncludeOverlays )
                {
                    mViewport = vp;
                }
            }

            if( !mViewport )
            {
                mViewport = mTarget->addViewport( mDefinition->mVpLeft, mDefinition->mVpTop,
                                                  mDefinition->mVpWidth, mDefinition->mVpHeight );
                mViewport->setScissors( scLeft, scTop, scWidth, scHeight );
                mViewport->setOverlaysEnabled( mDefinition->mIncludeOverlays );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::notifyDestroyed( const CompositorChannel &channel )
    {
        if( mTarget == calculateRenderTarget( mDefinition->getRtIndex(), channel ) )
            mTarget = 0;
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::notifyCleared(void)
    {
        mTarget = 0;
    }
}
