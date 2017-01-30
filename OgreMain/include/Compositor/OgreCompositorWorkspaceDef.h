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

#ifndef __CompositorWorkspaceDef_H__
#define __CompositorWorkspaceDef_H__

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreTextureDefinition.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    /** @See CompositorWorkspace. Workspace definitions assume all other definitions are already
        parsed as we need them to perform validation checks.
        Workspace definitions work by assigning aliases to each node. A node whose name is the
        same as its alias is called an implicit alias.
    @remarks
        Aliasing nodes allows having more than one instance of the same node.
    @author
        Matias N. Goldberg
    @version
        1.1
    */
    class _OgreExport CompositorWorkspaceDef : public TextureDefinitionBase
    {
        friend class CompositorWorkspace;
    public:
        struct ChannelRoute
        {
            uint32      outChannel;
            IdString    outNode;        /// Name of the alias
            uint32      inChannel;
            IdString    inNode;         /// Name of the alias
            ChannelRoute( uint32 _outChannel, IdString _outNode, uint32 _inChannel, IdString _inNode ) :
                        outChannel( _outChannel ), outNode( _outNode ),
                        inChannel( _inChannel ), inNode( _inNode ) {}
        };

        typedef map<IdString, IdString>::type   NodeAliasMap;
        typedef list<ChannelRoute>::type        ChannelRouteList;
    protected:

        IdString            mName;
        String              mNameStr;
        NodeAliasMap        mAliasedNodes;
        ChannelRouteList    mChannelRoutes;
        ChannelRouteList    mBufferChannelRoutes;
        
        /// outChannel  => Index to CompositorWorkspace::mExternalRenderTargets.
        /// outNode     => Not used.
        /// inChannel   => Which channel to connect to.
        /// inNode      => Which node to connect to.
        ChannelRouteList	mExternalChannelRoutes;

        /// outChannel  => Index to CompositorWorkspace::mExternalBuffers
        /// outNode     => Not used.
        /// inChannel   => Which channel to connect to.
        /// inNode      => Which node to connect to.
        ChannelRouteList    mExternalBufferChannelRoutes;

        CompositorManager2  *mCompositorManager;

        /** Checks if nodeName is already aliased (whether explicitly or implicitly). If not,
            checks whether the name of the node corresponds to an actual Node definition.
            If so, creates the implicit alias; otherwise throws
        @remarks
            This means it's only safe to pass names that aren't Nodes if it's already
             been explicitly aliased.
        @param
            Name of the node definition.
        */
        void createImplicitAlias( IdString nodeName );

        /// Checks the input channel of the given node is not in use.
        /// Logs a warning if it's in use. Generic version.
        void checkInputChannelIsEmpty( const ChannelRouteList &internalChannelRoutes,
                                       const ChannelRouteList &externalChannelRoutes,
                                       IdString inNode, uint32 inChannel,
                                       const std::string &outNodeName,
                                       uint32 outChannel ) const;

        /// Checks the input channel of the given node is not in use.
        /// Logs a warning if it's in use.
        void checkInputChannelIsEmpty( IdString inNode, uint32 inChannel,
                                       const std::string &outNodeName,
                                       uint32 outChannel ) const;

        /// Checks the buffer input channel of the given node is not in use.
        /// Logs a warning if it's in use.
        void checkInputBufferChannelIsEmpty( IdString inNode, uint32 inChannel,
                                             const std::string &outNodeName,
                                             uint32 outChannel ) const;

    public:
        CompositorWorkspaceDef( const String& name, CompositorManager2 *compositorManager );
        virtual ~CompositorWorkspaceDef() {}

        IdString getName(void) const                                { return mName; }
        String getNameStr(void) const                               { return mNameStr; }

        /** Connects outNode's output channel to inNode's input channel.
        @remarks
            This mapping will later be used to know how connections should be done when
            instantiating. @See CompositorNode::connectTo
            If outNode & inNode are not yet aliased, an alias for them will be created.
        */
        void connect( IdString outNode, uint32 outChannel, IdString inNode, uint32 inChannel );

        /** Connects all outputs channels from outNode to all input channels from inNode. If
            the number of channels don't match, only the first N channels are set (where N is
            the minimum between outNode's output channels and inNode's input channels).
        @remarks
            If outNode & inNode are not yet aliased, an alias for them will be created.
        */
        void connect( IdString outNode, IdString inNode );

        /** Connects the (probably "final") node by passing the RenderWindow in the given input channel
        @remarks
            @see connect
            inNode is not yet aliased, an implicit alias will be created.
        */
        void connectExternal( uint32 externalIdx, IdString inNode, uint32 inChannel );

        /** Connects outNode's output buffer channel to inNode's input buffer channel.
        @remarks
            This mapping will later be used to know how connections should be done when
            instantiating. @See CompositorNode::connectBufferTo
            If outNode & inNode are not yet aliased, an alias for them will be created.
        */
        void connectBuffer( IdString outNode, uint32 outChannel, IdString inNode, uint32 inChannel );

        /** Connects all output buffer channels from outNode to all input buffer channels from inNode.
            If the number of channels don't match, only the first N channels are set (where N is
            the minimum between outNode's output channels and inNode's input channels).
        @remarks
            If outNode & inNode are not yet aliased, an alias for them will be created.
        */
        void connectBuffer( IdString outNode, IdString inNode );

        /** Connects an external buffer to the given input channel
        @remarks
            If inNode is not yet aliased, an implicit alias will be created.
        */
        void connectExternalBuffer( uint32 externalBufferIdx, IdString inNode, uint32 inChannel );

        /** Clears all the connection between channels of the nodes (@see connect).
        @remarks
            1. We don't clear the output connection (@see connectOutput, @see clearOutputConnections)
            2. The node aliases (both implicit and explicit) will still exist. @See clearAll.
            3. A node with incomplete inputs should be disabled before the workspace is instantiated
               (@see CompositorNodeDef::setStartEnabled). If the workspace has already been instantiated,
               the node instance should be disabled, @see CompositorNode::setEnabled)
            4. It is safe to call this function while there are still workspaces, but you must call
               @Workspace::reconnectAllNodes after you're done setting the new node connections
        */
        void clearAllInterNodeConnections(void);

        /** Clears the connection from the "final output RenderTarget" (i.e. usually the RenderWindow)
            that goes to the input channel of one of our nodes. @See connectOutput.
        @remarks
            1. We don't clear other type of connections (@see connect, @see clearAllInterNodeConnections)
            2. The node aliases (both implicit and explicit) will still exist. @See clearAll.
            3. A node with incomplete inputs should be disabled before the workspace is instantiated
               (@see CompositorNodeDef::setStartEnabled). If the workspace has already been instantiated,
               the node instance should be disabled, @see CompositorNode::setEnabled)
            4. It is safe to call this function while there are still workspaces, but you must call
               @Workspace::reconnectAllNodes after you're done setting the new node connections
        */
        void clearOutputConnections(void);

        /** Clears everything: All node aliases, and their connections (including output connection).
        @remarks
            This function shouldn't be called while there are still instantiated workspaces
            It is safe to call this function while there are still workspaces, but you must call
            @Workspace::recreateAllNodes after you're done setting the new node connections.
        */
        void clearAll(void);

        /** An alias is explicitly used when the user wants to use multiple, independent
            instances of the same node. Each alias equals one instance.
            An implicit alias is when the name of the alias and it's node name match.
        @remarks
            When the name of the node and its alias are the same, it is said to be called an
            "implicit" alias.
            This function will throw if trying the alias is already taken by a node
            definition (except for implicit aliases)
            This function will throw if the alias is already in use.
        @param alias
            Name of the alias (instance). May be equal to nodeName.
        @param nodeName
            Name of the node definition
        */
        void addNodeAlias( IdString alias, IdString nodeName );

        /** Removes a particular Node. All of its connections to other node are also removed,
            which may leave other nodes with its inputs in an incomplete state.
        */
        void removeNodeAlias( IdString alias );

        /** Gets read-only access to the map to all added nodes and their aliases.
            Useful to know which nodes are in use by this compositor.
            Use @addNodeAlias @removeNodeAlias and @connect to safely modify the map.
        */
        const NodeAliasMap& getNodeAliasMap(void)                   { return mAliasedNodes; }

        /** Gets direct access to the channel route (aka the interconnections between all of our nodes).
            Useful for advanced C++ users who want fine control of the connections.
        @remarks
            Incorrect manipulation of the channel routes can lead to glitches or crashes.
            Remember:
                1. Two nodes can't connect to the same node alias on the same channel
                2. Don't reference a node or alias that isn't in mAliasedNodes by the
                   time the Workspace is instantiated
                3. A node that doesn't have all of its input channels connected is incomplete
                   and should be disabled for the Workspace instance to be valid.
        */
        ChannelRouteList& _getChannelRoutes(void)                   { return mChannelRoutes; }

        CompositorManager2* getCompositorManager(void) const        { return mCompositorManager; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
