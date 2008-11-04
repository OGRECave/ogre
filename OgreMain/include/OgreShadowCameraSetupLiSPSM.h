/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2006 Torus Knot Software Ltd
Copyright (c) 2006 Matthias Fink, netAllied GmbH <matthias.fink@web.de>								
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
#ifndef __ShadowCameraSetupLiSPSM_H__
#define __ShadowCameraSetupLiSPSM_H__

#include "OgrePrerequisites.h"
#include "OgreShadowCameraSetupFocused.h"


namespace Ogre 
{

	/** Implements the Light Space Perspective Shadow Mapping Algorithm.
	@remarks
	Implements the LiSPSM algorithm for an advanced shadow map generation. LiSPSM was
	developed by Michael Wimmer, Daniel Scherzer and Werner Purgathofer of the TU Wien.
	The algorithm was presented on the Eurographics Symposium on Rendering 2004.
	@note
	Shadow mapping was introduced by Williams in 1978. First a depth image is rendered
	from the light's view and compared in a second pass with depth values of the normal 
	camera view. In case the depth camera's depth value is greater than the depth seen
	by the light the fragment lies in the shadow.
	The concept has a major draw back named perspective aliasing. The shadow map distri-
	butes the samples uniformly meaning the position of the viewer is ignored. For the 
	viewer however the perspective projection affects near objects to be displayed 
	bigger than further away objects. The same thing happens with the shadow map texels:
	Near shadows appear very coarse and far away shadows are perfectly sampled.
	In 2002 Stamminger et al. presented an algorithm called Perspective Shadow Maps 
	(PSM). PSM battles the perspective aliasing by distributing 50% of the shadow map 
	texels for objects in the range of <near clipping plane> to <near clipping plane * 2>
	which inverts the problem: The shadows near the viewer are perfectly sampled, 
	however far away shadow may contain aliasing artefacts. A near clipping plane may be
	a problem. But this is not the only one. In the post-perspective space the light 
	sources are non-intuitively mapped: Directional lights may become point light and 
	point lights may become directional lights. Also light sinks (opposite of a light 
	source) may appear.	Another problem are shadow casters located behind the viewer. 
	In post-projective space objects behind the viewer are mapped in front of him with 
	a flipped up-vector.
	LiSPSM battles the light source problem of the post-projective space by rearranging
	the light space before transformation in such a way that no special cases appear. 
	This is done by converting point/spot lights into directional lights. The light 
	space is arranged in such a way that the light direction equals the inverse UNIT_Y.
	In this combination	the directional light will neither change its type nor its 
	direction. Furthermore all visible objects and shadow casters affecting the user's 
	visible area lie in front of the shadow camera: After building the intersection body
	that contains all these objects (body intersection building was introduced with PSM; 
	have a look at the description for the method "calculateB" for further info) a 
	frustum around the body's light space bounding box is created. A parameter (called 
	'n') automatically adjusts the shadow map sample distribution by specifying the 
	frustum's view point - near plane which affects the perspective warp. In case the 
	distance is small the perspecive warp will be strong. As a consequence near objects 
	will gain quality.
	However there are still problems. PSM as well as LiSPSM only devote to minimize
	perspective aliasing. Projection aliasing is still a problem, also 'swimming 
	artefacts' still occur. The LiSPSM quality distribution is very good but not the 
	best available: Some sources say logarithmic shadow mapping is the non plus ultra, 
	however others reject this thought. There is a research project on logarithmic shadow 
	maps. The web page url is http://gamma.cs.unc.edu/logsm/. However there is no techical 
	report available yet (Oct 23rd, 2006).
	@note
	More information can be found on the webpage of the TU Wien: 
	http://www.cg.tuwien.ac.at/research/vr/lispsm/
	@note
	Original implementation by Matthias Fink <matthias.fink@web.de>, 2006.
	*/
	class _OgreExport LiSPSMShadowCameraSetup : public FocusedShadowCameraSetup
	{
	protected:
		/// Warp factor adjustment
		Real mOptAdjustFactor;
		/// Use simple nopt derivation?
		bool mUseSimpleNOpt;
		/// Extra calculated warp factor
		mutable Real mOptAdjustFactorTweak;
		/// Threshold (cos angle) within which to start increasing the opt adjust as camera direction approaches light direction
		Real mCosCamLightDirThreshold;

		/** Calculates the LiSPSM projection matrix P.
		@remarks
		The LiSPSM projection matrix will be built around the axis aligned bounding box 
		of the intersection body B in light space. The distance between the near plane 
		and the projection center is chosen in such a way (distance is set by the para-
		meter n) that the perspective error is the same on the near and far	plane. In 
		case P equals the identity matrix the algorithm falls back to a uniform shadow
		mapping matrix.
		@param lightSpace: matrix of the light space transformation
		@param bodyB: intersection body B
		@param bodyLVS: intersection body LVS (relevant space in front of the camera)
		@param sm: scene manager
		@param cam: currently active camera
		@param light: currently active light
		*/
		Matrix4 calculateLiSPSM(const Matrix4& lightSpace, const PointListBody& bodyB, 
			const PointListBody& bodyLVS, const SceneManager& sm, 
			const Camera& cam, const Light& light) const;

