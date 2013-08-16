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

#ifndef __CompositorNode_H__
#define __CompositorNode_H__

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreCompositorCommon.h"
#include "OgreIdString.h"

namespace Ogre
{
	class CompositorNodeDef;

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/

	/** Compositor nodes are the core subject of compositing.
		This is an instantiation. All const, shared parameters are in the definition
		(CompositorNodeDef) and we assume they don't change throughout the lifetime
		of our instance.
	@par
		The textures in mLocalTextures are managed by us and we're responsible for
		freeing them when they're no longer needed.
	@par
		Before nodes can be used, they have to be connected between each other,
		followed by a call to routeOutputs()
		Connections must be done in a very specific order, so let the manager
		take care of solving the dependencies. Basically the problem is
		that if the chain is like this: A -> B -> C; if we connect node
		B to C first, then there's a chance of giving null pointers to C
		instead of the valid ones that belong to A.
	@par
		To solve this problem, we first start with nodes that have no input,
		and then continue with those who have all of their input set; then repeat
		until there are no nodes to be processed.
		If there's still nodes with input left open; then those nodes can't be
		activated and the workspace is invalid.
	@par
		No Node can be valid if it has disconnected input channels left.
		Nodes can have no input because they either use passes that don't need it
		(eg. scene pass) or use global textures as means for sharing their work
		Similarly, Nodes may have no output because they use global textures.
	@par
		Nodes with feedback loops are not supported and may or may not work.
		A feedback loop is when A's output is used in B, B to C, then
		C is plugged back into A.
	@par
		It's possible to assign the same output to two different input channels,
		though it could work very unintuitively... (because two textures that may
		be intended to be hard copies are actually sharing the same memory)
	@remarks
		We own the local textures, so it's our job to destroy them
	@author
		Matias N. Goldberg
    @version
        1.0
    */
	class _OgreExport CompositorNode : public CompositorInstAlloc, public IdObject
	{
	protected:
		typedef vector<CompositorChannel>::type CompositorChannelVec;

		/// Must be <= mInTextures.size(). Tracks how many pointers are not null in mInTextures
		size_t					mNumConnectedInputs;
		CompositorChannelVec	mInTextures;
		CompositorChannelVec	mLocalTextures;

		/// Contains pointers that are ither in mInTextures or mLocalTextures
		CompositorChannelVec	mOutTextures;

		CompositorPassVec	mPasses;

		/// Nodes we're connected to. If we destroy our local textures, we need to inform them
		CompositorNodeVec	mConnectedNodes;

		RenderSystem		*mRenderSystem; /// Used to create/destroy MRTs

		/** Fills mOutTextures with the pointers from mInTextures & mLocalTextures according
			to CompositorNodeDef::mOutChannelMapping. Call this immediately after modifying
			mInTextures or mLocalTextures
		*/
		void routeOutputs();

		/** Disconnects this node's output from all nodes we send our textures to. We only
			disconnect local textures.
		@remarks
			Textures that we got from our input channel and then pass it to the output channel
			are left untouched. This allows for some node to be plugged back & forth without
			making much mess and leaving everything else working.
		*/
		void disconnectOutput();

		/** Call this function when caller has destroyed a RenderTarget in which the callee
			may have a reference to that pointer, so that we can clean it up.
		@param channel
			Channel containing the pointer about to be destroyed (must still be valid)
		*/
		void notifyDestroyed( const CompositorChannel &channel );

	public:
		CompositorNode( IdType id, const CompositorNodeDef *definition, RenderSystem *renderSys );
		CompositorNode( IdType id, const CompositorNodeDef *definition,
						RenderSystem *renderSys, const RenderTarget *finalTarget );
		virtual ~CompositorNode();

		/** Connects this node (let's call it node 'A') to node 'B', mapping the output
			channels from A into the input channels from B
		@param outChannelsA
			Output to use from node A. Container must be same size as inChannelsB
		@param inChannelsB
			Input to connect the output from A. Container must be same size as outChannelsA
		*/
		void connectTo( const ChannelVec &outChannelsA, CompositorNode *nodeB,
						const ChannelVec &inChannelsB );

	private:
		CompositorNodeDef const *mDefinition;
	};

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
