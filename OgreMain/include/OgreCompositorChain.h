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
#ifndef __CompositorChain_H__
#define __CompositorChain_H__

#include "OgrePrerequisites.h"
#include "OgreRenderTargetListener.h"
#include "OgreRenderQueueListener.h"
#include "OgreCompositorInstance.h"
#include "OgreViewport.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */
    /** Chain of compositor effects applying to one viewport.
    */
    class _OgreExport CompositorChain : public RenderTargetListener, public Viewport::Listener, public CompositorInstAlloc
    {
    public:
        CompositorChain(Viewport *vp);
        /** Another gcc warning here, which is no problem because RenderTargetListener is never used
            to delete an object.
        */
        virtual ~CompositorChain();
        
        /// Data types
        typedef std::vector<CompositorInstance*> Instances;
        typedef VectorIterator<Instances> InstanceIterator;
        
        enum {
            /// Identifier for best technique.
            BEST = 0,
            /// Identifier for "last" compositor in chain.
            LAST = (size_t)-1,
            NPOS = LAST
        };
        
        /** Apply a compositor. Initially, the filter is enabled.
        @param filter
            Filter to apply.
        @param addPosition
            Position in filter chain to insert this filter at; defaults to the end (last applied filter).
        @param scheme
            Scheme to use (blank means default).
        */
        CompositorInstance* addCompositor(const CompositorPtr& filter, size_t addPosition=LAST, const String& scheme = BLANKSTRING);

        /** Remove a compositor.
        @param position
            Position in filter chain of filter to remove; defaults to the end (last applied filter)
        */
        void removeCompositor(size_t position=LAST);
        
        /// @deprecated use getCompositorInstances
        OGRE_DEPRECATED size_t getNumCompositors();
        
        /** Remove all compositors.
        */
        void removeAllCompositors();
        
        /** Get compositor instance by name. Returns null if not found.
        */
        CompositorInstance* getCompositor(const String& name) const;

        /// @overload
        CompositorInstance *getCompositor(size_t index) const { return mInstances.at(index); }

        /// Get compositor position by name. Returns #NPOS if not found.
        size_t getCompositorPosition(const String& name) const;

        /** Get the original scene compositor instance for this chain (internal use). 
        */
        CompositorInstance* _getOriginalSceneCompositor(void) { return mOriginalScene; }

        /** The compositor instances. The first compositor in this list is applied first, the last one is applied last.
        */
        const Instances& getCompositorInstances() const { return mInstances; }

        /// @deprecated use getCompositorInstances
        OGRE_DEPRECATED InstanceIterator getCompositors();
    
        /** Enable or disable a compositor, by position. Disabling a compositor stops it from rendering
            but does not free any resources. This can be more efficient than using removeCompositor and 
            addCompositor in cases the filter is switched on and off a lot.
        @param position
            Position in filter chain of filter
        @param state enabled flag
        */
        void setCompositorEnabled(size_t position, bool state);

        void preRenderTargetUpdate(const RenderTargetEvent& evt) override;
        void postRenderTargetUpdate(const RenderTargetEvent& evt) override;
        void preViewportUpdate(const RenderTargetViewportEvent& evt) override;
        void postViewportUpdate(const RenderTargetViewportEvent& evt) override;
        void viewportCameraChanged(Viewport* viewport) override;
        void viewportDimensionsChanged(Viewport* viewport) override;
        void viewportDestroyed(Viewport* viewport) override;

        /** Mark state as dirty, and to be recompiled next frame.
        */
        void _markDirty();
        
        /** Get viewport that is the target of this chain
        */
        Viewport *getViewport();
        /** Set viewport that is the target of this chain
        */
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

        /** Get the previous instance in this chain to the one specified. 
        */
        CompositorInstance* getPreviousInstance(CompositorInstance* curr, bool activeOnly = true);
        /** Get the next instance in this chain to the one specified. 
        */
        CompositorInstance* getNextInstance(CompositorInstance* curr, bool activeOnly = true);

    private:
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

        String mOriginalSceneScheme;

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

        void createOriginalScene();
        void destroyOriginalScene();

        /// destroy internal resources
        void destroyResources(void);
        
        /** Internal method to get a unique name of a compositor
        */
        const String getCompositorName() const;

        /** Render queue listener used to set up rendering events. */
        class _OgreExport RQListener: public RenderQueueListener
        {
        public:
            RQListener() : mOperation(0), mSceneManager(0), mRenderSystem(0), mViewport(0) {}

            void renderQueueStarted(uint8 queueGroupId, const String& cameraName, bool& skipThisInvocation) override;

            /** Set current operation and target. */
            void setOperation(CompositorInstance::TargetOperation *op,SceneManager *sm,RenderSystem *rs);

            /** Notify current destination viewport. */
            void notifyViewport(Viewport* vp) { mViewport = vp; }

            /** Flush remaining render system operations. */
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
        /// Store old viewport material scheme
        String mOldMaterialScheme;
        /// Store old shadows enabled flag
        bool mOldShadowsEnabled;

    };
    /** @} */
    /** @} */
} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __CompositorChain_H__
