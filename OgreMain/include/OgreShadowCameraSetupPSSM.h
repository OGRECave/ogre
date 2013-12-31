/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Copyright (c) 2006 Matthias Fink, netAllied GmbH <matthias.fink@web.de>								

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
#ifndef __ShadowCameraSetupPSSM_H__
#define __ShadowCameraSetupPSSM_H__

#include "OgrePrerequisites.h"
#include "OgreShadowCameraSetupLiSPSM.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/
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
		typedef vector<Real>::type SplitPointList;
		typedef vector<Real>::type OptimalAdjustFactorList;

	protected:
		uint mSplitCount;
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
		void calculateSplitPoints(uint splitCount, Real nearDist, Real farDist, Real lambda = 0.95);

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
		uint getSplitCount() const { return mSplitCount; }

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
	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
