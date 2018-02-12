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

#ifndef __CompositorManager2_H__
#define __CompositorManager2_H__

#include "OgreHeaderPrefix.h"
#include "OgreCompositorCommon.h"
#include "OgreIdString.h"
#include "OgreResourceTransition.h"
#include "OgreVector4.h"
#include "Compositor/OgreCompositorChannel.h"

#include "OgreTexture.h"

namespace Ogre
{
    namespace v1
    {
        class Rectangle2D;
    }
    class CompositorPassProvider;

    typedef vector<TexturePtr>::type TextureVec;
    typedef vector<UavBufferPacked*>::type UavBufferPackedVec;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    /** Main system for managing Render Targets through the use of nodes. All applications
        must at least define a workspace definition and create a workspace instance in order
        to start rendering.
    @remarks
        The CompositorManager2 works by defining definitions which tell how the instance will
        behave.
        The top down view is the following:
            * Workspace
                * Node
                    * Target
                        * PASS_SCENE
                        * PASS_QUAD
                        * PASS_CLEAR 
                        * PASS_STENCIL
                        * PASS_RESOLVE
                * Shadow Node
        A Node definition must be created first. Inside the Node Def. different passes can be defined
        including which targets they should render to.
        Once the definitions are set, a workspace instance must be created using addWorkspace
        and rendering will start automatically.
        Each definition is shared by all instances, and is assumed to be immutable (read only) for
        the life time of those objects.
    @par
        If you wish to change the definitions, you should destroy all instances first. In theory
        many changes can actually happen in real time without any harm, but that depends on how
        the code was written and thus the behavior is undefined.
    @par
        A node has inputs (textures), local textures, and outputs. It can also directly global textures
        that are defined in a workspace definition. There a few basic rules:
            * Global Textures use the "global_" prefix. For example "global_myRT" is a global texture.
              Trying to create a Local texture with that name will throw.
            * Global Textures can't be used as node input nor output.
            * Textures that came as Input can be used as Output.
            * A node may have no Input nor Output.
            * Shadow Nodes can't have input, but can have output to be used with other nodes.
    @par
        Shadow Nodes are particular case of Nodes which are used for rendering shadow maps, and can
        only be references from a PASS_SCENE object; and will be executed when that pass is.
        After the pass is executed, its output can be used for other regular Nodes (i.e. for
        postprocessing), which enables the possibility of easily creating RSM (Reflective Shadow
        Maps) for Global Illumination calculations.
    @par
        For more information @see CompositorNode & @see CompositorShadowNode
    */
    class _OgreExport CompositorManager2 : public ResourceAlloc
    {
    public:
        typedef map<IdString, CompositorNodeDef*>::type         CompositorNodeDefMap;

    protected:
        struct QueuedWorkspace
        {
            CompositorWorkspace *workspace;
            int position;
            QueuedWorkspace( CompositorWorkspace *_workspace, int _position ) :
                workspace( _workspace ), position( _position ) {}
        };

        CompositorNodeDefMap    mNodeDefinitions;

        typedef map<IdString, CompositorShadowNodeDef*>::type   CompositorShadowNodeDefMap;
        typedef vector<CompositorShadowNodeDef*>::type          CompositorShadowNodeDefVec;
        CompositorShadowNodeDefMap mShadowNodeDefs;
        CompositorShadowNodeDefVec mUnfinishedShadowNodes;

        typedef map<IdString, CompositorWorkspaceDef*>::type    CompositorWorkspaceDefMap;
        CompositorWorkspaceDefMap mWorkspaceDefs;

        typedef vector<CompositorWorkspace*>::type              WorkspaceVec;
        typedef vector<QueuedWorkspace>::type                   QueuedWorkspaceVec;
        typedef vector<CompositorWorkspaceListener*>::type      CompositorWorkspaceListenerVec;
        WorkspaceVec            mWorkspaces;
        /// All workspaces created via addWorkspace are first stored in this
        /// container, to prevent corrupting mWorkspaces' iterators in
        /// case we happen to be adding a workspace while inside the
        /// workspace's update loop (i.e. in a listener)
        QueuedWorkspaceVec      mQueuedWorkspaces;
        /// Listeners to call CompositorWorkspaceListener::allWorkspacesBeginUpdate
        CompositorWorkspaceListenerVec mListeners;

