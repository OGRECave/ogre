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

#ifndef _OgreCompositorWorkspace_H_
#define _OgreCompositorWorkspace_H_

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/OgreCompositorChannel.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    /** A compositor workspace is the main interface to render into an RT, be it a RenderWindow or an
        RTT (Render Texture Target). Whereas Ogre 1.x needed you to set a Viewport in order to render
        to an RTT or RW and then call renderTarget->update, now you need to set a workspace definition
        instead and call workspace->_update().
    @par
        <b>Compositors are not just a "fancy effects" system anymore, they tell Ogre how you want to
        render the scene. You can't render to an RT without setting a compositor!</b>
    @par
        A workspace may be instantiated multiple times for different RTs, or using different cameras
        (i.e. multiple monitors, stereo 3D, etc), while they all will share the same definition.
        A workspace definition (@see CompositorWorkspaceDef) contains all the information needed
        by this CompositorWorkspace to instantiate and know which nodes to create and how to connect
        them. @See CompositorNodeDef
        A workspace may define global textures that are visible to all of its Node instances.
    @par
        If you want to have (e.g.) two monitors rendering the same but with different compositor
        effects, you will also have to create two different definitions (CompositorWorkspaceDefs)
    @par
        The intention of Ogre 2.0's compositor is to ideally have one Workspace per monitor (or per eye)
        while handling all RTTs inside that workspace with compositor nodes, local and global textures
        (including manually updated stuff like procedural textures, terrain's auto generated normal
        maps, etc); but it is also possible to do things the 1.x way: use additional Workspaces for
        manually rendering and controlling RTTs.
    @par
        Users with basic needs (no advanced effects) can @see CompositorManager2::createBasicWorkspaceDef
        for quickly having a basic definition set for them.
    @par
        For more information about Compositors, consult the Ogre 2.0 Porting Manual in the Docs folder.
    @author
        Matias N. Goldberg
    @version
        1.1
    */
    class _OgreExport CompositorWorkspace : public CompositorInstAlloc, public IdObject
    {
    protected:
        CompositorWorkspaceDef const *mDefinition;

        bool                    mValid;
        bool                    mEnabled;

        CompositorWorkspaceListener *mListener;

        /// Main sequence in the order they should be executed
        CompositorNodeVec       mNodeSequence;
        CompositorShadowNodeVec mShadowNodes;
        CompositorChannelVec    mGlobalTextures;
        Camera                  *mDefaultCamera; /// Could be null. @See CompositorManager2::addWorkspace
        SceneManager            *mSceneManager;
        RenderSystem            *mRenderSys;

        CompositorChannel       mRenderWindow;
        uint                    mCurrentWidth;
        uint                    mCurrentHeight;

        /// Creates all the node instances from our definition
        void createAllNodes(void);

        /// Destroys all node instances
        void destroyAllNodes(void);

        /** Connects all nodes' input and output channels (including final rt)
            according to our definition. Then creates the passes from all nodes
        @remarks
            Call this function after createAllNodes
        @param reconnect
            When true, assumes the node's passes have already been created and
            we're just connecting the channels again.
        */
        void connectAllNodes(void);

        void clearAllConnections(void);

        /** Setup ShadowNodes in every pass from every node so that we recalculate them as
            little as possible (when passes use SHADOW_NODE_FIRST_ONLY flag)
        @remarks
            Call this function after calling createPasses() on every node, since we
            need the passes to have been already created
        */
        void setupPassesShadowNodes(void);

    public:
        CompositorWorkspace( IdType id, const CompositorWorkspaceDef *definition,
                                const CompositorChannel &finalRenderTarget, SceneManager *sceneManager,
                                Camera *defaultCam, RenderSystem *renderSys, bool bEnabled );
        virtual ~CompositorWorkspace();

        const CompositorChannel& getGlobalTexture( IdString name ) const;

        /// Only valid workspaces can update without crashing
        bool isValid(void) const                            { return mValid; }

        void setEnabled( bool bEnabled )                    { mEnabled = bEnabled; }
        bool getEnabled() const                             { return mEnabled; }

        void setListener( CompositorWorkspaceListener *listener )   { mListener = listener; }
        CompositorWorkspaceListener* getListener(void) const        { return mListener; }

        /** Finds a node instance with the given aliased name
        @remarks
            Linear search O(N)
        @param aliasName
            Name of the node instance (they're unique)
        @param includeShadowNodes
            When true, also looks for ShadowNodes with that name, if the instance doesn't exists,
            it will not be created (default: false). @See findShadowNode
            When a Node has the same name of a Shadow Node, the Node takes precedence.
        @return
            Null if not found. Valid pointer otherwise.
        */
        CompositorNode* findNode( IdString aliasName, bool includeShadowNodes=false ) const;

        /** Destroys and recreates all nodes. TODO: Only revalidate nodes adjacent to those that
            were invalidated, to avoid recreating so many D3D/GL resources (local textures)
            which is important for GUI editors.
        */
        void recreateAllNodes(void);

        /** Reconnects all nodes. Use this function if you only altered the channel connections
            between nodes, but didn't add new ones or removed existing nodes.
        @remarks
            If there is a "loose node" (its inputs are not fully connected),
            disable it (@see CompositorNode::setEnabled)
        */
        void reconnectAllNodes(void);

        /** Call before _update unless the final render target is not a render window
        @param forceBeginFrame
            Forces a beginFrame call to the D3D9 API, even if the final render target is not
            a RenderWindow (not recommended). To avoid forcing extra begin/end frame pairs,
            update your manual workspaces inside @CompositorWorkspaceListener::workspacePreUpdate
            (performance optimization)
        */
        void _beginUpdate( bool forceBeginFrame );

        /// Updates the workspace's nodes.
        void _update(void);

        /** Call after _update unless the final render target is not a render window
        @param forceEndFrame
            @See _beginUpdate
            !!!WARNING!!! Forcing an end frame can cause API issues w/ D3D9 if Ogre had already
            issued a begin frame automatically (i.e. if you're calling from inside a RenderTarget
            or CompositorWorkspace listener). These API issues may not manifest on all HW/Driver
            combinations, making it hard to detect (if you're on D3D, use the Debug Runtimes)
        */
        void _endUpdate( bool forceEndFrame );

        /** In the case of RenderWindows, swaps/copies/flips the front with the back buffer.
            In the case of RenderTextures, resolves FSAA (unless it's tagged as explicit
            resolve, or its contents haven't changed since the last resolve)
        @remarks
            Call this after _endUpdate
        */
        void _swapFinalTarget(void);

        /** For compatibility with D3D9, forces a device lost check
            on the RenderWindow, so that BeginScene doesn't fail.
        */
        void _validateFinalTarget(void);

        /** Finds a shadow node instance with a given name.
            Note that unlike nodes, there can only be one ShadowNode instance per definition
            (in the same workspace)
        @remarks
            Performs a linear search O(N). There aren't many ShadowNodes active in a workspace
            to justify a better container (plus we mostly iterate through it).
        @param nodeDefName
            Name of the definition.
        @return
            ShadowNode pointer. Null if not found.
        */
        CompositorShadowNode* findShadowNode( IdString nodeDefName ) const;

        /** Finds a shadow node given it's definition name. If it doesn't exist, creates one.
            Note that unlike nodes, there can only be one ShadowNode instance per definition
            (in the same workspace)
        @remarks
            Performs a linear search O(N). There aren't many ShadowNodes active in a workspace
            to justify a better container (plus we mostly iterate through it).
        @par
            Throws if the shadow definition doesn't exist.
        @param nodeDefName
            Name of the definition.
        @param bCreated [out]
            Set to true if we had to create a new shadow node (it didn't exist)
        @return
            ShadowNode pointer
        */
        CompositorShadowNode* findOrCreateShadowNode( IdString nodeDefName, bool &bCreated );

        const CompositorNodeVec& getNodeSequence(void) const    { return mNodeSequence; }

        /// Finds a camera in the scene manager we have.
        Camera* findCamera( IdString cameraName ) const;

        /// Gets the default camera passed through mDefaultViewport.
        Camera* getDefaultCamera() const                    { return mDefaultCamera; }

        SceneManager* getSceneManager() const               { return mSceneManager; }

        /// Returns the RenderTarget we're rendering to. May be null.
        RenderTarget* getFinalTarget(void) const            { return mRenderWindow.target; }

        /// Gets the compositor manager (non const)
        CompositorManager2* getCompositorManager();

        /// Gets the compositor manager (const version)
        const CompositorManager2* getCompositorManager() const;

        size_t getFrameCount(void) const;
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