		/** Calculates the distance between camera position and near clipping plane.
		@remarks
		n_opt determines the distance between light space origin (shadow camera position)
		and	the near clipping plane to achieve an optimal perspective foreshortening effect.
		In this	way the texel distribution over the shadow map is controlled.

		Formula:
		               d
		n_opt = ---------------
		        sqrt(z1/z0) - 1

		Parameters:
		d: distance between the near and the far clipping plane
		z0: located on the near clipping plane of the intersection body b
		z1: located on the far clipping plane with the same x/y values as z0		
		@note
		A positive value is applied as the distance between viewer and near clipping plane.
		In case null is returned uniform shadow mapping will be applied.
		@param lightSpace: matrix of the light space transformation
		@param bodyBABB_ls: bounding box of the tranformed (light space) bodyB
		@param bodyLVS: point list of the bodyLVS which describes the scene space which is in 
		front of the light and the camera
		@param cam: currently active camera
		*/
		Real calculateNOpt(const Matrix4& lightSpace, const AxisAlignedBox& bodyBABB_ls, 
			const PointListBody& bodyLVS, const Camera& cam) const;

		/** Calculates a simpler version than the one above.
		*/
		Real calculateNOptSimple(const PointListBody& bodyLVS, 
			const Camera& cam) const;

		/** Calculates the visible point on the near plane for the n_opt calculation
		@remarks
		z0 lies on the parallel plane to the near plane through e and on the near plane of 
		the frustum C (plane z = bodyB_zMax_ls) and on the line x = e.x.
		@param lightSpace: matrix of the light space transformation
		@param e: the LiSPSM parameter e is located near or on the near clipping plane of the 
		LiSPSM frustum C
		@param bodyB_zMax_ls: maximum z-value of the light space bodyB bounding box
		@param cam: currently active camera
		*/
		Vector3 calculateZ0_ls(const Matrix4& lightSpace, const Vector3& e, Real bodyB_zMax_ls, 
			const Camera& cam) const;

		/** Builds a frustum matrix.
		@remarks
		Builds a standard frustum matrix out of the distance infos of the six frustum 
		clipping planes.
		*/
		Matrix4 buildFrustumProjection(Real left, Real right, Real bottom, 
			Real top, Real near, Real far) const;

	public:
		/** Default constructor.
		@remarks
		Nothing done here.
		*/
		LiSPSMShadowCameraSetup(void);

		/** Default destructor.
		@remarks
		Nothing done here.
		*/
		virtual ~LiSPSMShadowCameraSetup(void);

		/** Returns a LiSPSM shadow camera.
		@remarks
		Builds and returns a LiSPSM shadow camera. 
		More information can be found on the webpage of the TU Wien: 
		http://www.cg.tuwien.ac.at/research/vr/lispsm/
		*/
		virtual void getShadowCamera(const SceneManager *sm, const Camera *cam, 
			const Viewport *vp, const Light *light, Camera *texCam, size_t iteration) const;

		/** Adjusts the parameter n to produce optimal shadows.
		@remarks
		The smaller the parameter n, the stronger the perspective warping effect.
		The consequence of a stronger warping is that the near shadows will gain 
		quality while the far ones will lose it. Depending on your scene and light
		types you may want to tweak this value - for example directional lights
		tend to benefit from higher values of n than other types of light, 
		especially if you expect to see more distant shadows (say if the viewpoint is
		higher above the ground plane). Remember that you can supply separate
		ShadowCameraSetup instances configured differently per light if you wish.
		@param n The adjustment factor - default is 0.1f. 
		*/
		virtual void setOptimalAdjustFactor(Real n) { mOptAdjustFactor = n; }
		/** Get the parameter n used to produce optimal shadows. 
		@see setOptimalAdjustFactor
		*/
		virtual Real getOptimalAdjustFactor() const { return mOptAdjustFactor; }
		/** Sets whether or not to use a slightly simpler version of the 
			camera near point derivation (default is true)
		*/
		virtual void setUseSimpleOptimalAdjust(bool s) { mUseSimpleNOpt = s; }
		/** Gets whether or not to use a slightly simpler version of the 
		camera near point derivation (default is true)
		*/
		virtual bool getUseSimpleOptimalAdjust() const { return mUseSimpleNOpt; }

		/** Sets the threshold between the camera and the light direction below
			which the LiSPSM projection is 'flattened', since coincident light
			and camera projections cause problems with the perspective skew.
			@remarks
			For example, setting this to 20 degrees will mean that as the difference 
			between the light and camera direction reduces from 20 degrees to 0
			degrees, the perspective skew will be proportionately removed.
		*/
		virtual void setCameraLightDirectionThreshold(Degree angle);

		/** Sets the threshold between the camera and the light direction below
		which the LiSPSM projection is 'flattened', since coincident light
		and camera projections cause problems with the perspective skew.
		*/
		virtual Degree getCameraLightDirectionThreshold() const;


	};

}

#endif