        size_t                  mFrameCount;

        RenderSystem            *mRenderSystem;

        TextureVec              mNullTextureList;
        v1::Rectangle2D         *mSharedTriangleFS;
        v1::Rectangle2D         *mSharedQuadFS;
        ObjectMemoryManager     *mDummyObjectMemoryManager;

        /// For custom passes.
        CompositorPassProvider  *mCompositorPassProvider;

        void addQueuedWorkspaces(void);

    public:
        CompositorManager2( RenderSystem *renderSystem );
        ~CompositorManager2();

        /** The final rendering is done by passing the RenderWindow to one of the input
            channels. This functions does exactly that.
        */
        void connectOutput( CompositorNode *finalNode, size_t inputChannel );

        /// Returns true if a node definition with the given name exists
        bool hasNodeDefinition( IdString nodeDefName ) const;

        /// Returns the node definition with the given name. Throws if not found
        const CompositorNodeDef* getNodeDefinition( IdString nodeDefName ) const;

        /// @See getNodeDefinition. Returns a non-const pointer. Use this only if you
        /// know what you're doing. Modifying a NodeDef while it's being used by
        /// CompositorNode instances is undefined. It's safe if you're sure it's not
        /// being used.
        CompositorNodeDef* getNodeDefinitionNonConst( IdString nodeDefName ) const;

        /// Returns a const iterator to all existing definitions
        const CompositorNodeDefMap& getNodeDefinitions(void) const  { return mNodeDefinitions; }

        /// Returns a new node definition. The name must be unique, throws otherwise.
        CompositorNodeDef* addNodeDefinition( const String &name );

        /// Removes the node definition with the given name. Throws if not found
        void removeNodeDefinition( IdString nodeDefName );

        /// Returns true if a shadow node definition with the given name exists
        bool hasShadowNodeDefinition( IdString nodeDefName ) const;

        /// Returns the node definition with the given name. Throws if not found
        const CompositorShadowNodeDef* getShadowNodeDefinition( IdString nodeDefName ) const;

        /// @See getShadowNodeDefinition. Returns a non-const pointer. Use this only if you
        /// know what you're doing. Modifying a ShadowNodeDef while it's being used by
        /// CompositorShadowNode instances is undefined. It's safe if you're sure it's not
        /// being used.
        CompositorShadowNodeDef* getShadowNodeDefinitionNonConst( IdString nodeDefName ) const;

        /// Returns a new node definition. The name must be unique, throws otherwise.
        CompositorShadowNodeDef* addShadowNodeDefinition( const String &name );

        /// Removes the node definition with the given name. Throws if not found
        void removeShadowNodeDefinition( IdString nodeDefName );

        /// Returns true if a workspace definition with the given name exists
        bool hasWorkspaceDefinition( IdString name ) const;

        /// Returns the workspace definition with the given name. Throws if not found
        CompositorWorkspaceDef* getWorkspaceDefinition( IdString name ) const;
        CompositorWorkspaceDef* getWorkspaceDefinitionNoThrow( IdString name ) const;

        /** Returns a new workspace definition. The name must be unique, throws otherwise.
        @remarks
            Setting workspace def's connections must be done *after* all node
            definitions have been created
        */
        CompositorWorkspaceDef* addWorkspaceDefinition( const String& name );

        /// Removes the workspace definition with the given name. Throws if not found
        void removeWorkspaceDefinition( IdString name );

        /// Returns how many times _update has been called.
        size_t getFrameCount(void) const                    { return mFrameCount; }

        /** Get an appropriately defined 'null' texture, i.e. one which will always
            result in no shadows.
        */
        TexturePtr getNullShadowTexture( PixelFormat format );

        /** Returns a shared fullscreen rectangle/triangle useful for PASS_QUAD passes
        @remarks
            Pointer is valid throughout the lifetime of this CompositorManager2
        */
        v1::Rectangle2D* getSharedFullscreenTriangle(void) const    { return mSharedTriangleFS; }
        /// @copydoc getSharedFullscreenTriangle
        v1::Rectangle2D* getSharedFullscreenQuad(void) const        { return mSharedQuadFS; }

