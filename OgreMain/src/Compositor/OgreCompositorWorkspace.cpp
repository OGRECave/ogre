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

#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "Compositor/Pass/PassScene/OgreCompositorPassScene.h"

#include "OgreSceneManager.h"
#include "OgreRenderTarget.h"
#include "OgreLogManager.h"

namespace Ogre
{
    CompositorWorkspace::CompositorWorkspace( IdType id, const CompositorWorkspaceDef *definition,
                                                const CompositorChannel &finalRenderTarget,
                                                SceneManager *sceneManager, Camera *defaultCam,
                                                RenderSystem *renderSys, bool bEnabled ) :
            IdObject( id ),
            mDefinition( definition ),
            mValid( false ),
            mEnabled( bEnabled ),
            mListener( 0 ),
            mDefaultCamera( defaultCam ),
            mSceneManager( sceneManager ),
            mRenderSys( renderSys ),
            mRenderWindow( finalRenderTarget )
    {
        //Create global textures
        TextureDefinitionBase::createTextures( definition->mLocalTextureDefs, mGlobalTextures,
                                                id, true, mRenderWindow.target, mRenderSys );

        recreateAllNodes();

        mCurrentWidth   = mRenderWindow.target->getWidth();
        mCurrentHeight  = mRenderWindow.target->getHeight();
    }
    //-----------------------------------------------------------------------------------
    CompositorWorkspace::~CompositorWorkspace()
    {
        destroyAllNodes();

        //Destroy our global textures
        TextureDefinitionBase::destroyTextures( mGlobalTextures, mRenderSys );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::createAllNodes(void)
    {
        destroyAllNodes();

        CompositorWorkspaceDef::NodeAliasMap::const_iterator itor = mDefinition->mAliasedNodes.begin();
        CompositorWorkspaceDef::NodeAliasMap::const_iterator end  = mDefinition->mAliasedNodes.end();

        const CompositorManager2 *compoManager = mDefinition->mCompositorManager;

        while( itor != end )
        {
            const CompositorNodeDef *nodeDef = compoManager->getNodeDefinition( itor->second );
            CompositorNode *newNode = OGRE_NEW CompositorNode( Id::generateNewId<CompositorNode>(),
                                                                itor->first, nodeDef, this, mRenderSys,
                                                                mRenderWindow.target );
            mNodeSequence.push_back( newNode );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::destroyAllNodes(void)
    {
        mValid = false;
        {
            CompositorNodeVec::const_iterator itor = mNodeSequence.begin();
            CompositorNodeVec::const_iterator end  = mNodeSequence.end();

            while( itor != end )
                OGRE_DELETE *itor++;
            mNodeSequence.clear();
        }

        {
            CompositorShadowNodeVec::const_iterator itor = mShadowNodes.begin();
            CompositorShadowNodeVec::const_iterator end  = mShadowNodes.end();

            while( itor != end )
                OGRE_DELETE *itor++;
            mShadowNodes.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::connectAllNodes(void)
    {
        {
            //First connect the RenderWindow, otherwise the node could end up not being processed
            CompositorNode *finalNode = findNode( mDefinition->mFinalNode );
            finalNode->connectFinalRT( mRenderWindow.target, mRenderWindow.textures,
                                       mDefinition->mFinalInChannel );
        }

        CompositorNodeVec unprocessedList( mNodeSequence.begin(), mNodeSequence.end() );
        CompositorNodeVec processedList;
        processedList.reserve( mNodeSequence.size() );

        bool noneProcessed = false;

        while( !unprocessedList.empty() && !noneProcessed )
        {
            noneProcessed = true;
            CompositorNodeVec::iterator itor = unprocessedList.begin();
            CompositorNodeVec::iterator end  = unprocessedList.end();

            while( itor != end )
            {
                CompositorNode *node = *itor;
                if( node->areAllInputsConnected() )
                {
                    //This node has no missing dependency, we can process it!
                    CompositorWorkspaceDef::ChannelRouteList::const_iterator itRoute =
                                                                    mDefinition->mChannelRoutes.begin();
                    CompositorWorkspaceDef::ChannelRouteList::const_iterator enRoute =
                                                                    mDefinition->mChannelRoutes.end();

                    //Connect all nodes according to our routing map. Perhaps I could've chosen a more
                    //efficient representation for lookup. Oh well, it's not like there's a 1000 nodes
                    //per workspace. If this is a hotspot, refactor.
                    while( itRoute != enRoute )
                    {
                        if( itRoute->outNode == node->getName() )
                        {
                            node->connectTo( itRoute->outChannel, findNode( itRoute->inNode, true ),
                                             itRoute->inChannel );
                        }
                        ++itRoute;
                    }

                    //The processed list is now in order
                    processedList.push_back( *itor );

                    //Remove processed nodes from the list. We'll keep until there's no one left
                    itor = efficientVectorRemove( unprocessedList, itor );
                    end  = unprocessedList.end();

                    noneProcessed = false;
                }
                else
                {
                    ++itor;
                }
            }
        }

        //unprocessedList should be empty by now, or contain only disabled nodes.
        bool incomplete = false;
        {
            CompositorNodeVec::const_iterator itor = unprocessedList.begin();
            CompositorNodeVec::const_iterator end  = unprocessedList.end();

            while( itor != end )
            {
                incomplete |= (*itor)->getEnabled();
                ++itor;
            }
        }

        if( incomplete )
        {
            CompositorNodeVec::const_iterator itor = unprocessedList.begin();
            CompositorNodeVec::const_iterator end  = unprocessedList.end();
            while( itor != end )
            {
                if( (*itor)->getEnabled() )
                {
                    LogManager::getSingleton().logMessage(
                        "WARNING: Node '" + (*itor)->getName().getFriendlyText() + "' has the following "
                        "channels in a disconnected state. Workspace won't work until they're solved:" );

                    const CompositorChannelVec& inputChannels = (*itor)->getInputChannel();
                    CompositorChannelVec::const_iterator itChannels = inputChannels.begin();
                    CompositorChannelVec::const_iterator enChannels = inputChannels.end();
                    while( itChannels != enChannels )
                    {
                        if( !itChannels->isValid() )
                        {
                            const size_t channelIdx = itChannels - inputChannels.begin();
                            LogManager::getSingleton().logMessage( "\t\t\t Channel # " +
                                                StringConverter::toString( channelIdx ) );
                        }
                        ++itChannels;
                    }
                }
                
                ++itor;
            }
        }
        else
        {
            // We need mNodeSequence in the right order of execution!
            mNodeSequence.clear();
            mNodeSequence.insert( mNodeSequence.end(), processedList.begin(), processedList.end() );

            CompositorNodeVec::iterator itor = mNodeSequence.begin();
            CompositorNodeVec::iterator end  = mNodeSequence.end();

            while( itor != end )
            {
                (*itor)->createPasses();
                ++itor;
            }

            //Now manage automatic shadow nodes present PASS_SCENE passes
            //(when using SHADOW_NODE_FIRST_ONLY)
            setupPassesShadowNodes();

            //unprocessedList may not be empty if they were incomplete but disabled.
            mNodeSequence.insert( mNodeSequence.end(), unprocessedList.begin(), unprocessedList.end() );

            mValid = true;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::clearAllConnections(void)
    {
        {
            CompositorNodeVec::iterator itor = mNodeSequence.begin();
            CompositorNodeVec::iterator end  = mNodeSequence.end();

            while( itor != end )
            {
                (*itor)->_notifyCleared();
                ++itor;
            }
        }

        {
            CompositorShadowNodeVec::const_iterator itor = mShadowNodes.begin();
            CompositorShadowNodeVec::const_iterator end  = mShadowNodes.end();

            while( itor != end )
                OGRE_DELETE *itor++;
            mShadowNodes.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::setupPassesShadowNodes(void)
    {
        CompositorShadowNodeVec::iterator itShadowNode = mShadowNodes.begin();
        CompositorShadowNodeVec::iterator enShadowNode = mShadowNodes.end();

        while( itShadowNode != enShadowNode )
        {
#ifndef NDEBUG
            set<Camera*>::type usedCameras;
#endif
            Camera *lastCamera = 0;
            CompositorShadowNode *shadowNode = *itShadowNode;

            CompositorNodeVec::const_iterator itor = mNodeSequence.begin();
            CompositorNodeVec::const_iterator end  = mNodeSequence.end();
            while( itor != end )
            {
                const CompositorPassVec &passes = (*itor)->_getPasses();
                CompositorPassVec::const_iterator itPasses = passes.begin();
                CompositorPassVec::const_iterator enPasses = passes.end();

                while( itPasses != enPasses )
                {
                    if( (*itPasses)->getType() == PASS_SCENE )
                    {
                        assert( dynamic_cast<CompositorPassScene*>( *itPasses ) );
                        CompositorPassScene *pass = static_cast<CompositorPassScene*>( *itPasses );

                        if( shadowNode == pass->getShadowNode() )
                        {
                            ShadowNodeRecalculation recalc =
                                                    pass->getDefinition()->mShadowNodeRecalculation;

                            if( recalc == SHADOW_NODE_RECALCULATE )
                            {
                                //We're forced to recalculate anyway, save the new camera
                                lastCamera = pass->getCamera();
#ifndef NDEBUG
                                usedCameras.insert( lastCamera );
#endif
                            }
                            else if( recalc == SHADOW_NODE_FIRST_ONLY )
                            {
                                if( lastCamera != pass->getCamera() )
                                {
                                    //Either this is the first one, or camera changed.
                                    //We need to recalculate
                                    pass->_setUpdateShadowNode( true );
                                    lastCamera = pass->getCamera();

                                    //Performance warning check. Only on non-release builds.
                                    //We don't raise the log on SHADOW_NODE_RECALCULATE because
                                    //that's explicit. We assume the user knows what he's doing.
                                    //(may be he changed the objects without us knowing
                                    //through a listener)
#ifndef NDEBUG
                                    if( usedCameras.find( lastCamera ) != usedCameras.end() )
                                    {
                                        LogManager::getSingleton().logMessage(
                "\tPerformance Warning: Shadow Node '" + (*itor)->getName().getFriendlyText() +
                "' is forced to recalculate twice (or more) its contents for the same camera.\n"
                "\tThis happens when assigning a shadow node to a pass using a different camera "
                "and then using it back again in another pass with the older camera.\n"
                "\tYou can fix this by cloning the shadow node and using the clone for the pass with "
                "a different camera. But beware you'll be trading performance for more VRAM usage.\n"
                "\tOr you can ignore this warning." );
                                    }
                                    else
                                    {
                                        usedCameras.insert( lastCamera );
                                    }
#endif

                                }
                                else
                                {
                                    pass->_setUpdateShadowNode( false );
                                }
                            }
                        }
                    }
                    ++itPasses;
                }
                ++itor;
            }
            ++itShadowNode;
        }
    }
    //-----------------------------------------------------------------------------------
    CompositorNode* CompositorWorkspace::findNode( IdString aliasName, bool includeShadowNodes ) const
    {
        CompositorNode *retVal = 0;
        CompositorNodeVec::const_iterator itor = mNodeSequence.begin();
        CompositorNodeVec::const_iterator end  = mNodeSequence.end();

        while( itor != end && !retVal )
        {
            if( (*itor)->getName() == aliasName )
                retVal = *itor;
            ++itor;
        }

        if( !retVal && includeShadowNodes )
            retVal = findShadowNode( aliasName );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const CompositorChannel& CompositorWorkspace::getGlobalTexture( IdString name ) const
    {
        size_t index;
        TextureDefinitionBase::TextureSource textureSource;
        mDefinition->getTextureSource( name, index, textureSource );
        return mGlobalTextures[index];
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::recreateAllNodes(void)
    {
        createAllNodes();
        connectAllNodes();
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::reconnectAllNodes(void)
    {
        clearAllConnections();
        connectAllNodes();
    }
    //-----------------------------------------------------------------------------------
    Camera* CompositorWorkspace::findCamera( IdString cameraName ) const 
    {
        return mSceneManager->findCamera( cameraName );
    }
    //-----------------------------------------------------------------------------------
    CompositorManager2* CompositorWorkspace::getCompositorManager()
    {
        return mDefinition->mCompositorManager;
    }
    //-----------------------------------------------------------------------------------
    const CompositorManager2* CompositorWorkspace::getCompositorManager() const
    {
        return mDefinition->mCompositorManager;
    }
    //-----------------------------------------------------------------------------------
    size_t CompositorWorkspace::getFrameCount(void) const
    {
        return mDefinition->mCompositorManager->getFrameCount();
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::_beginUpdate( bool forceBeginFrame )
    {
        //We need to do this so that D3D9 (and D3D11?) knows which device
        //is active now, so that _beginFrame calls go to the right device.
        mRenderSys->_setRenderTarget( mRenderWindow.target );
        if( mRenderWindow.target->isRenderWindow() || forceBeginFrame )
        {
            // Begin the frame
            mRenderSys->_beginFrame();
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::_endUpdate( bool forceEndFrame )
    {
        //We need to do this so that D3D9 (and D3D11?) knows which device
        //is active now, so that _endFrame calls go to the right device.
        mRenderSys->_setRenderTarget( mRenderWindow.target );
        if( mRenderWindow.target->isRenderWindow() || forceEndFrame )
        {
            // End the frame
            mRenderSys->_endFrame();
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::_update(void)
    {
        if( mListener )
            mListener->workspacePreUpdate();

        //We need to do this so that D3D9 (and D3D11?) knows which device
        //is active now, so that our calls go to the right device.
        mRenderSys->_setRenderTarget( mRenderWindow.target );

        if( mCurrentWidth != mRenderWindow.target->getWidth() || mCurrentHeight != mRenderWindow.target->getHeight() )
        {
            //Main RenderTarget reference changed resolution. Some nodes may need to rebuild
            //their textures if they're based on mRenderWindow's resolution.
            mCurrentWidth   = mRenderWindow.target->getWidth();
            mCurrentHeight  = mRenderWindow.target->getHeight();

            {
                CompositorNodeVec::const_iterator itor = mNodeSequence.begin();
                CompositorNodeVec::const_iterator end  = mNodeSequence.end();

                while( itor != end )
                {
                    CompositorNode *node = *itor;
                    node->finalTargetResized( mRenderWindow.target );
                    ++itor;
                }
            }

            {
                CompositorShadowNodeVec::const_iterator itor = mShadowNodes.begin();
                CompositorShadowNodeVec::const_iterator end  = mShadowNodes.end();

                while( itor != end )
                {
                    CompositorShadowNode *node = *itor;
                    node->finalTargetResized( mRenderWindow.target );
                    ++itor;
                }
            }

            CompositorNodeVec allNodes;
            allNodes.reserve( mNodeSequence.size() + mShadowNodes.size() );
            allNodes.insert( allNodes.end(), mNodeSequence.begin(), mNodeSequence.end() );
            allNodes.insert( allNodes.end(), mShadowNodes.begin(), mShadowNodes.end() );
            TextureDefinitionBase::recreateResizableTextures( mDefinition->mLocalTextureDefs,
                                                                mGlobalTextures, mRenderWindow.target,
                                                                mRenderSys, allNodes, 0 );
        }

        //Add global textures to the SceneManager so they can be referenced by materials
        size_t oldNumTextures = mSceneManager->getNumCompositorTextures();
        TextureDefinitionBase::NameToChannelMap::const_iterator it =
                                                                mDefinition->mNameToChannelMap.begin();
        TextureDefinitionBase::NameToChannelMap::const_iterator en =
                                                                mDefinition->mNameToChannelMap.end();

        while( it != en )
        {
            size_t index;
            TextureDefinitionBase::TextureSource texSource;
            mDefinition->decodeTexSource( it->second, index, texSource );
            mSceneManager->_addCompositorTexture( it->first, &this->mGlobalTextures[index].textures );

            ++it;
        }

        CompositorNodeVec::const_iterator itor = mNodeSequence.begin();
        CompositorNodeVec::const_iterator end  = mNodeSequence.end();

        while( itor != end )
        {
            CompositorNode *node = *itor;
            if( node->getEnabled() )
            {
                if( node->areAllInputsConnected() )
                {
                    node->_update( (Camera*)0, mSceneManager );
                }
                else
                {
                    //If we get here, this means a node didn't have all of its input channels connected,
                    //but we ignored it because the node was disabled. But now it is enabled again.
                    LogManager::getSingleton().logMessage(
                        "ERROR: Invalid Node '" + node->getName().getFriendlyText() +
                        "' was re-enabled without calling CompositorWorkspace::clearAllConnections" );
                    mValid = false;
                }
            }
            ++itor;
        }

        //Remove our textures
        mSceneManager->_removeCompositorTextures( oldNumTextures );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::_swapFinalTarget(void)
    {
        if( mRenderWindow.target )
            mRenderWindow.target->swapBuffers();
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspace::_validateFinalTarget(void)
    {
        mRenderSys->_setRenderTarget( mRenderWindow.target );
    }
    //-----------------------------------------------------------------------------------
    CompositorShadowNode* CompositorWorkspace::findShadowNode( IdString nodeDefName ) const
    {
        CompositorShadowNode *retVal = 0;

        CompositorShadowNodeVec::const_iterator itor = mShadowNodes.begin();
        CompositorShadowNodeVec::const_iterator end  = mShadowNodes.end();

        while( itor != end && !retVal )
        {
            if( (*itor)->getName() == nodeDefName )
                retVal = *itor;
            ++itor;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    CompositorShadowNode* CompositorWorkspace::findOrCreateShadowNode( IdString nodeDefName, bool &bCreated )
    {
        CompositorShadowNode *retVal = findShadowNode( nodeDefName );
        bCreated = false;

        if( !retVal )
        {
            //Not found, create one.
            const CompositorManager2 *compoManager = mDefinition->mCompositorManager;
            const CompositorShadowNodeDef *def = compoManager->getShadowNodeDefinition( nodeDefName );
            retVal = OGRE_NEW CompositorShadowNode( Id::generateNewId<CompositorNode>(),
                                                    def, this, mRenderSys, mRenderWindow.target );
            mShadowNodes.push_back( retVal );
            bCreated = true;
        }

        return retVal;
    }
}
