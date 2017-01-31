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

#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

namespace Ogre
{
    CompositorWorkspaceDef::CompositorWorkspaceDef( const String& name,
                                                    CompositorManager2 *compositorManager ) :
            TextureDefinitionBase( TEXTURE_GLOBAL ),
            mName( name ),
            mNameStr( name ),
            mCompositorManager( compositorManager )
    {
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::createImplicitAlias( IdString nodeName )
    {
        if( mAliasedNodes.find( nodeName ) == mAliasedNodes.end() )
        {
            if( !mCompositorManager->hasNodeDefinition( nodeName ) )
            {
                OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                             "Can't find node '" + nodeName.getFriendlyText() + "'. "
                             "Note declaration order is important. You may need to define "
                             "it earlier if the name is correct.",
                             "CompositorWorkspaceDef::createImplicitAlias" );
            }
            else
            {
                //Create the implicit alias
                mAliasedNodes[nodeName] = nodeName;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::checkInputChannelIsEmpty( const ChannelRouteList &internalChannelRoutes,
                                                           const ChannelRouteList &externalChannelRoutes,
                                                           IdString inNode, uint32 inChannel,
                                                           const std::string &outNodeName,
                                                           uint32 outChannel ) const
    {
        ChannelRouteList::const_iterator itor = internalChannelRoutes.begin();
        ChannelRouteList::const_iterator end  = internalChannelRoutes.end();

        while( itor != end )
        {
            if( itor->inNode == inNode && itor->inChannel == inChannel )
            {
                LogManager::getSingleton().logMessage( "WARNING: Node '" +
                            itor->outNode.getFriendlyText() + "' from channel #" +
                            StringConverter::toString( itor->outChannel ) + " and Node '" +
                            outNodeName + "' from channel #" + StringConverter::toString( outChannel ) +
                            "are both trying to connect its output to input channel #" +
                            StringConverter::toString( inChannel ) +
                            " of node '" + inNode.getFriendlyText() + "'. Only the latter will work" );
                break; //Early out
            }
            ++itor;
        }

        itor = externalChannelRoutes.begin();
        end  = externalChannelRoutes.end();

        while( itor != end )
        {
            if( itor->inNode == inNode && itor->inChannel == inChannel )
            {
                LogManager::getSingleton().logMessage( "WARNING: An external buffer/texture "
                            "from channel #" + StringConverter::toString( itor->outChannel ) +
                            " and Node '" + outNodeName + "' from channel #" +
                            StringConverter::toString( outChannel ) + " are both trying to connect "
                            "its output to input channel #" + StringConverter::toString( inChannel ) +
                            " of node '" + inNode.getFriendlyText() + "'. Only the latter will work" );
                break; //Early out
            }
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::checkInputChannelIsEmpty( IdString inNode, uint32 inChannel,
                                                           const std::string &outNodeName,
                                                           uint32 outChannel ) const
    {
        checkInputChannelIsEmpty( mChannelRoutes, mExternalChannelRoutes,
                                  inNode, inChannel, outNodeName, outChannel );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::checkInputBufferChannelIsEmpty( IdString inNode,
                                                                 uint32 inChannel,
                                                                 const std::string &outNodeName,
                                                                 uint32 outChannel ) const
    {
        checkInputChannelIsEmpty( mBufferChannelRoutes, mExternalBufferChannelRoutes,
                                  inNode, inChannel, outNodeName, outChannel );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::connect( IdString outNode, uint32 outChannel,
                                          IdString inNode, uint32 inChannel )
    {
        checkInputChannelIsEmpty( inNode, inChannel, outNode.getFriendlyText(), outChannel );

        createImplicitAlias( outNode );
        createImplicitAlias( inNode );

        mChannelRoutes.push_back( ChannelRoute( outChannel, outNode, inChannel, inNode ) );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::connect( IdString outNode, IdString inNode )
    {
        const CompositorNodeDef *outDef = mCompositorManager->getNodeDefinition( outNode );
        const CompositorNodeDef *inDef  = mCompositorManager->getNodeDefinition( inNode );

        size_t inputChannels  = inDef->getNumInputChannels();
        size_t outputChannels = outDef->getNumOutputChannels();

        for( uint32 i=0; i<inputChannels && i<outputChannels; ++i )
            connect( outNode, i, inNode, i );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::connectExternal( uint32 externalIdx, IdString inNode,
                                                  uint32 inChannel )
    {
        checkInputChannelIsEmpty( inNode, inChannel, "connect_external / connect_output", externalIdx );
        createImplicitAlias( inNode );
        mExternalChannelRoutes.push_back( ChannelRoute( externalIdx, IdString(),
                                                        inChannel, inNode ) );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::connectBuffer( IdString outNode, uint32 outChannel,
                                                IdString inNode, uint32 inChannel )
    {
        checkInputBufferChannelIsEmpty( inNode, inChannel, outNode.getFriendlyText(), outChannel );

        createImplicitAlias( outNode );
        createImplicitAlias( inNode );

        mBufferChannelRoutes.push_back( ChannelRoute( outChannel, outNode, inChannel, inNode ) );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::connectBuffer( IdString outNode, IdString inNode )
    {
        const CompositorNodeDef *outDef = mCompositorManager->getNodeDefinition( outNode );
        const CompositorNodeDef *inDef  = mCompositorManager->getNodeDefinition( inNode );

        size_t inputChannels  = inDef->getNumInputChannels();
        size_t outputChannels = outDef->getNumOutputChannels();

        for( uint32 i=0; i<inputChannels && i<outputChannels; ++i )
            connect( outNode, i, inNode, i );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::connectExternalBuffer( uint32 externalBufferIdx, IdString inNode,
                                                        uint32 inChannel )
    {
        checkInputBufferChannelIsEmpty( inNode, inChannel, "connect_buffer_external",
                                        externalBufferIdx );
        createImplicitAlias( inNode );
        mExternalBufferChannelRoutes.push_back( ChannelRoute( externalBufferIdx, IdString(),
                                                              inChannel, inNode ) );
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::clearAllInterNodeConnections(void)
    {
        mChannelRoutes.clear();
        mBufferChannelRoutes.clear();
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::clearOutputConnections(void)
    {
        mExternalChannelRoutes.clear();
        mExternalBufferChannelRoutes.clear();
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::clearAll(void)
    {
        clearAllInterNodeConnections();
        clearOutputConnections();
        mAliasedNodes.clear();
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::addNodeAlias( IdString alias, IdString nodeName )
    {
        if( alias != nodeName && mCompositorManager->hasNodeDefinition( alias ) )
        {
            OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM,
                         "Can't use the name of a node definition as alias.",
                         "CompositorWorkspaceDef::addAlias" );
        }

        mAliasedNodes[alias] = nodeName;
    }
    //-----------------------------------------------------------------------------------
    void CompositorWorkspaceDef::removeNodeAlias( IdString alias )
    {
        NodeAliasMap::iterator it = mAliasedNodes.find( alias );
        if( it != mAliasedNodes.end() )
        {
            mAliasedNodes.erase( it );

            ChannelRouteList::iterator itor = mChannelRoutes.begin();
            ChannelRouteList::iterator end  = mChannelRoutes.end();

            while( itor != end )
            {
                if( itor->outNode == alias || itor->inNode == alias )
                {
                    itor = mChannelRoutes.erase( itor );
                }
                else
                {
                    ++itor;
                }
            }
        }
    }
}