        /** Main function to start rendering. Creates a workspace instance based on a
            workspace definition.
        @remarks
            When rendering in stereo using split screen (or when the screen is split
            to support multiplayer) you can use viewportModifier, vpModifierMask and
            executionMask parameters to control how the entire workspace renders to
            a portion of the screen while using two (or more) workspaces without
            having to duplicate the nodes just to alter their viewport parameters.

            viewportModifier controls how to stretch the viewport in each pass,
            vpModifierMask controls which passes will ignore the stretching, and
            executionMask controls which passes get skipped.

            All passes have a default executionMask = 0xFF vpModifierMask = 0xFF
            except for clear passes which default to
            executionMask = 0x01 vpModifierMask = 0x00

            The reasoning behind this is that often you want to clear the whole renderTarget
            the first time (it's GPU-friendly to discard the entire buffer; aka
            vpModifierMask = 0), but the second time you don't want the clear to be executed
            at all to prevent overwritting the contents from the first pass (executionMask = 1).
        @par

            Example, stereo (split screen):
            //Render Eye0 to the left side of the screen
            m_workspaceEye0 = mgr->addWorkspace( sceneManager, renderTarget,
                                                 eyeCamera0, "MainWorkspace", true,
                                                 -1, Vector4( 0, 0, 0.5f, 1 ),
                                                 0x01, 0x01 );
            //Render Eye1 to the right side of the screen
            m_workspaceEye1 = mgr->addWorkspace( sceneManager, renderTarget,
                                                 eyeCamera1, "MainWorkspace", true,
                                                 -1, Vector4( 0.5f, 0, 0.5f, 1 ),
                                                 0x02, 0x02 );
        @par

            Example, split screen, multiplayer, 4 players (e.g. Mario Kart (R)-like games)
            for( int i=0; i<4; ++i )
            {
                Vector4 vpModifier( (i % 2) * 0.5f, (i >> 1) * 0.5f, 0.25f, 0.25f );
                m_workspace[i] = mgr->addWorkspace( sceneManager, renderTarget,
                                                    playerCam[i], "MainWorkspace", true,
                                                    -1, vpModifier,
                                                    (1 << i), (1 << i) );
            }

        @param sceneManager
            The SceneManager this workspace will be associated with. You can have multiple
            scene managers, each with multiple workspaces. Those workspaces can be set to
            render to the same final render target, regardless of scene manager (or not).
        @param finalRenderTarget
            The final RT where the workspace will be rendered to. Usually the RenderWindow.
            We need this pointer in order to correctly create RTTs that depend on
            the final target's width, height, gamma & fsaa settings.
            This pointer will be used for "connectOutput" channels
            (@see CompositorWorkspaceDef::connectOutput)
            In theory if none of your nodes use width & height relative to final RT &
            you don't use connectOutput, this pointer could be null. Although it's not
            recommended nor explicitly supported.
        @param defaultCam
            Default camera to use when a camera name wasn't specified explicitly in a
            pass definition (i.e. PASS_SCENE passes). This pointer can be null if you
            promise to use all explicit camera names in your passes (and those cameras
            have already been created)
        @param definitionName
            The unique name of the workspace definition
        @param bEnabled
            True if this workspace should start enabled, false otherwise.
        @param position
            If there are multiple workspaces, specifies the order in which this compositor
            should be updated. i.e. "0" means this new workspace gets updated first. Note
            that subsequent calls will place other workspaces to be updated first.
            Typically you will want to update workspace that renders to the RenderWindow
            last (depending on what you do with RTs, some OSes, like OS X, may not like
            it).
            Defaults to -1; which means update last.
        @param uavBuffers
            Array of UAV Buffers that will be exposed to compositors, via the
            'connect_buffer_external' script keyword, or the call
            CompositorWorkspaceDef::connectExternalBuffer
        @param vpOffsetScale
            The viewport of every pass from every node will be offseted and scaled by
            the following formula:
                left    += vpOffsetScale.x;
                top     += vpOffsetScale.y;
                width   *= vpOffsetScale.z;
                height  *= vpOffsetScale.w;
            This affects both the viewport dimensions as well as the scissor rect.
        @param vpModifierMask
            An 8-bit mask that will be AND'ed with the viewport modifier mask of each pass
            from every node. When the result is zero, the previous parameter "viewportModifier"
            isn't applied to that pass.

            This is useful when you want to apply a pass (like Clear) to the whole render
            target and not just to the scaled region.
        @param executionMask
            An 8-bit mask that will be AND'ed with the execution mask of each pass from
            every node. When the result is zero, the pass isn't executed. See remarks
            on how to use this for efficient Stereo or split screen.

            This is useful when you want to skip a pass (like Clear) when rendering the second
            eye (or the second split from the second player).
        */
        CompositorWorkspace* addWorkspace( SceneManager *sceneManager, RenderTarget *finalRenderTarget,
                                           Camera *defaultCam, IdString definitionName, bool bEnabled,
                                           int position=-1, const UavBufferPackedVec *uavBuffers=0,
                                           const ResourceLayoutMap* initialLayouts=0,
                                           const ResourceAccessMap* initialUavAccess=0,
                                           const Vector4 &vpOffsetScale = Vector4::ZERO,
                                           uint8 vpModifierMask=0x00, uint8 executionMask=0xFF );

