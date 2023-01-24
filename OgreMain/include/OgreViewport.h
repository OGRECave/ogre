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
#ifndef __Viewport_H__
#define __Viewport_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreFrustum.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */
    /** An abstraction of a viewport, i.e. a rendering region on a render
        target.

            A viewport is the meeting of a camera and a rendering surface -
            the camera renders the scene from a viewpoint, and places its
            results into some subset of a rendering target, which may be the
            whole surface or just a part of the surface. Each viewport has a
            single camera as source and a single target as destination. A
            camera only has 1 viewport, but a render target may have several.
            A viewport also has a Z-order, i.e. if there is more than one
            viewport on a single render target and they overlap, one must
            obscure the other in some predetermined way.
    */
    class _OgreExport Viewport : public ViewportAlloc
    {
    public:
        /** Listener interface so you can be notified of Viewport changes. */
        class _OgreExport Listener
        {
        public:
            virtual ~Listener() {}

            /** Notification of when a new camera is set to target listening Viewport. */
            virtual void viewportCameraChanged(Viewport* viewport) {}

            /** Notification of when target listening Viewport's dimensions changed. */
            virtual void viewportDimensionsChanged(Viewport* viewport) {}

            /** Notification of when target listening Viewport's is destroyed. */
            virtual void viewportDestroyed(Viewport* viewport) {}
        };

        /** The usual constructor.
            @param camera
                Pointer to a camera to be the source for the image.
            @param target
                Pointer to the render target to be the destination
                for the rendering.
            @param left, top, width, height
                Dimensions of the viewport, expressed as a value between
                0 and 1. This allows the dimensions to apply irrespective of
                changes in the target's size: e.g. to fill the whole area,
                values of 0,0,1,1 are appropriate.
            @param ZOrder
                Relative Z-order on the target. Lower = further to
                the front.
        */
        Viewport(Camera* camera, RenderTarget* target, float left, float top, float width, float height, int ZOrder);

        /** Default destructor.
        */
        virtual ~Viewport();

        /** Notifies the viewport of a possible change in dimensions.

                Used by the target to update the viewport's dimensions
                (usually the result of a change in target size).
            @note
                Internal use by Ogre only.
        */
        void _updateDimensions(void);

        /** Instructs the viewport to updates its contents.
        */
        void update(void);
        
        /** Instructs the viewport to clear itself, without performing an update.

            You would not normally call this method when updating the viewport, 
            since the viewport usually clears itself when updating anyway (@see 
            Viewport::setClearEveryFrame). However, if you wish you have the
            option of manually clearing the frame buffer (or elements of it)
            using this method.
         @param buffers Bitmask identifying which buffer elements to clear
         @param colour The colour value to clear to, if FBT_COLOUR is included
         @param depth The depth value to clear to, if FBT_DEPTH is included
         @param stencil The stencil value to clear to, if FBT_STENCIL is included
        */
        void clear(uint32 buffers = FBT_COLOUR | FBT_DEPTH, const ColourValue& colour = ColourValue::Black,
                   float depth = 1.0f, uint16 stencil = 0);

        /** Retrieves a pointer to the render target for this viewport.
        */
        RenderTarget* getTarget(void) const { return mTarget; }

        /** Retrieves a pointer to the camera for this viewport.
        */
        Camera* getCamera(void) const { return mCamera; }

        /** Sets the camera to use for rendering to this viewport. */
        void setCamera(Camera* cam);

        /** Gets the Z-Order of this viewport. */
        int getZOrder(void) const { return mZOrder; }
        /** Gets one of the relative dimensions of the viewport,
            a value between 0.0 and 1.0.
        */
        float getLeft(void) const { return mRelRect.left; }

        /** Gets one of the relative dimensions of the viewport, a value
            between 0.0 and 1.0.
        */
        float getTop(void) const { return mRelRect.top; }

        /** Gets one of the relative dimensions of the viewport, a value
            between 0.0 and 1.0.
        */
        float getWidth(void) const { return mRelRect.width(); }
        /** Gets one of the relative dimensions of the viewport, a value
            between 0.0 and 1.0.
        */
        float getHeight(void) const { return mRelRect.height(); }
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */

        int getActualLeft(void) const { return mActRect.left; }
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */

        int getActualTop(void) const { return mActRect.top; }
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */
        int getActualWidth(void) const { return mActRect.width(); }
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */

        int getActualHeight(void) const { return mActRect.height(); }

        /** Sets the dimensions (after creation).
            @param
                left Left point of viewport.
            @param
                top Top point of the viewport.
            @param
                width Width of the viewport.
            @param
                height Height of the viewport.
            @note Dimensions relative to the size of the target,
                represented as real values between 0 and 1. i.e. the full
                target area is 0, 0, 1, 1.
        */
        void setDimensions(float left, float top, float width, float height);

        /** Sets the initial background colour of the viewport (before
            rendering).
        */
        void setBackgroundColour(const ColourValue& colour) { mBackColour = colour; }

        /** Gets the background colour.
        */
        const ColourValue& getBackgroundColour(void) const { return mBackColour; }

        /** Sets the initial depth buffer value of the viewport (before
            rendering). Default is 1
        */
        void setDepthClear( float depth ) { mDepthClearValue = depth; }

        /** Gets the default depth buffer value to which the viewport is cleared.
        */
        float getDepthClear(void) const { return mDepthClearValue; }

        /** Determines whether to clear the viewport before rendering.

            You can use this method to set which buffers are cleared
            (if any) before rendering every frame.
        @param clear Whether or not to clear any buffers
        @param buffers One or more values from FrameBufferType denoting
            which buffers to clear, if clear is set to true. Note you should
            not clear the stencil buffer here unless you know what you're doing.
         */
        void setClearEveryFrame(bool clear, unsigned int buffers = FBT_COLOUR | FBT_DEPTH);

        /** Determines if the viewport is cleared before every frame.
        */
        bool getClearEveryFrame(void) const { return mClearEveryFrame; }

        /** Gets which buffers are to be cleared each frame. */
        unsigned int getClearBuffers(void) const { return mClearBuffers; }

        /** Sets whether this viewport should be automatically updated 
            if Ogre's rendering loop or RenderTarget::update is being used.

            By default, if you use Ogre's own rendering loop (Root::startRendering)
            or call RenderTarget::update, all viewports are updated automatically.
            This method allows you to control that behaviour, if for example you 
            have a viewport which you only want to update periodically.
        @param autoupdate If true, the viewport is updated during the automatic
            render loop or when RenderTarget::update() is called. If false, the 
            viewport is only updated when its update() method is called explicitly.
        */
        void setAutoUpdated(bool autoupdate) { mIsAutoUpdated = autoupdate; }
        /** Gets whether this viewport is automatically updated if 
            Ogre's rendering loop or RenderTarget::update is being used.
        */
        bool isAutoUpdated() const { return mIsAutoUpdated; }

        /** Set the material scheme which the viewport should use.

            This allows you to tell the system to use a particular
            material scheme when rendering this viewport, which can 
            involve using different techniques to render your materials.
        @see Technique::setSchemeName
        */
        void setMaterialScheme(const String& schemeName)
        { mMaterialSchemeName = schemeName; }
        
        /** Get the material scheme which the viewport should use.
        */
        const String& getMaterialScheme(void) const
        { return mMaterialSchemeName; }

        /** Access to actual dimensions (based on target size).
        */
        Rect getActualDimensions() const { return mActRect; }

        /// @deprecated
        OGRE_DEPRECATED void getActualDimensions(int& left, int& top, int& width, int& height) const;

        bool _isUpdated(void) const;
        void _clearUpdatedFlag(void);

        /** Gets the number of rendered faces in the last update.
        */
        unsigned int _getNumRenderedFaces(void) const;

        /** Gets the number of rendered batches in the last update.
        */
        unsigned int _getNumRenderedBatches(void) const;

        /** Tells this viewport whether it should display Overlay objects.

            Overlay objects are layers which appear on top of the scene. They are created via
            SceneManager::createOverlay and every viewport displays these by default.
            However, you probably don't want this if you're using multiple viewports,
            because one of them is probably a picture-in-picture which is not supposed to
            have overlays of it's own. In this case you can turn off overlays on this viewport
            by calling this method.
        @param enabled If true, any overlays are displayed, if false they are not.
        */
        void setOverlaysEnabled(bool enabled) { mShowOverlays = enabled; }

        /** Returns whether or not Overlay objects (created in the SceneManager) are displayed in this
            viewport. */
        bool getOverlaysEnabled(void) const { return mShowOverlays; }

        /** Tells this viewport whether it should display skies.

            Skies are layers which appear on background of the scene. They are created via
            SceneManager::setSkyBox, SceneManager::setSkyPlane and SceneManager::setSkyDome and
            every viewport displays these by default. However, you probably don't want this if
            you're using multiple viewports, because one of them is probably a picture-in-picture
            which is not supposed to have skies of it's own. In this case you can turn off skies
            on this viewport by calling this method.
        @param enabled If true, any skies are displayed, if false they are not.
        */
        void setSkiesEnabled(bool enabled) { mShowSkies = enabled; }

        /** Returns whether or not skies (created in the SceneManager) are displayed in this
            viewport. */
        bool getSkiesEnabled(void) const { return mShowSkies; }

        /** Tells this viewport whether it should display shadows.

            This setting enables you to disable shadow rendering for a given viewport. The global
            shadow technique set on SceneManager still controls the type and nature of shadows,
            but this flag can override the setting so that no shadows are rendered for a given
            viewport to save processing time where they are not required.
        @param enabled If true, any shadows are displayed, if false they are not.
        */
        void setShadowsEnabled(bool enabled) { mShowShadows = enabled; }

        /** Returns whether or not shadows (defined in the SceneManager) are displayed in this
            viewport. */
        bool getShadowsEnabled(void) const { return mShowShadows; }


        /** Sets a per-viewport visibility mask.

            The visibility mask is a way to exclude objects from rendering for
            a given viewport. For each object in the frustum, a check is made
            between this mask and the objects visibility flags 
            (@see MovableObject::setVisibilityFlags), and if a binary 'and'
            returns zero, the object will not be rendered.
        */
        void setVisibilityMask(uint32 mask) { mVisibilityMask = mask; }

        /** Gets a per-viewport visibility mask.
        @see Viewport::setVisibilityMask
        */
        uint getVisibilityMask(void) const { return mVisibilityMask; }

        /// Add a listener to this viewport
        void addListener(Listener* l);
        /// Remove a listener to this viewport
        void removeListener(Listener* l);
		
		/** Sets the draw buffer type for the next frame.

			Specifies the particular buffer that will be
			targeted by the render target. Should be used if
			the render target supports quad buffer stereo. If
			the render target does not support stereo (ie. left
			and right), then only back and front will be used.
		@param
			colourBuffer Specifies the particular buffer that will be
			targeted by the render target.
		*/
		void setDrawBuffer(ColourBufferType colourBuffer) { mColourBuffer = colourBuffer; }

		/** Returns the current colour buffer type for this viewport.*/
		ColourBufferType getDrawBuffer() const { return mColourBuffer; }

    private:
        Camera* mCamera;
        RenderTarget* mTarget;
        /// Relative dimensions, irrespective of target dimensions (0..1)
        FloatRect mRelRect;
        /// Actual dimensions, based on target dimensions
        Rect mActRect;
        /// Z-order
        int mZOrder;
        /// Background options
        ColourValue mBackColour;
        float mDepthClearValue;
        bool mClearEveryFrame;
        unsigned int mClearBuffers;
        bool mUpdated;
        bool mShowOverlays;
        bool mShowSkies;
        bool mShowShadows;
        uint32 mVisibilityMask;
        /// Material scheme
        String mMaterialSchemeName;

        /// Automatic rendering on/off
        bool mIsAutoUpdated;

        typedef std::vector<Listener*> ListenerList;
        ListenerList mListeners;
		ColourBufferType mColourBuffer;
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
