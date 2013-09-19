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

#ifndef __CompositorManager2_H__
#define __CompositorManager2_H__

#include "OgreHeaderPrefix.h"
#include "OgreCompositorCommon.h"
#include "OgreIdString.h"

#include "OgreTexture.h"

namespace Ogre
{
	class Rectangle2D;
	typedef vector<TexturePtr>::type TextureVec;

	//class _OgreExport CompositorManager2 : public ResourceManager
	class _OgreExport CompositorManager2 : public ResourceAlloc
	{
		typedef map<IdString, CompositorNodeDef*>::type			CompositorNodeDefMap;
		CompositorNodeDefMap	mNodeDefinitions;

		typedef map<IdString, CompositorShadowNodeDef*>::type	CompositorShadowNodeDefMap;
		typedef vector<CompositorShadowNodeDef*>::type			CompositorShadowNodeDefVec;
		CompositorShadowNodeDefMap mShadowNodeDefs;
		CompositorShadowNodeDefVec mUnfinishedShadowNodes;

		typedef map<IdString, CompositorWorkspaceDef*>::type	CompositorWorkspaceDefMap;
		CompositorWorkspaceDefMap mWorkspaceDefs;

		typedef vector<CompositorWorkspace*>::type				WorkspaceVec;
		WorkspaceVec			mWorkspaces;

		size_t					mFrameCount;

		RenderSystem			*mRenderSystem;

		TextureVec				mNullTextureList;
		Rectangle2D				*mSharedTriangleFS;
		Rectangle2D				*mSharedQuadFS;

		void validateNodes(void);

	public:
		CompositorManager2();
		~CompositorManager2();

		/** The final rendering is done by passing the RenderWindow to one of the input
			channels. This functions does exactly that.
		*/
		void connectOutput( CompositorNode *finalNode, size_t inputChannel );

		bool hasNodeDefinition( IdString nodeDefName ) const;

		const CompositorNodeDef* getNodeDefinition( IdString nodeDefName ) const;

		CompositorNodeDef* addNodeDefinition( IdString name );

		const CompositorShadowNodeDef* getShadowNodeDefinition( IdString nodeDefName ) const;

		CompositorShadowNodeDef* addShadowNodeDefinition( IdString name );

		CompositorWorkspaceDef* addWorkspaceDefinition( IdString name );

		size_t getFrameCount(void) const					{ return mFrameCount; }

		/** Get an appropriately defined 'null' texture, i.e. one which will always
			result in no shadows.
		*/
		TexturePtr getNullShadowTexture( PixelFormat format );

		/** Returns a shared fullscreen rectangle/triangle useful for PASS_QUAD passes
		@remarks
			Pointer is valid throughout the lifetime of this CompositorManager2
		*/
		Rectangle2D* getSharedFullscreenTriangle(void) const		{ return mSharedTriangleFS; }
		/// @copydoc getSharedFullscreenTriangle
		Rectangle2D* getSharedFullscreenQuad(void) const			{ return mSharedQuadFS; }

		/**
		@param defaultVp
			Default viewport to use when a camera name wasn't specified explicitly in a
			pass definition. This pointer can be null if you promise to use all explicit
			camera names in your passes (and those cameras already exist)
		*/
		CompositorWorkspace* addWorkspace( SceneManager *sceneManager, RenderTarget *finalRenderTarget,
											Camera *defaultCam, IdString definitionName, bool bEnabled );

		/// Calls @see CompositorNode::_validateAndFinish on all objects who aren't yet validated
		void validateAllNodes();

		void _update( bool swapFinalTargets, bool waitForVSync );
		void _swapAllFinalTargets( bool waitForVSync );
	};
}

#include "OgreHeaderSuffix.h"

#endif