        /// Overload that allows a full RenderTexture to be used as render target (see CubeMapping demo)
        CompositorWorkspace* addWorkspace( SceneManager *sceneManager,
                                           const CompositorChannelVec &externalRenderTargets,
                                           Camera *defaultCam, IdString definitionName, bool bEnabled,
                                           int position=-1, const UavBufferPackedVec *uavBuffers=0,
                                           const ResourceLayoutMap* initialLayouts=0,
                                           const ResourceAccessMap* initialUavAccess=0,
                                           const Vector4 &vpOffsetScale = Vector4::ZERO,
                                           uint8 vpModifierMask=0x00, uint8 executionMask=0xFF );

        /// Removes the given workspace. Pointer is no longer valid after this call
        void removeWorkspace( CompositorWorkspace *workspace );

        /// Removes all workspaces. Make sure you don't hold any reference to a CompositorWorkpace!
        void removeAllWorkspaces(void);
        void removeAllWorkspaceDefinitions(void);

        size_t getNumWorkspaces(void) const                         { return mWorkspaces.size(); }

        /** Removes all shadow nodes defs. Make sure there are no active nodes using the definition!
        @remarks
            Call removeAllWorkspaceDefinitions first
        */
        void removeAllShadowNodeDefinitions(void);

        /** Removes all node defs. Make sure there are no active nodes using the definition!
        @remarks
            Call removeAllWorkspaceDefinitions first
        */
        void removeAllNodeDefinitions(void);

        /// Calls @see CompositorNode::_validateAndFinish on all objects who aren't yet validated
        void validateAllNodes();

        /// Will call the renderSystem which in turns calls _updateImplementation
        void _update( SceneManagerEnumerator &sceneManagers, HlmsManager *hlmsManager );
        
        /// This should be called by the render system to
        /// perform the actual compositor manager update.
        /// DO NOT CALL THIS DIRECTLY.
        void _updateImplementation( SceneManagerEnumerator &sceneManagers, HlmsManager *hlmsManager );

        void _swapAllFinalTargets(void);

        /** Utility helper to create a basic workspace to get you out of the rush. Advanced users will
            probably prefer to create the workspace definition using scripts or manipulating functions
            directly
        @param workspaceDefName
            Name to give to the workspace definition. Must be unique
        @param backgroundColour
            Clear colour
        @param shadowNodeName
            Name of the shadow node. Leave blank if no shadows.
            Caller is supposed to have set the shadow node correctly
        */
        void createBasicWorkspaceDef( const String &workspaceDefName,
                                        const ColourValue &backgroundColour,
                                        IdString shadowNodeName=IdString() );

        /// Sets a custom pass provider in order to implement custom passes in
        /// your nodes. @see CompositorPassProvider
        void setCompositorPassProvider( CompositorPassProvider *passProvider );
        CompositorPassProvider* getCompositorPassProvider(void) const;

        void addListener( CompositorWorkspaceListener *listener );
        void removeListener( CompositorWorkspaceListener *listener );
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
