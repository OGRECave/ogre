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
#ifndef __Frustum_H__
#define __Frustum_H__

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreAxisAlignedBox.h"
#include "OgreVertexIndexData.h"
#include "OgreMovablePlane.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Specifies perspective (realistic) or orthographic (architectural) projection.
    */
    enum ProjectionType
    {
        PT_ORTHOGRAPHIC,
        PT_PERSPECTIVE
    };

    /** Worldspace clipping planes.
    */
    enum FrustumPlane
    {
        FRUSTUM_PLANE_NEAR   = 0,
        FRUSTUM_PLANE_FAR    = 1,
        FRUSTUM_PLANE_LEFT   = 2,
        FRUSTUM_PLANE_RIGHT  = 3,
        FRUSTUM_PLANE_TOP    = 4,
        FRUSTUM_PLANE_BOTTOM = 5
    };

    /** A frustum represents a pyramid, capped at the near and far end which is
        used to represent either a visible area or a projection area. Can be used
        for a number of applications.
    */
    class _OgreExport Frustum : public MovableObject, public Renderable
    {
    protected:
        /// Orthographic or perspective?
        ProjectionType mProjType;

        /// y-direction field-of-view (default 45)
        Radian mFOVy;
        /// Far clip distance - default 10000
        Real mFarDist;
        /// Near clip distance - default 100
        Real mNearDist;
        /// x/y viewport ratio - default 1.3333
        Real mAspect;
		/// Ortho height size (world units)
		Real mOrthoHeight;
        /// Off-axis frustum center offset - default (0.0, 0.0)
        Vector2 mFrustumOffset;
        /// Focal length of frustum (for stereo rendering, defaults to 1.0)
        Real mFocalLength;

        /// The 6 main clipping planes
        mutable Plane mFrustumPlanes[6];

        /// Stored versions of parent orientation / position
        mutable Quaternion mLastParentOrientation;
        mutable Vector3 mLastParentPosition;

        /// Pre-calced projection matrix for the specific render system
        mutable Matrix4 mProjMatrixRS;
        /// Pre-calced standard projection matrix but with render system depth range
        mutable Matrix4 mProjMatrixRSDepth;
        /// Pre-calced standard projection matrix
        mutable Matrix4 mProjMatrix;
        /// Pre-calced view matrix
        mutable Matrix4 mViewMatrix;
        /// Something's changed in the frustum shape?
        mutable bool mRecalcFrustum;
        /// Something re the view pos has changed
        mutable bool mRecalcView;
        /// Something re the frustum planes has changed
        mutable bool mRecalcFrustumPlanes;
        /// Something re the world space corners has changed
        mutable bool mRecalcWorldSpaceCorners;
        /// Something re the vertex data has changed
        mutable bool mRecalcVertexData;
		/// Are we using a custom view matrix?
		bool mCustomViewMatrix;
		/// Are we using a custom projection matrix?
		bool mCustomProjMatrix;
		/// Have the frustum extents been manually set?
		bool mFrustumExtentsManuallySet;
		/// Frustum extents
		mutable Real mLeft, mRight, mTop, mBottom;

		
        // Internal functions for calcs
        virtual void calcProjectionParameters(Real& left, Real& right, Real& bottom, Real& top) const;
		/// Update frustum if out of date
        virtual void updateFrustum(void) const;
		/// Update view if out of date
        virtual void updateView(void) const;
		/// Implementation of updateFrustum (called if out of date)
		virtual void updateFrustumImpl(void) const;
		/// Implementation of updateView (called if out of date)
		virtual void updateViewImpl(void) const;
        virtual void updateFrustumPlanes(void) const;
		/// Implementation of updateFrustumPlanes (called if out of date)
		virtual void updateFrustumPlanesImpl(void) const;
        virtual void updateWorldSpaceCorners(void) const;
		/// Implementation of updateWorldSpaceCorners (called if out of date)
		virtual void updateWorldSpaceCornersImpl(void) const;
        virtual void updateVertexData(void) const;
        virtual bool isViewOutOfDate(void) const;
        virtual bool isFrustumOutOfDate(void) const;
        /// Signal to update frustum information.
        virtual void invalidateFrustum(void) const;
        /// Signal to update view information.
        virtual void invalidateView(void) const;

        /// Shared class-level name for Movable type
        static String msMovableType;

        mutable AxisAlignedBox mBoundingBox;
        mutable VertexData mVertexData;

        MaterialPtr mMaterial;
        mutable Vector3 mWorldSpaceCorners[8];

        /// Is this frustum to act as a reflection of itself?
        bool mReflect;
		/// Derived reflection matrix
        mutable Matrix4 mReflectMatrix;
        /// Fixed reflection plane
		mutable Plane mReflectPlane;
		/// Pointer to a reflection plane (automatically updated)
		const MovablePlane* mLinkedReflectPlane;
		/// Record of the last world-space reflection plane info used
		mutable Plane mLastLinkedReflectionPlane;
		
        /// Is this frustum using an oblique depth projection?
		bool mObliqueDepthProjection;
		/// Fixed oblique projection plane
		mutable Plane mObliqueProjPlane;
		/// Pointer to oblique projection plane (automatically updated)
		const MovablePlane* mLinkedObliqueProjPlane;
		/// Record of the last world-space oblique depth projection plane info used
		mutable Plane mLastLinkedObliqueProjPlane;

    public:

        Frustum();
        virtual ~Frustum();
        /** Sets the Y-dimension Field Of View (FOV) of the frustum.
            @remarks
                Field Of View (FOV) is the angle made between the frustum's position, and the edges
                of the 'screen' onto which the scene is projected. High values (90+ degrees) result in a wide-angle,
                fish-eye kind of view, low values (30- degrees) in a stretched, telescopic kind of view. Typical values
                are between 45 and 60 degrees.
            @par
                This value represents the VERTICAL field-of-view. The horizontal field of view is calculated from
                this depending on the dimensions of the viewport (they will only be the same if the viewport is square).
            @note
                Setting the FOV overrides the value supplied for frustum::setNearClipPlane.
         */
        virtual void setFOVy(const Radian& fovy);

        /** Retrieves the frustums Y-dimension Field Of View (FOV).
        */
        virtual const Radian& getFOVy(void) const;

        /** Sets the position of the near clipping plane.
            @remarks
                The position of the near clipping plane is the distance from the frustums position to the screen
                on which the world is projected. The near plane distance, combined with the field-of-view and the
                aspect ratio, determines the size of the viewport through which the world is viewed (in world
                co-ordinates). Note that this world viewport is different to a screen viewport, which has it's
                dimensions expressed in pixels. The frustums viewport should have the same aspect ratio as the
                screen viewport it renders into to avoid distortion.
            @param
                near The distance to the near clipping plane from the frustum in world coordinates.
         */
        virtual void setNearClipDistance(Real nearDist);

        /** Sets the position of the near clipping plane.
        */
        virtual Real getNearClipDistance(void) const;

        /** Sets the distance to the far clipping plane.
            @remarks
                The view frustum is a pyramid created from the frustum position and the edges of the viewport.
                This method sets the distance for the far end of that pyramid. 
                Different applications need different values: e.g. a flight sim
                needs a much further far clipping plane than a first-person 
                shooter. An important point here is that the larger the ratio 
                between near and far clipping planes, the lower the accuracy of
                the Z-buffer used to depth-cue pixels. This is because the
                Z-range is limited to the size of the Z buffer (16 or 32-bit) 
                and the max values must be spread over the gap between near and
                far clip planes. As it happens, you can affect the accuracy far 
                more by altering the near distance rather than the far distance, 
                but keep this in mind.
            @param
                far The distance to the far clipping plane from the frustum in 
                world coordinates.If you specify 0, this means an infinite view
                distance which is useful especially when projecting shadows; but
                be careful not to use a near distance too close.
        */
        virtual void setFarClipDistance(Real farDist);

        /** Retrieves the distance from the frustum to the far clipping plane.
        */
        virtual Real getFarClipDistance(void) const;

        /** Sets the aspect ratio for the frustum viewport.
            @remarks
                The ratio between the x and y dimensions of the rectangular area visible through the frustum
                is known as aspect ratio: aspect = width / height .
            @par
                The default for most fullscreen windows is 1.3333 - this is also assumed by Ogre unless you
                use this method to state otherwise.
        */
        virtual void setAspectRatio(Real ratio);

        /** Retreives the current aspect ratio.
        */
        virtual Real getAspectRatio(void) const;

        /** Sets frustum offsets, used in stereo rendering.
            @remarks
                You can set both horizontal and vertical plane offsets of "eye"; in
                stereo rendering frustum is moved in horizontal plane. To be able to
                render from two "eyes" you'll need two cameras rendering on two
                RenderTargets.
            @par
                The frustum offsets is in world coordinates, and default to (0, 0) - no offsets.
            @param
                offset The horizontal and vertical plane offsets.
        */
        virtual void setFrustumOffset(const Vector2& offset);

        /** Sets frustum offsets, used in stereo rendering.
            @remarks
                You can set both horizontal and vertical plane offsets of "eye"; in
                stereo rendering frustum is moved in horizontal plane. To be able to
                render from two "eyes" you'll need two cameras rendering on two
                RenderTargets.
            @par
                The frustum offsets is in world coordinates, and default to (0, 0) - no offsets.
            @param
                horizontal The horizontal plane offset.
            @param
                vertical The vertical plane offset.
        */
        virtual void setFrustumOffset(Real horizontal = 0.0, Real vertical = 0.0);

        /** Retrieves the frustum offsets.
        */
        virtual const Vector2& getFrustumOffset() const;

        /** Sets frustum focal length (used in stereo rendering).
            @param
                focalLength The distance to the focal plane from the frustum in world coordinates.
        */
        virtual void setFocalLength(Real focalLength = 1.0);

        /** Returns focal length of frustum.
        */
        virtual Real getFocalLength() const;

		/** Manually set the extents of the frustum.
		@param left, right, top, bottom The position where the side clip planes intersect
			the near clip plane, in eye space
		*/
		virtual void setFrustumExtents(Real left, Real right, Real top, Real bottom);
		/** Reset the frustum extents to be automatically derived from other params. */
		virtual void resetFrustumExtents(); 
		/** Get the extents of the frustum in view space. */
		virtual void getFrustumExtents(Real& outleft, Real& outright, Real& outtop, Real& outbottom) const;


        /** Gets the projection matrix for this frustum adjusted for the current
			rendersystem specifics (may be right or left-handed, depth range
			may vary).
        @remarks
            This method retrieves the rendering-API dependent version of the projection
            matrix. If you want a 'typical' projection matrix then use 
            getProjectionMatrix.

        */
        virtual const Matrix4& getProjectionMatrixRS(void) const;
        /** Gets the depth-adjusted projection matrix for the current rendersystem,
			but one which still conforms to right-hand rules.
        @remarks
            This differs from the rendering-API dependent getProjectionMatrix
            in that it always returns a right-handed projection matrix result 
            no matter what rendering API is being used - this is required for
            vertex and fragment programs for example. However, the resulting depth
            range may still vary between render systems since D3D uses [0,1] and 
            GL uses [-1,1], and the range must be kept the same between programmable
            and fixed-function pipelines.
        */
        virtual const Matrix4& getProjectionMatrixWithRSDepth(void) const;
        /** Gets the normal projection matrix for this frustum, ie the 
        projection matrix which conforms to standard right-handed rules and
        uses depth range [-1,+1].
        @remarks
            This differs from the rendering-API dependent getProjectionMatrixRS
            in that it always returns a right-handed projection matrix with depth
            range [-1,+1], result no matter what rendering API is being used - this
            is required for some uniform algebra for example.
        */
        virtual const Matrix4& getProjectionMatrix(void) const;

        /** Gets the view matrix for this frustum. Mainly for use by OGRE internally.
        */
        virtual const Matrix4& getViewMatrix(void) const;

		/** Calculate a view matrix for this frustum, relative to a potentially dynamic point. 
			Mainly for use by OGRE internally when using camera-relative rendering
			for frustums that are not the centre (e.g. texture projection)
		*/
		virtual void calcViewMatrixRelative(const Vector3& relPos, Matrix4& matToUpdate) const;

		/** Set whether to use a custom view matrix on this frustum.
		@remarks
			This is an advanced method which allows you to manually set
			the view matrix on this frustum, rather than having it calculate
			itself based on it's position and orientation. 
		@note
			After enabling a custom view matrix, the frustum will no longer
			update on its own based on position / orientation changes. You 
			are completely responsible for keeping the view matrix up to date.
			The custom matrix will be returned from getViewMatrix.
		@param enable If true, the custom view matrix passed as the second 
			parameter will be used in preference to an auto calculated one. If
			false, the frustum will revert to auto calculating the view matrix.
		@param viewMatrix The custom view matrix to use, the matrix must be an
            affine matrix.
		@see Frustum::setCustomProjectionMatrix, Matrix4::isAffine
		*/
		virtual void setCustomViewMatrix(bool enable, 
			const Matrix4& viewMatrix = Matrix4::IDENTITY);
		/// Returns whether a custom view matrix is in use
		virtual bool isCustomViewMatrixEnabled(void) const 
		{ return mCustomViewMatrix; }
		
		/** Set whether to use a custom projection matrix on this frustum.
		@remarks
			This is an advanced method which allows you to manually set
			the projection matrix on this frustum, rather than having it 
			calculate itself based on it's position and orientation. 
		@note
			After enabling a custom projection matrix, the frustum will no 
			longer update on its own based on field of view and near / far
			distance changes. You are completely responsible for keeping the 
			projection matrix up to date if those values change. The custom 
			matrix will be returned from getProjectionMatrix and derivative
			functions.
		@param enable If true, the custom projection matrix passed as the 
			second parameter will be used in preference to an auto calculated 
			one. If	false, the frustum will revert to auto calculating the 
			projection matrix.
		@param projectionMatrix The custom view matrix to use
		@see Frustum::setCustomViewMatrix
		*/
		virtual void setCustomProjectionMatrix(bool enable, 
			const Matrix4& projectionMatrix = Matrix4::IDENTITY);
		/// Returns whether a custom projection matrix is in use
		virtual bool isCustomProjectionMatrixEnabled(void) const
		{ return mCustomProjMatrix; }

		/** Retrieves the clipping planes of the frustum (world space).
        @remarks
            The clipping planes are ordered as declared in enumerate constants FrustumPlane.
        */
        virtual const Plane* getFrustumPlanes(void) const;

        /** Retrieves a specified plane of the frustum (world space).
            @remarks
                Gets a reference to one of the planes which make up the frustum frustum, e.g. for clipping purposes.
        */
        virtual const Plane& getFrustumPlane( unsigned short plane ) const;

        /** Tests whether the given container is visible in the Frustum.
            @param
                bound Bounding box to be checked (world space)
            @param
                culledBy Optional pointer to an int which will be filled by the plane number which culled
                the box if the result was false;
            @returns
                If the box was visible, true is returned.
            @par
                Otherwise, false is returned.
        */
        virtual bool isVisible(const AxisAlignedBox& bound, FrustumPlane* culledBy = 0) const;

        /** Tests whether the given container is visible in the Frustum.
            @param
                bound Bounding sphere to be checked (world space)
            @param
                culledBy Optional pointer to an int which will be filled by the plane number which culled
                the box if the result was false;
            @returns
                If the sphere was visible, true is returned.
            @par
                Otherwise, false is returned.
        */
        virtual bool isVisible(const Sphere& bound, FrustumPlane* culledBy = 0) const;

        /** Tests whether the given vertex is visible in the Frustum.
            @param
                vert Vertex to be checked (world space)
            @param
                culledBy Optional pointer to an int which will be filled by the plane number which culled
                the box if the result was false;
            @returns
                If the box was visible, true is returned.
            @par
                Otherwise, false is returned.
        */
        virtual bool isVisible(const Vector3& vert, FrustumPlane* culledBy = 0) const;

		/// Overridden from MovableObject::getTypeFlags
		uint32 getTypeFlags(void) const;

        /** Overridden from MovableObject */
        const AxisAlignedBox& getBoundingBox(void) const;

        /** Overridden from MovableObject */
		Real getBoundingRadius(void) const;

		/** Overridden from MovableObject */
        void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from MovableObject */
        const String& getMovableType(void) const;

        /** Overridden from MovableObject */
        void _notifyCurrentCamera(Camera* cam);

        /** Overridden from Renderable */
        const MaterialPtr& getMaterial(void) const;

        /** Overridden from Renderable */
        void getRenderOperation(RenderOperation& op);

        /** Overridden from Renderable */
        void getWorldTransforms(Matrix4* xform) const;

        /** Overridden from Renderable */
        Real getSquaredViewDepth(const Camera* cam) const;

        /** Overridden from Renderable */
        const LightList& getLights(void) const;

        /** Gets the world space corners of the frustum.
        @remarks
            The corners are ordered as follows: top-right near, 
            top-left near, bottom-left near, bottom-right near, 
            top-right far, top-left far, bottom-left far, bottom-right far.
        */
        virtual const Vector3* getWorldSpaceCorners(void) const;

        /** Sets the type of projection to use (orthographic or perspective). Default is perspective.
        */
        virtual void setProjectionType(ProjectionType pt);

        /** Retrieves info on the type of projection used (orthographic or perspective).
        */
        virtual ProjectionType getProjectionType(void) const;

		/** Sets the orthographic window settings, for use with orthographic rendering only. 
		@note Calling this method will recalculate the aspect ratio, use 
			setOrthoWindowHeight or setOrthoWindowWidth alone if you wish to 
			preserve the aspect ratio but just fit one or other dimension to a 
			particular size.
		@param w, h The dimensions of the view window in world units
		*/
		virtual void setOrthoWindow(Real w, Real h);
		/** Sets the orthographic window height, for use with orthographic rendering only. 
		@note The width of the window will be calculated from the aspect ratio. 
		@param h The height of the view window in world units
		*/
		virtual void setOrthoWindowHeight(Real h);
		/** Sets the orthographic window width, for use with orthographic rendering only. 
		@note The height of the window will be calculated from the aspect ratio. 
		@param w The width of the view window in world units
		*/
		virtual void setOrthoWindowWidth(Real w);
		/** Gets the orthographic window height, for use with orthographic rendering only. 
		*/
		virtual Real getOrthoWindowHeight() const;
		/** Gets the orthographic window width, for use with orthographic rendering only. 
		@note This is calculated from the orthographic height and the aspect ratio
		*/
		virtual Real getOrthoWindowWidth() const;

        /** Modifies this frustum so it always renders from the reflection of itself through the
        plane specified.
        @remarks
        This is obviously useful for performing planar reflections. 
        */
        virtual void enableReflection(const Plane& p);
        /** Modifies this frustum so it always renders from the reflection of itself through the
        plane specified. Note that this version of the method links to a plane
		so that changes to it are picked up automatically. It is important that
		this plane continues to exist whilst this object does; do not destroy
		the plane before the frustum.
        @remarks
        This is obviously useful for performing planar reflections. 
        */
        virtual void enableReflection(const MovablePlane* p);

        /** Disables reflection modification previously turned on with enableReflection */
        virtual void disableReflection(void);

        /// Returns whether this frustum is being reflected
        virtual bool isReflected(void) const { return mReflect; }
        /// Returns the reflection matrix of the frustum if appropriate
        virtual const Matrix4& getReflectionMatrix(void) const { return mReflectMatrix; }
        /// Returns the reflection plane of the frustum if appropriate
        virtual const Plane& getReflectionPlane(void) const { return mReflectPlane; }

        /** Project a sphere onto the near plane and get the bounding rectangle. 
        @param sphere The world-space sphere to project
        @param radius Radius of the sphere
        @param left, top, right, bottom Pointers to destination values, these
            will be completed with the normalised device coordinates (in the 
            range {-1,1})
        @returns true if the sphere was projected to a subset of the near plane,
            false if the entire near plane was contained
        */
        virtual bool projectSphere(const Sphere& sphere, 
            Real* left, Real* top, Real* right, Real* bottom) const;


		/** Links the frustum to a custom near clip plane, which can be used
			to clip geometry in a custom manner without using user clip planes.
		@remarks
			There are several applications for clipping a scene arbitrarily by
			a single plane; the most common is when rendering a reflection to 
			a texture, and you only want to render geometry that is above the 
			water plane (to do otherwise results in artefacts). Whilst it is
			possible to use user clip planes, they are not supported on all
			cards, and sometimes are not hardware accelerated when they are
			available. Instead, where a single clip plane is involved, this
			technique uses a 'fudging' of the near clip plane, which is 
			available and fast on all hardware, to perform as the arbitrary
			clip plane. This does change the shape of the frustum, leading 
			to some depth buffer loss of precision, but for many of the uses of
			this technique that is not an issue.
		@par 
			This version of the method links to a plane, rather than requiring
			a by-value plane definition, and therefore you can 
			make changes to the plane (e.g. by moving / rotating the node it is
			attached to) and they will automatically affect this object.
		@note This technique only works for perspective projection.
		@param plane The plane to link to to perform the clipping. This plane
			must continue to exist while the camera is linked to it; do not
			destroy it before the frustum. 
		*/
		virtual void enableCustomNearClipPlane(const MovablePlane* plane);
		/** Links the frustum to a custom near clip plane, which can be used
			to clip geometry in a custom manner without using user clip planes.
		@remarks
			There are several applications for clipping a scene arbitrarily by
			a single plane; the most common is when rendering a reflection to  
			a texture, and you only want to render geometry that is above the 
			water plane (to do otherwise results in artefacts). Whilst it is
			possible to use user clip planes, they are not supported on all
			cards, and sometimes are not hardware accelerated when they are
			available. Instead, where a single clip plane is involved, this
			technique uses a 'fudging' of the near clip plane, which is 
			available and fast on all hardware, to perform as the arbitrary
			clip plane. This does change the shape of the frustum, leading 
			to some depth buffer loss of precision, but for many of the uses of
			this technique that is not an issue.
		@note This technique only works for perspective projection.
		@param plane The plane to link to to perform the clipping. This plane
			must continue to exist while the camera is linked to it; do not
			destroy it before the frustum. 
		*/
		virtual void enableCustomNearClipPlane(const Plane& plane);
		/** Disables any custom near clip plane. */
		virtual void disableCustomNearClipPlane(void);
		/** Is a custom near clip plane in use? */
		virtual bool isCustomNearClipPlaneEnabled(void) const 
		{ return mObliqueDepthProjection; }

		/// @copydoc MovableObject::visitRenderables
		void visitRenderables(Renderable::Visitor* visitor, 
			bool debugRenderables = false);

        /// Small constant used to reduce far plane projection to avoid inaccuracies
        static const Real INFINITE_FAR_PLANE_ADJUST;

		/** Get the derived position of this frustum. */
		virtual const Vector3& getPositionForViewUpdate(void) const;
		/** Get the derived orientation of this frustum. */
		virtual const Quaternion& getOrientationForViewUpdate(void) const;
    };

	/** @} */
	/** @} */

}

#endif 
