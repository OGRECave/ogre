/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __CompositorInstance_H__
#define __CompositorInstance_H__

#include "OgrePrerequisites.h"
#include "OgreMaterial.h"
#include "OgreTexture.h"
#include "OgreRenderQueue.h"
#include "OgreCompositionTechnique.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	const size_t RENDER_QUEUE_COUNT = RENDER_QUEUE_MAX+1;       
            
    /** An instance of a Compositor object for one Viewport. It is part of the CompositorChain
		for a Viewport.
     */
	class _OgreExport CompositorInstance : public CompositorInstAlloc
    {
    public:
        CompositorInstance(CompositionTechnique *technique, CompositorChain *chain);
        virtual ~CompositorInstance();
		/** Provides an interface to "listen in" to to render system operations executed by this 
			CompositorInstance.
		*/
		class _OgreExport Listener
		{
		public:
			virtual ~Listener();

			/** Notification of when a render target operation involving a material (like
				rendering a quad) is compiled, so that miscellaneous parameters that are different
				per Compositor instance can be set up.
				@param pass_id	Pass identifier within Compositor instance, this is specified 
								by the user by CompositionPass::setIdentifier().
				@param mat		Material, this may be changed at will and will only affect
								the current instance of the Compositor, not the global material
								it was cloned from.
			 */
			virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);

			/** Notification before a render target operation involving a material (like
				rendering a quad), so that material parameters can be varied.
				@param pass_id	Pass identifier within Compositor instance, this is specified 
								by the user by CompositionPass::setIdentifier().
				@param mat		Material, this may be changed at will and will only affect
								the current instance of the Compositor, not the global material
								it was cloned from.
			 */
			virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);
		};
        /** Specific render system operation. A render target operation does special operations
		    between render queues like rendering a quad, clearing the frame buffer or 
			setting stencil state.
		*/
		class RenderSystemOperation : public CompositorInstAlloc
		{
		public:
			virtual ~RenderSystemOperation();
			/// Set state to SceneManager and RenderSystem
			virtual void execute(SceneManager *sm, RenderSystem *rs) = 0;
		};
		typedef map<int, MaterialPtr>::type QuadMaterialMap;
		typedef std::pair<int, RenderSystemOperation*> RenderSystemOpPair;
		typedef vector<RenderSystemOpPair>::type RenderSystemOpPairs;
        /** Operation setup for a RenderTarget (collected).
        */
        class TargetOperation
        {
        public:
            TargetOperation()
            { 
            }
            TargetOperation(RenderTarget *target):
                target(target), currentQueueGroupID(0), visibilityMask(0xFFFFFFFF),
                lodBias(1.0f),
                onlyInitial(false), hasBeenRendered(false), findVisibleObjects(false), 
				shadowsEnabled(true)
            { 
            }
            /// Target
            RenderTarget *target;

			/// Current group ID
			int currentQueueGroupID;

			/// RenderSystem operations to queue into the scene manager, by
			/// uint8
			RenderSystemOpPairs renderSystemOperations;

			/// Scene visibility mask
            /// If this is 0, the scene is not rendered at all
            uint32 visibilityMask;
            
            /// LOD offset. This is multiplied with the camera LOD offset
            /// 1.0 is default, lower means lower detail, higher means higher detail
            float lodBias;
            
            /** A set of render queues to either include or exclude certain render queues.
	 		*/
            typedef std::bitset<RENDER_QUEUE_COUNT> RenderQueueBitSet;

			/// Which renderqueues to render from scene
			RenderQueueBitSet renderQueues;
            
            /** @see CompositionTargetPass::mOnlyInitial
            */
            bool onlyInitial;
            /** "Has been rendered" flag; used in combination with
                onlyInitial to determine whether to skip this target operation.
            */
            bool hasBeenRendered;
            /** Whether this op needs to find visible scene objects or not 
            */
            bool findVisibleObjects;
			/** Which material scheme this op will use */
			String materialScheme;
			/** Whether shadows will be enabled */
			bool shadowsEnabled;
        };
        typedef vector<TargetOperation>::type CompiledState;
        
        /** Set enabled flag. The compositor instance will only render if it is
            enabled, otherwise it is pass-through.
        */
        void setEnabled(bool value);
        
        /** Get enabled flag.
        */
        bool getEnabled();

		/** Get the instance name for a local texture.
		@note It is only valid to call this when local textures have been loaded, 
			which in practice means that the compositor instance is active. Calling
			it at other times will cause an exception. Note that since textures
			are cleaned up aggressively, this name is not guaranteed to stay the
			same if you disable and re-enable the compositor instance.
		@param name The name of the texture in the original compositor definition
		@param mrtIndex If name identifies a MRT, which texture attachment to retrieve
		@returns The instance name for the texture, corresponds to a real texture
		*/
		const String& getTextureInstanceName(const String& name, size_t mrtIndex);

		/** Get the instance of a local texture.
		@note Textures are only valid when local textures have been loaded, 
			which in practice means that the compositor instance is active. Calling
			this method at other times will return null pointers. Note that since textures
			are cleaned up aggressively, this pointer is not guaranteed to stay the
			same if you disable and re-enable the compositor instance.
		@param name The name of the texture in the original compositor definition
		@param mrtIndex If name identifies a MRT, which texture attachment to retrieve
		@returns The texture pointer, corresponds to a real texture
		*/
		TexturePtr getTextureInstance(const String& name, size_t mrtIndex);

		/** Get the render target for a given render texture name. 
		@remarks
			You can use this to add listeners etc, but do not use it to update the
			targets manually or any other modifications, the compositor instance 
			is in charge of this.
		*/
		RenderTarget* getRenderTarget(const String& name);

       
        /** Recursively collect target states (except for final Pass).
            @param compiledState    This vector will contain a list of TargetOperation objects
        */
        virtual void _compileTargetOperations(CompiledState &compiledState);
        
        /** Compile the final (output) operation. This is done separately because this
            is combined with the input in chained filters.
        */
        virtual void _compileOutputOperation(TargetOperation &finalState);
        
        /** Get Compositor of which this is an instance
        */
        Compositor *getCompositor();
        
        /** Get CompositionTechnique used by this instance
        */
        CompositionTechnique *getTechnique();

		/** Change the technique we're using to render this compositor. 
		@param tech The technique to use (must be supported and from the same Compositor)
		@param reuseTextures If textures have already been created for the current
			technique, whether to try to re-use them if sizes & formats match.
		*/
		void setTechnique(CompositionTechnique* tech, bool reuseTextures = true);

		/** Pick a technique to use to render this compositor based on a scheme. 
		@remarks
			If there is no specific supported technique with this scheme name, 
			then the first supported technique with no specific scheme will be used.
			@see CompositionTechnique::setSchemeName
		@param schemeName The scheme to use 
		@param reuseTextures If textures have already been created for the current
			technique, whether to try to re-use them if sizes & formats match.
			Note that for this feature to be of benefit, the textures must have been created
			with the 'shared' option enabled.
		*/
		void setScheme(const String& schemeName, bool reuseTextures = true);

		/// Returns the name of the scheme this compositor is using
		const String& getScheme() const { return mActiveScheme; }

		/** Notify this instance that the primary surface has been resized. 
		@remarks
			This will allow the instance to recreate its resources that 
			are dependent on the size. 
		*/
		void notifyResized();


		/** Get Chain that this instance is part of
        */
        CompositorChain *getChain();

		/** Add a listener. Listeners provide an interface to "listen in" to to render system 
			operations executed by this CompositorInstance so that materials can be 
			programmatically set up.
			@see CompositorInstance::Listener
		*/
		void addListener(Listener *l);

		/** Remove a listener.
			@see CompositorInstance::Listener
		*/
		void removeListener(Listener *l);

		/** Notify listeners of a material compilation.
		*/
		void _fireNotifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);

		/** Notify listeners of a material render.
		*/
		void _fireNotifyMaterialRender(uint32 pass_id, MaterialPtr &mat);
	private:
        /// Compositor of which this is an instance
        Compositor *mCompositor;
        /// Composition technique used by this instance
        CompositionTechnique *mTechnique;
        /// Composition chain of which this instance is part
        CompositorChain *mChain;
        /// Is this instance enabled?
        bool mEnabled;
        /// Map from name->local texture
        typedef map<String,TexturePtr>::type LocalTextureMap;
        LocalTextureMap mLocalTextures;
		/// Store a list of MRTs we've created
		typedef map<String,MultiRenderTarget*>::type LocalMRTMap;
		LocalMRTMap mLocalMRTs;
		typedef map<CompositionTechnique::TextureDefinition*, TexturePtr>::type ReserveTextureMap;
		/** Textures that are not currently in use, but that we want to keep for now,
			for example if we switch techniques but want to keep all textures available
			in case we switch back. 
		*/
		ReserveTextureMap mReserveTextures;

		/// Vector of listeners
		typedef vector<Listener*>::type Listeners;
		Listeners mListeners;
        
        /// Previous instance (set by chain)
        CompositorInstance *mPreviousInstance;

		/// The scheme which is being used in this instance
		String mActiveScheme;
		
		/** Collect rendering passes. Here, passes are converted into render target operations
			and queued with queueRenderSystemOp.
        */
        virtual void collectPasses(TargetOperation &finalState, CompositionTargetPass *target);
        
        /** Create a local dummy material with one technique but no passes.
            The material is detached from the Material Manager to make sure it is destroyed
			when going out of scope.
        */
        MaterialPtr createLocalMaterial(const String& srcName);
        
        /** Create local rendertextures and other resources. Builds mLocalTextures.
        */
        void createResources(bool forResizeOnly);
        
        /** Destroy local rendertextures and other resources.
        */
        void freeResources(bool forResizeOnly, bool clearReserveTextures);

        /** Get RenderTarget for a named local texture.
        */
        RenderTarget *getTargetForTex(const String &name);
        
        /** Get source texture name for a named local texture.
		@param name The local name of the texture as given to it in the compositor
		@param mrtIndex For MRTs, which attached surface to retrieve
        */
        const String &getSourceForTex(const String &name, size_t mrtIndex = 0);

		/** Queue a render system operation.
			@returns destination pass
		 */
		void queueRenderSystemOp(TargetOperation &finalState, RenderSystemOperation *op);

		/// Util method for assigning a local texture name to a MRT attachment
		String getMRTTexLocalName(const String& baseName, size_t attachment);

		/** Search for options like AA and hardware gamma which we may want to 
			inherit from the main render target to which we're attached. 
		*/
		void deriveTextureRenderTargetOptions(const String& texname, 
			bool *hwGammaWrite, uint *fsaa, String* fsaaHint);
        
        friend class CompositorChain;
    };
	/** @} */
	/** @} */

}

#endif
