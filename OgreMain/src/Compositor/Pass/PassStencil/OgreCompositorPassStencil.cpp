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

#include "Compositor/Pass/PassStencil/OgreCompositorPassStencil.h"

#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"

#include "OgreRenderSystem.h"

namespace Ogre
{
    CompositorPassStencilDef::CompositorPassStencilDef( uint32 rtIndex ) :
            CompositorPassDef( PASS_STENCIL, rtIndex ),
            mStencilCheck( false ),
            mTwoSided( false ),
            mStencilPassOp( SOP_KEEP ),
            mStencilFailOp( SOP_KEEP ),
            mStencilDepthFailOp( SOP_KEEP ),
            mCompareFunc( CMPF_ALWAYS_PASS ),
            mStencilMask( 0xFFFFFFFF ),
            mStencilRef( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    CompositorPassStencil::CompositorPassStencil( const CompositorPassStencilDef *definition,
                                                    const CompositorChannel &target,
                                                    CompositorNode *parentNode,
                                                    RenderSystem *renderSystem ) :
                CompositorPass( definition, target, parentNode ),
                mDefinition( definition ),
                mRenderSystem( renderSystem )
    {
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassStencil::execute( const Camera *lodCamera )
    {
        //Execute a limited number of times?
        if( mNumPassesLeft != std::numeric_limits<uint32>::max() )
        {
            if( !mNumPassesLeft )
                return;
            --mNumPassesLeft;
        }

        //Fire the listener in case it wants to change anything
        CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if( listener )
            listener->passPreExecute( this );

        mRenderSystem->setStencilCheckEnabled( mDefinition->mStencilCheck );
        mRenderSystem->setStencilBufferParams( mDefinition->mCompareFunc, mDefinition->mStencilRef,
                                    mDefinition->mStencilMask, 0xFFFFFFFF, mDefinition->mStencilFailOp,
                                    mDefinition->mStencilDepthFailOp, mDefinition->mStencilPassOp,
                                    mDefinition->mTwoSided );
    }
}
