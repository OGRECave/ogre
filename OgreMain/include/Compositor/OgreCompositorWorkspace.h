/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#ifndef __CompositorWorkspace_H__
#define __CompositorWorkspace_H__

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

	/** TODO: Describe!!!
	@author
		Matias N. Goldberg
    @version
        1.0
    */
	class _OgreExport CompositorWorkspace : public CompositorInstAlloc, public IdObject
	{
	protected:
		CompositorWorkspaceDef const *mDefinition;

		bool					mValid;
		bool					mEnabled;

		/// Main sequence in the order they should be executed
		CompositorNodeVec		mNodeSequence;
		CompositorShadowNodeVec	mShadowNodes;
		CompositorChannelVec	mGlobalTextures;
		Camera					*mDefaultCamera; /// Could be null. @See CompositorManager2::addWorkspace
		SceneManager			*mSceneManager;
		RenderSystem			*mRenderSys;

		RenderTarget			*mRenderWindow;
		uint					mCurrentWidth;
		uint					mCurrentHeight;

		/// Creates all the node instances from our definition
		void createAllNodes(void);

		/// Destroys all node instances
		void destroyAllNodes(void);

		/** Connects all nodes' input and output channels (including final rt)
			according to our definition. Then creates the passes from all nodes
		@remarks
			Call this function after createAllNodes
		*/
		void connectAllNodes(void);

		/** Setup ShadowNodes in every pass from every node so that we recalculate them as
			little as possible (when passes use SHADOW_NODE_FIRST_ONLY flag)
		@remarks
			Call this function after calling createPasses() on every node, since we
			need the passes to have been already created
		*/
		void setupPassesShadowNodes(void);

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

	public:
		CompositorWorkspace( IdType id, const CompositorWorkspaceDef *definition,
								RenderTarget *finalRenderTarget, SceneManager *sceneManager,
								Camera *defaultCam, RenderSystem *renderSys, bool bEnabled );
		virtual ~CompositorWorkspace();

		const CompositorChannel& getGlobalTexture( IdString name ) const;

		/// Only valid workspaces can update without crashing
		bool isValid(void) const							{ return mValid; }

		void setEnabled( bool bEnabled )					{ mEnabled = bEnabled; }
		bool getEnabled() const								{ return mEnabled; }

		/** Destroys and recreates all nodes. TODO: Only revalidate nodes adjacent to those that
			were invalidated, to avoid recreating so many D3D/GL resources (local textures)
			which is important for GUI editors.
		*/
		void revalidateAllNodes(void);

		void _update( bool swapFinalTargets, bool waitForVSync );
		void _swapFinalTarget( bool waitForVSync );

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

		/// Finds a camera in the scene manager we have.
		Camera* findCamera( IdString cameraName ) const;

		/// Gets the default camera passed through mDefaultViewport.
		Camera* getDefaultCamera() const					{ return mDefaultCamera; }

		SceneManager* getSceneManager() const				{ return mSceneManager; }

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
