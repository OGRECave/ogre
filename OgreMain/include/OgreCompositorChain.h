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
#ifndef __CompositorChain_H__
#define __CompositorChain_H__

#include "OgrePrerequisites.h"
#include "OgreRenderTargetListener.h"
#include "OgreRenderQueueListener.h"
#include "OgreCompositorInstance.h"
#include "OgreCompositor.h"
namespace Ogre {
    
    /** Chain of compositor effects applying to one viewport.
     */
    class _OgreExport CompositorChain: public RenderTargetListener, public CompositorInstAlloc
    {
    public:
        CompositorChain(Viewport *vp);
        /** Another gcc warning here, which is no problem because RenderTargetListener is never used
            to delete an object.
            warning: `class Ogre::CompositorChain' has virtual functions but non-virtual destructor
        */
        virtual ~CompositorChain();
        
        /// Data types
        typedef std::vector<CompositorInstance*> Instances;
        typedef VectorIterator<Instances> InstanceIterator;
        
        /// Identifier for "last" compositor in chain
        static const size_t LAST = (size_t)-1;
        /// Identifier for best technique
        static const size_t BEST = 0;
        
        /** Apply a compositor. Initially, the filter is enabled.
        @param filter     Filter to apply
        @param addPosition    Position in filter chain to insert this filter at; defaults to the end (last applied filter)
        @param technique      Technique to use; CompositorChain::BEST (default) chooses to the best one 
                            available (first technique supported)
        */
        CompositorInstance* addCompositor(CompositorPtr filter, size_t addPosition=LAST, size_t technique=BEST);
    
        /** Remove a compositor.
        @param position    Position in filter chain of filter to remove; defaults to the end (last applied filter)
        */
        void removeCompositor(size_t position=LAST);
        
        /** Get the number of compositors.
        */
        size_t getNumCompositors();
        
        /** Remove all compositors.
        */
        void removeAllCompositors();
        
        /** Get compositor instance by position.
         */
        CompositorInstance *getCompositor(size_t index);

		/** Get the original scene compositor instance for this chain (internal use). 
		*/
		CompositorInstance* _getOriginalSceneCompositor(void) { return mOriginalScene; }
    
        /** Get an iterator over the compositor instances. The first compositor in this list is applied first, the last one is applied last.
        */
        InstanceIterator getCompositors();
    
        /** Enable or disable a compositor, by position. Disabling a compositor stops it from rendering
            but does not free any resources. This can be more efficient than using removeCompositor and 
			addCompositor in cases the filter is switched on and off a lot.
        @param position    Position in filter chain of filter
        */
        void setCompositorEnabled(size_t position, bool state);
    
        /** @see RenderTargetListener::preRenderTargetUpdate */
		virtual void preRenderTargetUpdate(const RenderTargetEvent& evt);
		/** @see RenderTargetListener::preViewportUpdate */
        virtual void preViewportUpdate(const RenderTargetViewportEvent& evt);
        /** @see RenderTargetListener::postViewportUpdate */
        virtual void postViewportUpdate(const RenderTargetViewportEvent& evt);
		/** @see RenderTargetListener::viewportRemoved */
		virtual void viewportRemoved(const RenderTargetViewportEvent& evt);
        
        /** Mark state as dirty, and to be recompiled next frame.
         */
        void _markDirty();
        
        /** Get viewport that is the target of this chain
         */
        Viewport *getViewport();

		/** Internal method for reconnecting with viewport */
		void _notifyViewport(Viewport* vp);

		/** Remove a compositor by pointer. This is internally used by CompositionTechnique to
			"weak" remove any instanced of a deleted technique.
        */
        void _removeInstance(CompositorInstance *i);

		/** Internal method for registering a queued operation for deletion later **/
		void _queuedOperation(CompositorInstance::RenderSystemOperation* op);

		/** Compile this Composition chain into a series of RenderTarget operations.
		*/
		void _compile();
    protected:    
        /// Viewport affected by this CompositorChain
        Viewport *mViewport;
        
        /** Plainly renders the scene; implicit first compositor in the chain.
        */
        CompositorInstance *mOriginalScene;
        
        /// Postfilter instances in this chain
        Instances mInstances;
        
        /// State needs recompile
        bool mDirty;
		/// Any compositors enabled?
		bool mAnyCompositorsEnabled;

        /// Compiled state (updated with _compile)
        CompositorInstance::CompiledState mCompiledState;
        CompositorInstance::TargetOperation mOutputOperation;
		/// Render System operations queued by last compile, these are created by this
		/// instance thus managed and deleted by it. The list is cleared with 
		/// clearCompilationState()
		typedef std::vector<CompositorInstance::RenderSystemOperation*> RenderSystemOperations;
		RenderSystemOperations mRenderSystemOperations;

        
		/** Clear compiled state */
		void clearCompiledState();
        
        /** Prepare a viewport, the camera and the scene for a rendering operation
         */
        void preTargetOperation(CompositorInstance::TargetOperation &op, Viewport *vp, Camera *cam);
        
        /** Restore a viewport, the camera and the scene after a rendering operation
         */
        void postTargetOperation(CompositorInstance::TargetOperation &op, Viewport *vp, Camera *cam);

		/// destroy internal resources
		void destroyResources(void);

		/** Render queue listener used to set up rendering events. */
		class _OgreExport RQListener: public RenderQueueListener
		{
		public:
			/** @copydoc RenderQueueListener::renderQueueStarted
			*/
			virtual void renderQueueStarted(uint8 id, const String& invocation, bool& skipThisQueue);
			/** @copydoc RenderQueueListener::renderQueueEnded
			*/
			virtual void renderQueueEnded(uint8 id, const String& invocation, bool& repeatThisQueue);

			/** Set current operation and target */
			void setOperation(CompositorInstance::TargetOperation *op,SceneManager *sm,RenderSystem *rs);

			/** Notify current destination viewport  */
			void notifyViewport(Viewport* vp) { mViewport = vp; }

			/** Flush remaining render system operations */
			void flushUpTo(uint8 id);
		private:
			CompositorInstance::TargetOperation *mOperation;
			SceneManager *mSceneManager;
			RenderSystem *mRenderSystem;
			Viewport* mViewport;
			CompositorInstance::RenderSystemOpPairs::iterator currentOp, lastOp;
		};
		RQListener mOurListener;
		/// Old viewport settings
		unsigned int mOldClearEveryFrameBuffers;
		/// Store old scene visibility mask
		uint32 mOldVisibilityMask;
		/// Store old find visible objects
		bool mOldFindVisibleObjects;
        /// Store old camera LOD bias      
        float mOldLodBias;     
		///	Store old viewport material scheme
		String mOldMaterialScheme;
		/// Store old shadows enabled flag
		bool mOldShadowsEnabled;

    };
}

#endif
