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
#ifndef __ShadowCameraSetupPSSM_H__
#define __ShadowCameraSetupPSSM_H__

#include "OgrePrerequisites.h"
#include "OgreShadowCameraSetupLiSPSM.h"

namespace Ogre
{

	/** Parallel Split Shadow Map (PSSM) shadow camera setup. 
	@remarks
		A PSSM shadow system uses multiple shadow maps per light and maps each
		texture into a region of space, progressing away from the camera. As such
		it is most appropriate for directional light setups. This particular version
		also uses LiSPSM projection for each split to maximise the quality. 
	@note
		Because PSSM uses multiple shadow maps per light, you will need to increase
		the number of shadow textures available (via SceneManager) to match the 
		number of shadow maps required (default is 3 per light). 
	*/
	class _OgreExport PSSMShadowCameraSetup : public Ogre::LiSPSMShadowCameraSetup
	{
	public:
		typedef std::vector<Real> SplitPointList;
		typedef std::vector<Real> OptimalAdjustFactorList;

	protected:
		size_t mSplitCount;
		SplitPointList mSplitPoints;
		OptimalAdjustFactorList mOptimalAdjustFactors;
		Real mSplitPadding;

		mutable size_t mCurrentIteration;

	public:
		/// Constructor, defaults to 3 splits
		PSSMShadowCameraSetup();
		~PSSMShadowCameraSetup();

		/** Calculate a new splitting scheme.
		@param splitCount The number of splits to use
		@param nearDist The near plane to use for the first split
		@param farDist The far plane to use for the last split
		@param lambda Factor to use to reduce the split size 
		*/
		void calculateSplitPoints(size_t splitCount, Real nearDist, Real farDist, Real lambda = 0.95);

		/** Manually configure a new splitting scheme.
		@param newSplitPoints A list which is splitCount + 1 entries long, containing the
			split points. The first value is the near point, the last value is the
			far point, and each value in between is both a far point of the previous
			split, and a near point for the next one.
		*/
		void setSplitPoints(const SplitPointList& newSplitPoints);

		/** Set the LiSPSM optimal adjust factor for a given split (call after
			configuring splits).
		*/
		void setOptimalAdjustFactor(size_t splitIndex, Real factor);

		/** Set the padding factor to apply to the near & far distances when matching up
			splits to one another, to avoid 'cracks'.
		*/
		void setSplitPadding(Real pad) { mSplitPadding = pad; }

		/** Get the padding factor to apply to the near & far distances when matching up
			splits to one another, to avoid 'cracks'.
		*/
		Real getSplitPadding() const { return mSplitPadding; }
		/// Get the number of splits. 
		size_t getSplitCount() const { return mSplitCount; }

		/// Returns a LiSPSM shadow camera with PSSM splits base on iteration.
		virtual void getShadowCamera(const Ogre::SceneManager *sm, const Ogre::Camera *cam,
			const Ogre::Viewport *vp, const Ogre::Light *light, Ogre::Camera *texCam, size_t iteration) const;

		/// Returns the calculated split points.
		inline const SplitPointList& getSplitPoints() const
		{ return mSplitPoints; }

		/// Returns the optimal adjust factor for a given split.
		inline Real getOptimalAdjustFactor(size_t splitIndex) const
		{ return mOptimalAdjustFactors[splitIndex]; }

		/// Overridden, recommended internal use only since depends on current iteration
		Real getOptimalAdjustFactor() const;

	};
}
#endif
