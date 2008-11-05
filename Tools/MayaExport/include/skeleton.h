////////////////////////////////////////////////////////////////////////////////
// skeleton.h
// Author       : Francesco Giordana
// Sponsored by : Anygma N.V. (http://www.nazooka.com)
// Start Date   : January 13, 2005
// Copyright    : (C) 2006 by Francesco Giordana
// Email        : fra.giordana@tiscali.it
////////////////////////////////////////////////////////////////////////////////

/*********************************************************************************
*                                                                                *
*   This program is free software; you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or            *
*   (at your option) any later version.                                          *
*                                                                                *
**********************************************************************************/

#ifndef _SKELETON_H
#define _SKELETON_H

#include "mayaExportLayer.h"
#include "paramList.h"
#include "animation.h"

namespace OgreMayaExporter
{
	/***** structure to hold joint info *****/
	typedef struct jointTag
	{
		MString name;
		int id;
		MMatrix localMatrix;
		MMatrix bindMatrix;
		int parentIndex;
		double posx,posy,posz;
		double angle;
		double axisx,axisy,axisz;
		float scalex,scaley,scalez;
		MDagPath jointDag;
	} joint;


	/*********** Class Skeleton **********************/
	class Skeleton
	{
	public:
		//constructor
		Skeleton();
		//destructor
		~Skeleton();
		//clear skeleton data
		void clear();
		//load skeleton data
		MStatus load(MFnSkinCluster* pSkinCluster,ParamList& params);
		//load skeletal animations
		MStatus loadAnims(ParamList& params);
		//get joints
		std::vector<joint>& getJoints();
		//get animations
		std::vector<Animation>& getAnimations();
		//restore skeleton pose
		void restorePose();
		//write to an OGRE binary skeleton
		MStatus writeOgreBinary(ParamList &params);

	protected:
		//load a joint
		MStatus loadJoint(MDagPath& jointDag, joint* parent, ParamList& params,MFnSkinCluster* pSkinCluster);
		//load a clip
		MStatus loadClip(MString clipName,float start,float stop,float rate,ParamList& params);
		//load a keyframe for a particular joint at current time
		skeletonKeyframe loadKeyframe(joint& j,float time,ParamList& params);
		//write joints to an Ogre skeleton
		MStatus createOgreBones(Ogre::SkeletonPtr pSkeleton,ParamList& params);
		// write skeleton animations to an Ogre skeleton
		MStatus createOgreSkeletonAnimations(Ogre::SkeletonPtr pSkeleton,ParamList& params);

		std::vector<joint> m_joints;
		std::vector<Animation> m_animations;
		std::vector<int> m_roots;
		MString m_restorePose;
	};

}	//end namespace

#endif
