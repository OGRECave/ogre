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
#ifndef __ShadowCameraSetupPlaneOptimal_H__
#define __ShadowCameraSetupPlaneOptimal_H__

#include "OgrePrerequisites.h"
#include "OgreShadowCameraSetup.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/
	/** Implements the plane optimal shadow camera algorithm.
        @remarks
            Given a plane of interest, it is possible to set up the shadow camera
			matrix such that the mapping between screen and shadow map is the identity
			(when restricted to pixels that view the plane of interest).  Therefore,
			if the shadow map resolution matches the screen space resolution (of the 
			seen planar receiver), we can get pixel perfect shadowing on the plane. 
			Off the plane, the shadowing is not guaranteed to be perfect and will 
			likely exhibit the usual sampling artifacts associated with shadow mapping.
		@note Important: this routine requires double-precision calculations. When you
			are running under Direct3D, you must ensure that you set the floating
			point mode to 'Consistent' rather than 'Fastest' to ensure this precision.
			This does allegedly come with some performance cost but when measuring 
			it appears to be negligible in modern systems for normal usage.
		@note Second important note: this projection also only works for lights with
			a finite position. Therefore you cannot use it for directional lights
			at this time.
    */
	class _OgreExport PlaneOptimalShadowCameraSetup : public ShadowCameraSetup
	{
	private:
		MovablePlane* mPlane;	///< pointer to plane of interest
	private:
		PlaneOptimalShadowCameraSetup() {}	///< Default constructor is private

		/// helper function computing projection matrix given constraints
		Matrix4 computeConstrainedProjection( const Vector4& pinhole, 
											  const vector<Vector4>::type& fpoint, 
											  const vector<Vector2>::type& constraint) const;

	public:
		/// Constructor -- requires a plane of interest
		PlaneOptimalShadowCameraSetup(MovablePlane *plane);
		/// Destructor
		virtual ~PlaneOptimalShadowCameraSetup();

		/// Returns shadow camera configured to get 1-1 homography between screen and shadow map when restricted to plane
		virtual void getShadowCamera (const SceneManager *sm, const Camera *cam, 
									  const Viewport *vp, const Light *light, Camera *texCam, size_t iteration) const;
	};
	/** @} */
	/** @} */

}

#include "OgreHeaderSuffix.h"

#endif
