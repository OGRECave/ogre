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
#ifndef __Viewport_H__
#define __Viewport_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreColourValue.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
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
        /** The usual constructor.
            @param
                cam Pointer to a camera to be the source for the image.
            @param
                target Pointer to the render target to be the destination
                for the rendering.
            @param
                left
            @param
                top
            @param
                width
            @param
                height
                Dimensions of the viewport, expressed as a value between
                0 and 1. This allows the dimensions to apply irrespective of
                changes in the target's size: e.g. to fill the whole area,
                values of 0,0,1,1 are appropriate.
            @param
                ZOrder Relative Z-order on the target. Lower = further to
                the front.
        */
        Viewport(
            Camera* camera,
            RenderTarget* target,
            Real left, Real top,
            Real width, Real height,
            int ZOrder);

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
        void update(void);

        /** Retrieves a pointer to the render target for this viewport.
        */
        RenderTarget* getTarget(void) const;

        /** Retrieves a pointer to the camera for this viewport.
        */
        Camera* getCamera(void) const;

        /** Sets the camera to use for rendering to this viewport. */
        void setCamera(Camera* cam);

        /** Gets the Z-Order of this viewport. */
		int getZOrder(void) const;
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

        /** Sets the dimensions (after creation).
            @param
                left
            @param
                top
            @param
                width
            @param
                height Dimensions relative to the size of the target,
                represented as real values between 0 and 1. i.e. the full
                target area is 0, 0, 1, 1.
        */
        void setDimensions(Real left, Real top, Real width, Real height);

        /** Sets the initial background colour of the viewport (before
            rendering).
        */
        void setBackgroundColour(const ColourValue& colour);

        /** Gets the background colour.
        */
        const ColourValue& getBackgroundColour(void) const;

        /** Determines whether to clear the viewport before rendering.
		@remarks
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
        bool getClearEveryFrame(void) const;

		/** Gets which buffers are to be cleared each frame. */
        unsigned int getClearBuffers(void) const;

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

        /** Gets the number of rendered faces in the last update.
        */
        unsigned int _getNumRenderedFaces(void) const;

        /** Gets the number of rendered batches in the last update.
        */
        unsigned int _getNumRenderedBatches(void) const;

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

        /** Tells this viewport whether it should display shadows.
        @remarks
            This setting enables you to disable shadow rendering for a given viewport. The global
			shadow technique set on SceneManager still controls the type and nature of shadows,
			but this flag can override the setting so that no shadows are rendered for a given
			viewport to save processing time where they are not required.
        @param enabled If true, any shadows are displayed, if false they are not.
        */
        void setShadowsEnabled(bool enabled);

        /** Returns whether or not shadows (defined in the SceneManager) are displayed in this
            viewport. */
        bool getShadowsEnabled(void) const;


		/** Sets a per-viewport visibility mask.
		@remarks
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

		/** Sets the use of a custom RenderQueueInvocationSequence for
			rendering this target.
		@remarks
			RenderQueueInvocationSequence instances are managed through Root. By
			setting this, you are indicating that you wish this RenderTarget to
			be updated using a custom sequence of render queue invocations, with
			potentially customised ordering and render state options. You should
			create the named sequence through Root first, then set the name here.
		@param The name of the RenderQueueInvocationSequence to use. If you
			specify a blank string, behaviour will return to the default render
			queue management.
		*/
		virtual void setRenderQueueInvocationSequenceName(const String& sequenceName);
		/** Gets the name of the render queue invocation sequence for this target. */
		virtual const String& getRenderQueueInvocationSequenceName(void) const;
		/// Get the invocation sequence - will return null if using standard
		RenderQueueInvocationSequence* _getRenderQueueInvocationSequence(void);

    protected:
        Camera* mCamera;
        RenderTarget* mTarget;
        // Relative dimensions, irrespective of target dimensions (0..1)
        float mRelLeft, mRelTop, mRelWidth, mRelHeight;
        // Actual dimensions, based on target dimensions
        int mActLeft, mActTop, mActWidth, mActHeight;
        /// ZOrder
        int mZOrder;
        /// Background options
        ColourValue mBackColour;
        bool mClearEveryFrame;
		unsigned int mClearBuffers;
        bool mUpdated;
        bool mShowOverlays;
        bool mShowSkies;
		bool mShowShadows;
		uint32 mVisibilityMask;
		// Render queue invocation sequence name
		String mRQSequenceName;
		RenderQueueInvocationSequence* mRQSequence;
		/// Material scheme
		String mMaterialSchemeName;
    };
	/** @} */
	/** @} */

}

#endif
