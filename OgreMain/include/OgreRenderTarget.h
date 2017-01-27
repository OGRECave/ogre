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
#ifndef __RenderTarget_H__
#define __RenderTarget_H__

#include "OgrePrerequisites.h"

#include "OgrePixelFormat.h"
#include "OgreResourceTransition.h"
#include "OgreHeaderPrefix.h"

/* Define the number of priority groups for the render system's render targets. */
#ifndef OGRE_NUM_RENDERTARGET_GROUPS
    #define OGRE_NUM_RENDERTARGET_GROUPS 10
    #define OGRE_DEFAULT_RT_GROUP 4
    #define OGRE_REND_TO_TEX_RT_GROUP 2
#endif

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */
    /** A 'canvas' which can receive the results of a rendering
        operation.
        @remarks
            This abstract class defines a common root to all targets of rendering operations. A
            render target could be a window on a screen, or another
            offscreen surface like a texture or bump map etc.
        @author
            Steven Streeting
        @version
            1.0
     */
    class _OgreExport RenderTarget : public GpuResource, public RenderSysAlloc
    {
    public:
        enum StatFlags
        {
            SF_NONE           = 0,
            SF_FPS            = 1,
            SF_AVG_FPS        = 2,
            SF_BEST_FPS       = 4,
            SF_WORST_FPS      = 8,
            SF_TRIANGLE_COUNT = 16,
            SF_ALL            = 0xFFFF
        };

        struct FrameStats
        {
            size_t triangleCount;
            size_t batchCount;
            int vBlankMissCount; // -1 means that the value is not applicable
        };

        enum FrameBuffer
        {
            FB_FRONT,
            FB_BACK,
            FB_AUTO
        };

        RenderTarget();
        virtual ~RenderTarget();

        /// Retrieve target's name.
        virtual const String& getName(void) const;

        /// Retrieve information about the render target.
        virtual void getMetrics(unsigned int& width, unsigned int& height, unsigned int& colourDepth);

        virtual uint32 getWidth(void) const;
        virtual uint32 getHeight(void) const;
        PixelFormat getFormat(void) const;

        virtual void getFormatsForPso( PixelFormat outFormats[OGRE_MAX_MULTIPLE_RENDER_TARGETS],
                                       bool outHwGamma[OGRE_MAX_MULTIPLE_RENDER_TARGETS] ) const;

        /**
         * Sets the pool ID this RenderTarget should query from. Default value is POOL_DEFAULT.
         * Set to POOL_NO_DEPTH to avoid using a DepthBuffer (or manually controlling it) @see DepthBuffer
         *  @remarks
         *      Changing the pool Id will cause the current depth buffer to be detached unless the old
         *      id and the new one are the same
         */
        virtual void setDepthBufferPool( uint16 poolId );

        /// Returns the pool ID this RenderTarget should query from. @see DepthBuffer
        uint16 getDepthBufferPool() const;

        /** Whether this RT should be attached to a depth texture, or a regular depth buffer.
        @remarks
            On older GPUs, preferring depth textures may result in certain depth precisions
            to not be available (or use integer precision instead of floating point, etc).
        @par
            Changing this setting will cause the current depth buffer to be detached unless
            the old and the setting are the same.
        @param preferDepthTexture
            True to use depth textures. False otherwise (default).
        */
        void setPreferDepthTexture( bool preferDepthTexture );

        /// @see setPreferDepthTexture
        bool prefersDepthTexture() const;

        /** Set the desired depth buffer format
        @remarks
            Ogre will try to honour this request, but if it's not supported,
            you may get lower precision.
            All formats will try to fallback to PF_D24_UNORM_S8_UINT if not supported.
        @par
            Changing this setting will cause the current depth buffer to be detached unless
            the old and the setting are the same.
        @param desiredDepthBufferFormat
            Must be one of the following:
                PF_D24_UNORM_S8_UINT
                PF_D24_UNORM_X8
                PF_D16_UNORM
                PF_D32_FLOAT
                PF_D32_FLOAT_X24_S8_UINT
        */
        void setDesiredDepthBufferFormat( PixelFormat desiredDepthBufferFormat );

        /// @see setDesiredDepthBufferFormat
        PixelFormat getDesiredDepthBufferFormat(void) const;

        DepthBuffer* getDepthBuffer() const;

        /// Returns false if couldn't attach
        virtual bool attachDepthBuffer( DepthBuffer *depthBuffer, bool exactFormatMatch );

        virtual void detachDepthBuffer();

        /** Detaches DepthBuffer without notifying it from the detach.
            Useful when called from the DepthBuffer while it iterates through attached
            RenderTargets (@see DepthBuffer::_setPoolId())
        */
        virtual void _detachDepthBuffer();

        /** Swaps the frame buffers to display the next frame.
            @remarks
                For targets that are double-buffered so that no
                'in-progress' versions of the scene are displayed
                during rendering. Once rendering has completed (to
                an off-screen version of the window) the buffers
                are swapped to display the new frame.
        */
        virtual void swapBuffers(void)                                      { mFsaaResolveDirty = false; }

        virtual void setFsaaResolveDirty(void)
        {
            mFsaaResolveDirty = true;
            mMipmapsDirty = true;
        }

        /** Adds a viewport to the rendering target.
            @remarks
                A viewport is the rectangle into which rendering output is sent. This method adds
                a viewport to the render target, rendering from the supplied camera. The
                rest of the parameters are only required if you wish to add more than one viewport
                to a single rendering target. Note that size information passed to this method is
                passed as a parametric, i.e. it is relative rather than absolute. This is to allow
                viewports to automatically resize along with the target.
            @param
                cam The camera from which the viewport contents will be rendered (mandatory)
            @param
                ZOrder The relative order of the viewport with others on the target (allows overlapping
                viewports i.e. picture-in-picture). Higher Z-orders are on top of lower ones. The actual number
                is irrelevant, only the relative Z-order matters (you can leave gaps in the numbering)
            @param
                left The relative position of the left of the viewport on the target, as a value between 0 and 1.
            @param
                top The relative position of the top of the viewport on the target, as a value between 0 and 1.
            @param
                width The relative width of the viewport on the target, as a value between 0 and 1.
            @param
                height The relative height of the viewport on the target, as a value between 0 and 1.
        */
        virtual Viewport* addViewport( float left = 0.0f, float top = 0.0f,
                                        float width = 1.0f, float height = 1.0f );

        /** Returns the number of viewports attached to this target.*/
        virtual unsigned short getNumViewports(void) const;

        /** Retrieves a pointer to the viewport with the given index. */
        virtual Viewport* getViewport(unsigned short index);

        /** Removes a viewport at a given Z-order.
        */
        virtual void removeViewport( Viewport *vp );

        /** Removes all viewports on this target.
        */
        virtual void removeAllViewports(void);

        virtual const FrameStats& getStatistics(void) const;

        /** Resets saved frame-rate statistices.
        */
        virtual void resetStatistics(void);

        /** Gets a custom (maybe platform-specific) attribute.
            @remarks
                This is a nasty way of satisfying any API's need to see platform-specific details.
                It horrid, but D3D needs this kind of info. At least it's abstracted.
            @param
                name The name of the attribute.
            @param
                pData Pointer to memory of the right kind of structure to receive the info.
        */
        virtual void getCustomAttribute(const String& name, void* pData);

        /** Add a listener to this RenderTarget which will be called back before & after rendering.
        @remarks
            If you want notifications before and after a target is updated by the system, use
            this method to register your own custom RenderTargetListener class. This is useful
            for potentially adding your own manual rendering commands before and after the
            'normal' system rendering.
        @par NB this should not be used for frame-based scene updates, use Root::addFrameListener for that.
        */
        virtual void addListener(RenderTargetListener* listener);
        /** Removes a RenderTargetListener previously registered using addListener. */
        virtual void removeListener(RenderTargetListener* listener);
        /** Removes all listeners from this instance. */
        virtual void removeAllListeners(void);

        /** Sets the priority of this render target in relation to the others. 
        @remarks
            This can be used in order to schedule render target updates. Lower
            priorities will be rendered first. Note that the priority must be set
            at the time the render target is attached to the render system, changes
            afterwards will not affect the ordering.
        */
        virtual void setPriority( uchar priority ) { mPriority = priority; }
        /** Gets the priority of a render target. */
        virtual uchar getPriority() const { return mPriority; }

        /** Used to retrieve or set the active state of the render target.
        */
        virtual bool isActive() const;

        /** Used to set the active state of the render target.
        */
        virtual void setActive( bool state );

        /** Copies the current contents of the render target to a pixelbox. 
        @remarks See suggestPixelFormat for a tip as to the best pixel format to
            extract into, although you can use whatever format you like and the 
            results will be converted.
        */
        virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer = FB_AUTO) = 0;

        /** Suggests a pixel format to use for extracting the data in this target, 
            when calling copyContentsToMemory.
        */
        virtual PixelFormat suggestPixelFormat() const { return PF_BYTE_RGBA; }
        
        /** Writes the current contents of the render target to the named file.
            If format is unspecified (PF_UNKNOWN), the most suitable one is automatically chosen.
        */
        void writeContentsToFile( const String& filename, PixelFormat format = PF_UNKNOWN );

        /** Writes the current contents of the render target to the (PREFIX)(time-stamp)(SUFFIX) file.
            @return the name of the file used.*/
        virtual String writeContentsToTimestampedFile( const String& filenamePrefix, const String& filenameSuffix,
                                                       PixelFormat format = PF_UNKNOWN );

        virtual bool requiresTextureFlipping() const = 0;

        /** Gets the number of triangles rendered in the last update() call. */
        virtual size_t getTriangleCount(void) const;
        /** Gets the number of batches rendered in the last update() call. */
        virtual size_t getBatchCount(void) const;

        /** Indicates whether this target is the primary window. The
            primary window is special in that it is destroyed when
            ogre is shut down, and cannot be destroyed directly.
            This is the case because it holds the context for vertex,
            index buffers and textures.
        */
        virtual bool isPrimary(void) const;

		/** Indicates whether stereo is currently enabled for this target. Default is false. */
		virtual bool isStereoEnabled(void) const;
		
        /** Indicates whether on rendering, linear colour space is converted to 
            sRGB gamma colour space. This is the exact opposite conversion of
            what is indicated by Texture::isHardwareGammaEnabled, and can only
            be enabled on creation of the render target. For render windows, it's
            enabled through the 'gamma' creation misc parameter. For textures, 
            it is enabled through the hwGamma parameter to the create call.
        */
        virtual bool isHardwareGammaEnabled() const { return mHwGamma; }

        /** Indicates whether multisampling is performed on rendering and at what level.
        */
        virtual uint getFSAA() const { return mFSAA; }

        /** Gets the FSAA hint (@see Root::createRenderWindow)
        */
        virtual const String& getFSAAHint() const { return mFSAAHint; }

        bool isFsaaResolveDirty(void) const         { return mFsaaResolveDirty; }

        /** Set the level of multisample AA to be used if hardware support it.
            This option will be ignored if the hardware does not support it 
            or setting can not be changed on the fly on per-target level. 
            @param fsaa The number of samples
            @param fsaaHint Any hinting text (@see Root::createRenderWindow)
        */
        virtual void setFSAA(uint fsaa, const String& fsaaHint) { }

        void _setMipmapsUpdated(void)               { mMipmapsDirty = false; }
        bool isMipmapsDirty(void) const             { return mMipmapsDirty; }

        /** RenderSystem specific interface for a RenderTarget;
            this should be subclassed by RenderSystems.
        */
        class Impl
        {
        protected:
            ~Impl() { }
        };
        /** Get rendersystem specific interface for this RenderTarget.
            This is used by the RenderSystem to (un)bind this target, 
            and to get specific information like surfaces
            and framebuffer objects.
        */
        virtual Impl *_getImpl();

        /** Method for manual management of rendering : fires 'preRenderTargetUpdate'
            and initialises statistics etc.
        @remarks 
        <ul>
        <li>_beginUpdate resets statistics and fires 'preRenderTargetUpdate'.</li>
        <li>_updateViewport renders the given viewport (even if it is not autoupdated),
        fires preViewportUpdate and postViewportUpdate and manages statistics.</li>
        <li>_updateAutoUpdatedViewports renders only viewports that are auto updated,
        fires preViewportUpdate and postViewportUpdate and manages statistics.</li>
        <li>_endUpdate() ends statistics calculation and fires postRenderTargetUpdate.</li>
        </ul>
        you can use it like this for example :
        <pre>
            renderTarget->_beginUpdate();
            renderTarget->_updateViewport(1); // which is not auto updated
            renderTarget->_updateViewport(2); // which is not auto updated
            renderTarget->_updateAutoUpdatedViewports();
            renderTarget->_endUpdate();
            renderTarget->swapBuffers();
        </pre>
            Please note that in that case, the zorder may not work as you expect,
            since you are responsible for calling _updateViewport in the correct order.
        */
        virtual void _beginUpdate();

        void _updateViewportCullPhase01(Viewport* viewport, Camera *camera, const Camera *lodCamera,
                                        uint8 firstRq, uint8 lastRq );

        /** Method for manual management of rendering - renders the given viewport (even if it is not autoupdated)
        @remarks
            This also fires preViewportUpdate and postViewportUpdate, and manages statistics
            if needed. You should call it between _beginUpdate() and _endUpdate().
            @see _beginUpdate for more details.
        @param viewport
            The viewport you want to update, it must be bound to the rendertarget.
        @param updateStatistics
            Whether you want to update statistics or not.
        */
        virtual void _updateViewportRenderPhase02( Viewport* viewport, Camera *camera,
                                                   const Camera *lodCamera,uint8 firstRq, uint8 lastRq,
                                                   bool updateStatistics );

        /// Whether our derived class is RenderWindow
        virtual bool isRenderWindow(void) const             { return false; }
        
        /** Method for manual management of rendering - finishes statistics calculation 
            and fires 'postRenderTargetUpdate'.
        @remarks
        You should call it after a _beginUpdate
        @see _beginUpdate for more details.
        */
        virtual void _endUpdate();

        /// Used by depth texture views which need to disable colour writes when rendering to it
        /// PF_NULL targets can be identified because they set this value to true and have
        /// no depth buffers attached.
        virtual bool getForceDisableColourWrites(void) const    { return false; }

    protected:
        /// The name of this target.
        String mName;
        /// The priority of the render target.
        uchar mPriority;

        uint32 mWidth;
        uint32 mHeight;
        PixelFormat mFormat;
        uint16      mDepthBufferPoolId;
        bool        mPreferDepthTexture;
        PixelFormat mDesiredDepthBufferFormat;
        DepthBuffer *mDepthBuffer;

        // Stats
        FrameStats mStats;

        bool mActive;
        // Hardware sRGB gamma conversion done on write?
        bool mHwGamma;
        // FSAA performed?
        uint mFSAA;
        String mFSAAHint;
        bool mFsaaResolveDirty;
        bool mMipmapsDirty;
        bool mStereoEnabled;

        typedef vector<Viewport*>::type ViewportList;
        /// List of viewports, map on Z-order
        ViewportList mViewportList;

        typedef vector<RenderTargetListener*>::type RenderTargetListenerList;
        RenderTargetListenerList mListeners;
    

        /// internal method for firing events
        virtual void firePreUpdate(void);
        /// internal method for firing events
        virtual void firePostUpdate(void);
        /// internal method for firing events
        virtual void fireViewportPreUpdate(Viewport* vp);
        /// internal method for firing events
        virtual void fireViewportPostUpdate(Viewport* vp);
        /// internal method for firing events
        virtual void fireViewportAdded(Viewport* vp);
        /// internal method for firing events
        virtual void fireViewportRemoved(Viewport* vp);
    };
    /** @} */
    /** @} */

} // Namespace

#include "OgreHeaderSuffix.h"

#endif
