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

		void createAllNodes(void);
		void destroyAllNodes(void);
		void connectAllNodes(void);

		/// Linear search. Returns null if not found.
		CompositorNode* findNode( IdString aliasName ) const;

	public:
		CompositorWorkspace( IdType id, const CompositorWorkspaceDef *definition,
								RenderTarget *finalRenderTarget, SceneManager *sceneManager,
								Camera *defaultCam, RenderSystem *renderSys, bool bEnabled );
		~CompositorWorkspace();

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

		void _update(void);

		//TODO
		CompositorShadowNode* findShadowNode( IdString nodeName ) const	{ return 0; }

		/// Finds a camera in the scene manager we have.
		Camera* findCamera( IdString cameraName ) const;

		/// Gets the default camera passed through mDefaultViewport.
		Camera* getDefaultCamera() const					{ return mDefaultCamera; }
	};

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
