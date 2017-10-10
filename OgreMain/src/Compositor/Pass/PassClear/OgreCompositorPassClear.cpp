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

#include "Compositor/Pass/PassClear/OgreCompositorPassClear.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"

#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "OgreSceneManager.h"
#include "OgreRenderTarget.h"

namespace Ogre
{
    CompositorPassClear::CompositorPassClear( const CompositorPassClearDef *definition,
                                                SceneManager *sceneManager,
                                                const CompositorChannel &target,
                                                CompositorNode *parentNode ) :
                CompositorPass( definition, target, parentNode ),
                mSceneManager( sceneManager ),
                mDefinition( definition )
    {
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassClear::execute( const Camera *lodCamera )
    {
        //Execute a limited number of times?
        if( mNumPassesLeft != std::numeric_limits<uint32>::max() )
        {
            if( !mNumPassesLeft )
                return;
            --mNumPassesLeft;
        }

        profilingBegin();

        CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if( listener )
            listener->passEarlyPreExecute( this );

        executeResourceTransitions();

        mSceneManager->_setViewport( mViewport );

        //Fire the listener in case it wants to change anything
        if( listener )
            listener->passPreExecute( this );

        if( mDefinition->mDiscardOnly )
        {
            mViewport->discard( mDefinition->mClearBufferFlags );
        }
        else
        {
            mTarget->setFsaaResolveDirty();
            mViewport->clear( mDefinition->mClearBufferFlags, mDefinition->mColourValue,
                              mDefinition->mDepthValue, mDefinition->mStencilValue );
        }

        if( listener )
            listener->passPosExecute( this );

        profilingEnd();
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassClear::_placeBarriersAndEmulateUavExecution( BoundUav boundUavs[64],
                                                                    ResourceAccessMap &uavsAccess,
                                                                    ResourceLayoutMap &resourcesLayout )
    {
        RenderSystem *renderSystem = mParentNode->getRenderSystem();
        const RenderSystemCapabilities *caps = renderSystem->getCapabilities();
        const bool explicitApi = caps->hasCapability( RSC_EXPLICIT_API );

        if( !explicitApi )
            return;

        //Check <anything> -> Clear
        ResourceLayoutMap::iterator currentLayout = resourcesLayout.find( mTarget );
        if( currentLayout->second != ResourceLayout::Clear )
        {
            addResourceTransition( currentLayout,
                                   ResourceLayout::Clear,
                                   ReadBarrier::RenderTarget );
        }

        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "D3D12/Vulkan/Mantle - Missing DepthBuffer ResourceTransition code",
                     "CompositorPassDepthCopy::_placeBarriersAndEmulateUavExecution" );

        //Do not use base class functionality at all.
        //CompositorPass::_placeBarriersAndEmulateUavExecution();
    }
}
