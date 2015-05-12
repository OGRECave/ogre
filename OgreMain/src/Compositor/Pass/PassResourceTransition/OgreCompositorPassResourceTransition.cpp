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

#include "Compositor/Pass/PassResourceTransition/OgreCompositorPassResourceTransition.h"

#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"

#include "OgreRenderSystem.h"

namespace Ogre
{
    CompositorPassResourceTransitionDef::CompositorPassResourceTransitionDef() :
        CompositorPassDef( PASS_RESOURCE_TRANSITION, 0 )
    {

    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    CompositorPassResourceTransition::CompositorPassResourceTransition(
                                                    CompositorPassResourceTransitionDef *definition,
                                                    CompositorNode *parentNode,
                                                    RenderSystem *renderSystem ) :
                CompositorPass( definition, parentNode, true ),
                mDefinition( definition ),
                mRenderSystem( renderSystem )
    {
    }
    //-----------------------------------------------------------------------------------
    CompositorPassResourceTransition::~CompositorPassResourceTransition()
    {
        OGRE_DELETE mDefinition;
        mDefinition = 0;
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassResourceTransition::execute( const Camera *lodCamera )
    {
        //Fire the listener in case it wants to change anything
        CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if( listener )
            listener->passPreExecute( this );


        if( listener )
            listener->passPosExecute( this );
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassResourceTransition::addTransition( const ResourceTransition &transition )
    {
        mResourceTransitions.push_back( transition );
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassResourceTransition::_prepareBarrierAndEmulateUavExecution(
                                            BoundUav boundUavs[64], ResourceAccessMap &uavsAccess,
                                            ResourceLayoutMap &resourcesLayout,
                                            CompositorPassResourceTransition **transitionPass )
    {
        ResourceTransitionVec::const_iterator itor = mResourceTransitions.begin();
        ResourceTransitionVec::const_iterator end  = mResourceTransitions.end();

        while( itor != end )
        {
            if( itor->newLayout == ResourceLayout::Uav &&
                itor->writeBarrierBits & WriteBarrier::Uav &&
                itor->readBarrierBits & ReadBarrier::Uav )
            {
                //resourcesLayout.
                //itor->resource.
                RenderTarget *renderTarget = 0;
                resourcesLayout[renderTarget] = ResourceLayout::Uav;
                //Set to undefined so that regular passes can see it's safe / shielded by a barrier
                uavsAccess[renderTarget] = ResourceAccess::Undefined;
            }
            ++itor;
        }
    }
}
