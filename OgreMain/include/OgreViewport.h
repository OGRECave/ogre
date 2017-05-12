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

    enum ViewportRenderTargetFlags
    {
        /// Do not write colour, only depth/stencil
        VP_RTT_COLOUR_WRITE         = 1u << 0u,
        /// Assume stencil is read only (it's a hint, not an enforcement)
        VP_RTT_READ_ONLY_DEPTH      = 1u << 1u,
        /// Assume stencil is read only (it's a hint, not an enforcement)
        VP_RTT_READ_ONLY_STENCIL    = 1u << 2u,
    };

    /** An abstraction of a viewport, i.e. a rendering region on a render
        target.
        @remarks
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
        /// @copydoc MovableObject::mGlobalIndex
        size_t mGlobalIndex;

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
        Viewport(
            RenderTarget* target,
            Real left, Real top,
            Real width, Real height );

        /** Default destructor.
        */
        virtual ~Viewport();

        /** Notifies the viewport of a possible change in dimensions.
            @remarks
                Used by the target to update the viewport's dimensions
                (usually the result of a change in target size).
            @note
                Internal use by Ogre only.
        */
        void _updateDimensions(void);

        /** Instructs the viewport to updates its contents.
        */
        void _updateCullPhase01(Camera* camera, const Camera *lodCamera, uint8 firstRq, uint8 lastRq );
        void _updateRenderPhase02( Camera* camera, const Camera *lodCamera,
                                   uint8 firstRq, uint8 lastRq );
        
        /** Instructs the viewport to clear itself, without performing an update.
         @remarks
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
        void clear(unsigned int buffers = FBT_COLOUR | FBT_DEPTH,
                   const ColourValue& colour = ColourValue::Black, 
                   Real depth = 1.0f, unsigned short stencil = 0);

        /** Instructs the viewport to 'discard' the buffers.
            @see CompositorPassClearDef::mDiscardOnly
        */
        void discard( unsigned int buffers = FBT_COLOUR | FBT_DEPTH );

        /** Retrieves a pointer to the render target for this viewport.
        */
        RenderTarget* getTarget(void) const;

        /** Gets one of the relative dimensions of the viewport,
            a value between 0.0 and 1.0.
        */
        Real getLeft(void) const;

        /** Gets one of the relative dimensions of the viewport, a value
            between 0.0 and 1.0.
        */
        Real getTop(void) const;

        /** Gets one of the relative dimensions of the viewport, a value
            between 0.0 and 1.0.
        */

        Real getWidth(void) const;
        /** Gets one of the relative dimensions of the viewport, a value
            between 0.0 and 1.0.
        */

        Real getHeight(void) const;
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */

        int getActualLeft(void) const;
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */

        int getActualTop(void) const;
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */
        int getActualWidth(void) const;
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */
        int getActualHeight(void) const;

        Real getScissorLeft(void) const                 { return mScissorRelLeft; }
        Real getScissorTop(void) const                  { return mScissorRelTop; }
        Real getScissorWidth(void) const                { return mScissorRelWidth; }
        Real getScissorHeight(void) const               { return mScissorRelHeight; }

        int getScissorActualLeft(void) const            { return mScissorActLeft; }
        int getScissorActualTop(void) const             { return mScissorActTop; }
        int getScissorActualWidth(void) const           { return mScissorActWidth; }
        int getScissorActualHeight(void) const          { return mScissorActHeight; }

        bool coversEntireTarget(void) const;
        bool scissorsMatchViewport(void) const;

        /** Sets the dimensions (after creation).
        @param left
            Left point of viewport.
        @param top
            Top point of the viewport.
        @param width
            Width of the viewport.
        @param height
            Height of the viewport.
        @param overrideScissors
            When true, the scissor dimensions will be the same as the viewport's
            @See setScissors
        @note
            Dimensions relative to the size of the target, represented as real values
            between 0 and 1. i.e. the full target area is 0, 0, 1, 1.
        */
        void setDimensions( Real left, Real top, Real width, Real height, bool overrideScissors=true );

        /** Only sets the scissor regions. The scissor rectangle must be fully inside
            the viewport rectangle. @See setDimensions for param description
        @remarks
            Only the scissor rect is set here; but the HLMS macroblock controls whether
            scissor testing is enabled or not (@See HlmsMacroblock). On some RenderSystem
            implementations (i.e. OpenGL), scissor testing needs to be enabled when
            clearing a region of the screen. In those cases, if scissor testing is disabled at
            the time of the clear, scissor testing will be temporarily enabled and then disabled.
        */
        void setScissors( Real left, Real top, Real width, Real height );

        /** Set the orientation mode of the viewport.
        */
        void setOrientationMode(OrientationMode orientationMode, bool setDefault = true);

        /** Get the orientation mode of the viewport.
        */
        OrientationMode getOrientationMode() const;

        /** Set the initial orientation mode of viewports.
        */
        static void setDefaultOrientationMode(OrientationMode orientationMode);

        /** Get the initial orientation mode of viewports.
        */
        static OrientationMode getDefaultOrientationMode();

        /** Set the material scheme which the viewport should use.
        @remarks
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
        void getActualDimensions(
            int &left, int &top, int &width, int &height ) const;

        bool _isUpdated(void) const;
        void _clearUpdatedFlag(void);

        /** Tells this viewport whether it should display Overlay objects.
        @remarks
            Overlay objects are layers which appear on top of the scene. They are created via
            SceneManager::createOverlay and every viewport displays these by default.
            However, you probably don't want this if you're using multiple viewports,
            because one of them is probably a picture-in-picture which is not supposed to
            have overlays of it's own. In this case you can turn off overlays on this viewport
            by calling this method.
        @param enabled If true, any overlays are displayed, if false they are not.
        */
        void setOverlaysEnabled(bool enabled);

        /** Returns whether or not Overlay objects (created in the SceneManager) are displayed in this
            viewport. */
        bool getOverlaysEnabled(void) const;

        /** Tells this viewport whether it should display skies.
        @remarks
            Skies are layers which appear on background of the scene. They are created via
            SceneManager::setSkyBox, SceneManager::setSkyPlane and SceneManager::setSkyDome and
            every viewport displays these by default. However, you probably don't want this if
            you're using multiple viewports, because one of them is probably a picture-in-picture
            which is not supposed to have skies of it's own. In this case you can turn off skies
            on this viewport by calling this method.
        @param enabled If true, any skies are displayed, if false they are not.
        */
        void setSkiesEnabled(bool enabled);

        /** Returns whether or not skies (created in the SceneManager) are displayed in this
            viewport. */
        bool getSkiesEnabled(void) const;

        /** Sets a per-viewport visibility mask.
        @remarks
            The visibility mask is a way to exclude objects from rendering for
            a given viewport. For each object in the frustum, a check is made
            between this mask and the objects visibility flags 
            (@see MovableObject::setVisibilityFlags), and if a binary 'and'
            returns zero, the object will not be rendered.
        @par
            Viewport's visibility mask assumes the user knows what he's doing
            with the reserved flags!
        */
        void _setVisibilityMask( uint32 mask, uint32 lightMask );

        /** Gets a per-viewport visibility mask.
        @see Viewport::setVisibilityMask
        */
        uint32 getVisibilityMask(void) const        { return mVisibilityMask; }
        uint32 getLightVisibilityMask(void) const   { return mLightVisibilityMask; }

        void setColourWrite( bool colourWrite );
        bool getColourWrite(void) const;

        void setReadOnly( bool readOnlyDepth, bool readOnlyStencil );
        bool getReadOnlyDepth(void) const;
        bool getReadOnlStencil(void) const;

        uint8 getViewportRenderTargetFlags(void) const          { return mViewportRenderTargetFlags; }

        /** Convert oriented input point coordinates to screen coordinates. */
        void pointOrientedToScreen(const Vector2 &v, int orientationMode, Vector2 &outv);
        void pointOrientedToScreen(Real orientedX, Real orientedY, int orientationMode,
                                   Real &screenX, Real &screenY);

        /** Sets the draw buffer type for the next frame.
        @remarks
            Specifies the particular buffer that will be
            targeted by the render target. Should be used if
            the render target supports quad buffer stereo. If
            the render target does not support stereo (ie. left
            and right), then only back and front will be used.
        @param
            buffer Specifies the particular buffer that will be
            targeted by the render target.
        */
        void setDrawBuffer(ColourBufferType colourBuffer);

        /** Returns the current colour buffer type for this viewport.*/
        ColourBufferType getDrawBuffer() const;

    protected:

        RenderTarget* mTarget;
        /// Relative dimensions, irrespective of target dimensions (0..1)
        float mRelLeft, mRelTop, mRelWidth, mRelHeight;
        /// Actual dimensions, based on target dimensions
        int mActLeft, mActTop, mActWidth, mActHeight;

        /// Relative dimensions, irrespective of target dimensions (0..1), scissor rect
        float mScissorRelLeft, mScissorRelTop, mScissorRelWidth, mScissorRelHeight;
        /// Actual dimensions, based on target dimensions, scissor rect
        int mScissorActLeft, mScissorActTop, mScissorActWidth, mScissorActHeight;
        bool mCoversEntireTarget;
        bool mScissorsMatchViewport;

        /// Z-order
        int mZOrder;
        uint8  mViewportRenderTargetFlags; /// See ViewportRenderTargetFlags
        /// Background options
        bool mUpdated;
        bool mShowOverlays;
        bool mShowSkies;
        uint32 mVisibilityMask;
        uint32 mLightVisibilityMask;
        /// Material scheme
        String mMaterialSchemeName;
        /// Viewport orientation mode
        OrientationMode mOrientationMode;
        static OrientationMode mDefaultOrientationMode;
        ColourBufferType mColourBuffer;
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
