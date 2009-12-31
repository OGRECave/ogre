/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __XSISKELETONEXPORTER_H__
#define __XSISKELETONEXPORTER_H__

#include "OgrePrerequisites.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreAxisAlignedBox.h"
#include "OgreXSIHelper.h"
#include <xsi_x3dobject.h>
#include <xsi_string.h>
#include <xsi_application.h>
#include <xsi_actionsource.h>
#include <xsi_mixer.h>


namespace Ogre {

	/** Class for performing a skeleton export from XSI.
	*/
	class XsiSkeletonExporter
	{
	public:
		XsiSkeletonExporter();
		virtual ~XsiSkeletonExporter();


		/** Export a skeleton to the provided filename.
		@param skeletonFileName The file name to export to
		@param deformers The list of deformers (bones) found during mesh traversal
		@param framesPerSecond The number of frames per second
		@param animList List of animation splits
		@returns AABB derived from bone animations, should be used to pad mesh bounds
		*/
		const AxisAlignedBox& exportSkeleton(const String& skeletonFileName, 
			DeformerMap& deformers, float framesPerSecond, 
			AnimationList& animList);
	protected:
		// XSI Objects
		XSI::Application mXsiApp;
		XSI::X3DObject mXsiSceneRoot;
		std::map<String, int> mXSITrackTypeNames; 
		// Lower-case version of deformer map (XSI seems to be case insensitive and
		// some animations rely on that!)
		DeformerMap mLowerCaseDeformerMap;
		// Actions created as part of IK sampling, will be deleted afterward
		XSI::CStringArray mIKSampledAnimations;
		AxisAlignedBox mAABB;

		/// Build the bone hierarchy from a simple list of bones
		void buildBoneHierarchy(Skeleton* pSkeleton, DeformerMap& deformers, 
			AnimationList& animList);
		/** Link the current bone with it's parent
		*/
		void linkBoneWithParent(DeformerEntry* deformer, 
			DeformerMap& deformers, std::list<DeformerEntry*>& deformerList);
		/** Validate and create a bone, or eliminate the current bone if it 
			has no animated parameters
		*/
		void validateAsBone(Skeleton* pSkeleton, DeformerEntry* deformer, 
			DeformerMap& deformers, std::list<DeformerEntry*>& deformerList, 
			AnimationList& animList);
		/// Process an action source
		void processActionSource(const XSI::ActionSource& source, DeformerMap& deformers);
		/// Bake animations
		void createAnimations(Skeleton* pSkel, DeformerMap& deformers, 
			float framesPerSecond, AnimationList& animList, AxisAlignedBox& AABBPadding);
		/// Bake animation tracks by sampling
		void createAnimationTracksSampled(Animation* pAnim, AnimationEntry& animEntry, 
			DeformerMap& deformers, float fps, AxisAlignedBox& AABBPadding);

		void cleanup(void);
		void copyDeformerMap(DeformerMap& deformers);
		/// Get deformer from passed in map or lower case version
		DeformerEntry* getDeformer(const String& name, DeformerMap& deformers);
		// Sample all bones, and also sample max global bone position for AABB padding
		void sampleAllBones(DeformerMap& deformers, 
			std::vector<NodeAnimationTrack*> deformerTracks, double frame, 
			Real time, float fps, AxisAlignedBox& AABBPadding);
		void establishInitialTransforms(DeformerMap& deformers);

	};



}


#endif
