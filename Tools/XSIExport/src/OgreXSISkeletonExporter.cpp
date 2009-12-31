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
#include "OgreXSISkeletonExporter.h"
#include "OgreResourceGroupManager.h"
#include "OgreSkeletonManager.h"
#include "OgreSkeleton.h"
#include "OgreBone.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreSkeletonSerializer.h"
#include "OgreQuaternion.h"
#include <xsi_model.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_math.h>
#include <xsi_rotation.h>
#include <xsi_animationsourceitem.h>
#include <xsi_source.h>
#include <xsi_fcurve.h>
#include <xsi_fcurvekey.h>
#include <xsi_time.h>
#include <xsi_chaineffector.h>
#include <xsi_chainroot.h>
#include <xsi_chainbone.h>
#include <xsi_matrix4.h>
#include <xsi_transformation.h>
#include <xsi_vector3.h>
#include <xsi_constraint.h>
#include <xsi_track.h>
#include <xsi_clip.h>
#include <xsi_selection.h>
#include <xsi_statickinematicstate.h>

using namespace XSI;

namespace Ogre
{
	//-----------------------------------------------------------------------------
	XsiSkeletonExporter::XsiSkeletonExporter()
	{
		mXsiSceneRoot = X3DObject(mXsiApp.GetActiveSceneRoot());
		mXSITrackTypeNames["posx"] = XTT_POS_X;
		mXSITrackTypeNames["posy"] = XTT_POS_Y;
		mXSITrackTypeNames["posz"] = XTT_POS_Z;
		mXSITrackTypeNames["rotx"] = XTT_ROT_X;
		mXSITrackTypeNames["roty"] = XTT_ROT_Y;
		mXSITrackTypeNames["rotz"] = XTT_ROT_Z;
		mXSITrackTypeNames["sclx"] = XTT_SCL_X;
		mXSITrackTypeNames["scly"] = XTT_SCL_Y;
		mXSITrackTypeNames["sclz"] = XTT_SCL_Z;
	}
	//-----------------------------------------------------------------------------
	XsiSkeletonExporter::~XsiSkeletonExporter()
	{
	}
	//-----------------------------------------------------------------------------
	const AxisAlignedBox& XsiSkeletonExporter::exportSkeleton(const String& skeletonFileName, 
		DeformerMap& deformers, float framesPerSecond, AnimationList& animList)
	{
		LogOgreAndXSI(L"** Begin OGRE Skeleton Export **");

		copyDeformerMap(deformers);

		SkeletonPtr skeleton = SkeletonManager::getSingleton().create("export",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		// construct the hierarchy
		buildBoneHierarchy(skeleton.get(), deformers, animList);

		// progress report
		ProgressManager::getSingleton().progress();

		establishInitialTransforms(deformers);

		// create animations 
		mAABB.setNull();
		createAnimations(skeleton.get(), deformers, framesPerSecond, animList, mAABB);
		// progress report
		ProgressManager::getSingleton().progress();

		// Optimise
		skeleton->optimiseAllAnimations();

		SkeletonSerializer ser;
		ser.exportSkeleton(skeleton.get(), skeletonFileName);
		// progress report
		ProgressManager::getSingleton().progress();

		LogOgreAndXSI(L"** OGRE Skeleton Export Complete **");

		cleanup();

		return mAABB;

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::copyDeformerMap(DeformerMap& deformers)
	{
		// Make lower-case version
		// some XSI animations appear to like to use case insensitive references :(
		for (DeformerMap::iterator i = deformers.begin(); i != deformers.end(); ++i)
		{
			DeformerEntry* deformer = i->second;
			String name = XSItoOgre(deformer->obj.GetName());
			StringUtil::toLowerCase(name);
			mLowerCaseDeformerMap[name] = deformer;
		}
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::buildBoneHierarchy(Skeleton* pSkeleton, 
		DeformerMap& deformers, AnimationList& animList)
	{
		/// Copy all entries from map into a list so iterators won't get invalidated
		std::list<DeformerEntry*> deformerList;
		LogOgreAndXSI(L"-- Bones with vertex assignments:");
		for (DeformerMap::iterator i = deformers.begin(); i != deformers.end(); ++i)
		{
			DeformerEntry* deformer = i->second;
			deformerList.push_back(deformer);
			LogOgreAndXSI(deformer->obj.GetName());
		}

		/* XSI allows you to use any object at all as a bone, not just chain elements.
		   A typical choice is a hierarchy of nulls, for example. In order to 
		   build a skeleton hierarchy which represents the actual one, we need
		   to find the relationships between all the deformers that we found.
		   
		   Well do this by navigating up the scene tree from each bone, looking for
		   a match in the existing bone list or creating a new one where we need it
		   to reach the root. We add bones even if they're not assigned vertices
		   because the animation may depend on them. If the
		   traversal hits the scene root this bone is clearly a root bone 
		   (there may be more than one). 
	   */
		for (std::list<DeformerEntry*>::iterator i = deformerList.begin(); i != deformerList.end(); ++i)
		{
			DeformerEntry* deformer = *i;
			if (deformer->parentName.empty())
			{
				linkBoneWithParent(deformer, deformers, deformerList);
			}
		}

		// Now eliminate all bones without any animated kine parameters
		// Need to do this after we've determined all relationships
		for (std::list<DeformerEntry*>::iterator i = deformerList.begin(); i != deformerList.end(); ++i)
		{
			DeformerEntry* deformer = *i;
			validateAsBone(pSkeleton, deformer, deformers, deformerList, animList);
		}

		// Now link
		for (DeformerMap::iterator i = deformers.begin(); i != deformers.end(); ++i)
		{
			DeformerEntry* deformer = i->second;

			// link to parent
			if (!deformer->parentName.empty())
			{
				DeformerEntry* parent = getDeformer(deformer->parentName, deformers);
				assert (parent && "Parent not found");
				assert (deformer->pBone && "Child bone not created");
				assert(parent->pBone && "Parent bone not created");
				parent->pBone->addChild(deformer->pBone);

			}
		}

	}
	//-----------------------------------------------------------------------------
	DeformerEntry* XsiSkeletonExporter::getDeformer(const String& name, 
		DeformerMap& deformers)
	{
		// Look in case sensitive list first
		DeformerMap::iterator i = deformers.find(name);
		if (i == deformers.end())
		{
			String lcaseName = name;
			StringUtil::toLowerCase(lcaseName);
			i = mLowerCaseDeformerMap.find(lcaseName);
			if (i == mLowerCaseDeformerMap.end())
			{
				return 0;
			}
			else
			{
				return i->second;
			}
		}
		else
		{
			return i->second;
		}

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::linkBoneWithParent(DeformerEntry* child, 
		DeformerMap& deformers, std::list<DeformerEntry*>& deformerList)
	{
		X3DObject parent(child->obj.GetParent());
		String childName = XSItoOgre(child->obj.GetName());

		if (child->obj == mXsiSceneRoot /* safety check for start node */)
			return;

		// Check for parenting by a chain end effector
		// These are sneaky little buggers - we actually want to attach the
		// child to the end of the final bone in the chain
		if (parent.IsA(XSI::siChainEffectorID))
		{
			ChainEffector effector(parent);
			CRefArray chainBones = effector.GetRoot().GetBones();
			// get the last
			parent = chainBones[chainBones.GetCount()-1];
			child->parentIsChainEndEffector = true;
			
		}
		// is the parent the scene root?
		if (parent == mXsiSceneRoot)
		{
			// we hit the root node
		}
		else
		{

			String parentName = XSItoOgre(parent.GetName());
			// Otherwise, check to see if the parent is in the deformer list
			DeformerEntry* parentDeformer = getDeformer(parentName, deformers);
			if (!parentDeformer)
			{
				// not found, create entry for parent 
				parentDeformer = new DeformerEntry(deformers.size(), parent);
				deformers[parentName] = parentDeformer;
				deformerList.push_back(parentDeformer);
				LogOgreAndXSI(CString(L"Added ") + parent.GetName() + 
					CString(L" as a parent of ") + child->obj.GetName() );
			}

			// Link child entry with parent (not bone yet)
			// link child to parent by name
			child->parentName = parentName;
			parentDeformer->childNames.push_back(childName);




		}

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::validateAsBone(Skeleton* pSkeleton, 
		DeformerEntry* deformer, 
		DeformerMap& deformers, std::list<DeformerEntry*>& deformerList, 
		AnimationList& animList)
	{
		/* The purpose of this method is to find out whether a node in the 
		   bone hierarchy is animated, and if not, to eliminate it and propagate
		   it's static transform contribution to it's children.
		   We do this because it's quite easy in XSI to build deep bone chains
		   with intermediate points that are only used for manipulation. We
		   don't want to include all of those.
	   */

		// TODO


		// if we weren't static, create bone
		if (!deformer->pBone)
		{
			String name = XSItoOgre(deformer->obj.GetName());
			deformer->pBone = pSkeleton->createBone(name, deformer->boneID);
			MATH::CTransformation trans; 

			if (deformer->parentName.empty())
			{
				// set transform on bone to global transform since no parents
				trans = deformer->obj.GetKinematics().GetGlobal().GetTransform();
			}
			else
			{
				// set transform on bone to local transform (since child)
				trans = deformer->obj.GetKinematics().GetLocal().GetTransform();
			}
			deformer->pBone->setPosition(XSItoOgre(trans.GetTranslation()));
			deformer->pBone->setOrientation(XSItoOgre(trans.GetRotation().GetQuaternion()));
			deformer->pBone->setScale(XSItoOgre(trans.GetScaling()));

			// special case a bone which is parented by a chain end
			if (deformer->parentIsChainEndEffector)
			{
				ChainEffector effector(deformer->obj.GetParent());
				CRefArray chainBones = effector.GetRoot().GetBones();
				// get the last
				X3DObject endBone = chainBones[chainBones.GetCount()-1];
				// offset along X the length of the bone
				double boneLen = endBone.GetParameterValue(L"Length");
				deformer->pBone->setPosition(
					deformer->pBone->getPosition() + Vector3::UNIT_X * boneLen);
			}

		}

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::processActionSource(const XSI::ActionSource& actSource,
		DeformerMap& deformers)
	{
		// Clear existing deformer links
		for(DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			for (int tt = XTT_POS_X; tt < XTT_COUNT; ++tt)
			{
				di->second->xsiTrack[tt].ResetObject();
			}
		}
		// Get all the items
		CRefArray items = actSource.GetItems();
		for (int i = 0; i < items.GetCount(); ++i)
		{
			XSI::AnimationSourceItem item = items[i];

			// Check the target
			String target = XSItoOgre(item.GetTarget());
			size_t firstDotPos = target.find_first_of(".");
			size_t lastDotPos = target.find_last_of(".");
			if (firstDotPos != String::npos && lastDotPos != String::npos)
			{
				String targetName = target.substr(0, firstDotPos);
				String paramName = target.substr(lastDotPos+1, 
					target.size() - lastDotPos - 1);
				// locate deformer
				DeformerEntry* deformer = getDeformer(targetName, deformers);
				if (deformer)
				{
					// determine parameter
					std::map<String, int>::iterator pi = mXSITrackTypeNames.find(paramName);
					if (pi != mXSITrackTypeNames.end())
					{
						deformer->xsiTrack[pi->second] = item;
						deformer->hasAnyTracks = true;
					}
				}
			}

		}
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::createAnimations(Skeleton* pSkel, 
		DeformerMap& deformers, float fps, AnimationList& animList, AxisAlignedBox& AABBPadding)
	{
		for (AnimationList::iterator ai = animList.begin(); ai != animList.end(); ++ai)
		{
			AnimationEntry& animEntry = *ai;

			// Note that we don't know if this time period includes bone animation
			// but we sample it anyway just in case; animation optimisation will
			// eliminate anything that's redundant
			// A little wasteful perhaps, but it's the only guaranteed way to pick
			// up all the potentially derived effects on deformers

			float animLength = (float)(animEntry.endFrame - animEntry.startFrame) / fps;
			StringUtil::StrStreamType str;
			str << "Creating animation " << animEntry.animationName << 
				" with length " << animLength << " seconds";
			LogOgreAndXSI(str.str());
			Animation* anim = pSkel->createAnimation(animEntry.animationName, animLength);

			createAnimationTracksSampled(anim, animEntry, deformers, fps, AABBPadding);
			
		}
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::createAnimationTracksSampled(Animation* pAnim, 
		AnimationEntry& animEntry, DeformerMap& deformers, float fps, AxisAlignedBox& AABBPadding)
	{

		// Create all tracks first
		std::vector<NodeAnimationTrack*> deformerTracks;
		deformerTracks.resize(deformers.size());
		for (DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			DeformerEntry* deformer = di->second;
			NodeAnimationTrack* track = pAnim->createNodeTrack(deformer->boneID, deformer->pBone);
			deformerTracks[deformer->boneID] = track;
		}

		// Iterate over frames, keying as we go
		long numFrames = animEntry.endFrame - animEntry.startFrame;
		if (animEntry.ikSampleInterval == 0)
		{
			// Don't allow zero samplign frequency - infinite loop!
			animEntry.ikSampleInterval = 5.0f;
		}

		// Sample all bones from start to before the end frame
		for (long frame = animEntry.startFrame; frame < animEntry.endFrame; 
			frame += animEntry.ikSampleInterval)
		{
			Real time = (float)(frame - animEntry.startFrame) / fps;
			sampleAllBones(deformers, deformerTracks, frame, time, fps, AABBPadding);

		}
		// sample final frame (must be guaranteed to be done)
		Real time = (float)(animEntry.endFrame - animEntry.startFrame) / fps;
		sampleAllBones(deformers, deformerTracks, animEntry.endFrame, time, fps, AABBPadding);


	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::establishInitialTransforms(DeformerMap& deformers)
	{
		for (DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			DeformerEntry* deformer = di->second;
			if (deformer->pBone->getParent() == 0)
			{
				// Based on global
				deformer->initialXform = 
					deformer->obj.GetKinematics().GetGlobal().GetTransform();
			}
			else
			{
				// Based on local
				deformer->initialXform = 
					deformer->obj.GetKinematics().GetLocal().GetTransform();
			}

		}
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::sampleAllBones(DeformerMap& deformers, 
		std::vector<NodeAnimationTrack*> deformerTracks, double frame, 
		Real time, float fps, AxisAlignedBox& AABBPadding)
	{
		CValueArray args;
		CValue dummy;
		args.Resize(2);
		// set the playcontrol 
		args[0] = L"PlayControl.Key";
		args[1] = frame;
		mXsiApp.ExecuteCommand(L"SetValue", args, dummy);
		args[0] = L"PlayControl.Current";
		mXsiApp.ExecuteCommand(L"SetValue", args, dummy);

		// Refresh
		mXsiApp.ExecuteCommand(L"Refresh", CValueArray(), dummy);
		// Sample all bones
		for (DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			DeformerEntry* deformer = di->second;
			NodeAnimationTrack* track = deformerTracks[deformer->boneID];

			double initposx, initposy, initposz;
			deformer->initialXform.GetTranslationValues(initposx, initposy, initposz);
			double initrotx, initroty, initrotz;
			deformer->initialXform.GetRotation().GetXYZAngles(initrotx, initroty, initrotz);
			double initsclx, initscly, initsclz;
			deformer->initialXform.GetScalingValues(initsclx, initscly, initsclz);
			XSI::MATH::CMatrix4 invTrans = deformer->initialXform.GetMatrix4();
			invTrans.InvertInPlace();

			XSI::MATH::CTransformation transformation;
			if (deformer->pBone->getParent() == 0)
			{
				// Based on global
				transformation = 
					deformer->obj.GetKinematics().GetGlobal().GetTransform();
			}
			else
			{
				// Based on local
				transformation = 
					deformer->obj.GetKinematics().GetLocal().GetTransform();
			}

			double posx, posy, posz;
			transformation.GetTranslationValues(posx, posy, posz);
			double sclx, scly, sclz;
			transformation.GetScalingValues(sclx, scly, sclz);

			// Make relative to initial
			XSI::MATH::CMatrix4 transformationMatrix = transformation.GetMatrix4();
			transformationMatrix.MulInPlace(invTrans);
			transformation.SetMatrix4(transformationMatrix);

			// create keyframe
			TransformKeyFrame* kf = track->createNodeKeyFrame(time);
			// not sure why inverted transform doesn't work for position, but it doesn't
			// I thought XSI used same transform order as OGRE
			kf->setTranslate(Vector3(posx - initposx, posy - initposy, posz - initposz));
			kf->setRotation(XSItoOgre(transformation.GetRotationQuaternion()));
			kf->setScale(Vector3(sclx / initsclx, scly / initscly, sclz / initsclz));

			// Derive AABB of bone positions, for padding animated mesh AABB
			XSI::MATH::CVector3 bonePos = 
				deformer->obj.GetKinematics().GetGlobal().GetTransform().GetTranslation();
			AABBPadding.merge(XSItoOgre(bonePos));



		}

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::cleanup(void)
	{

		mLowerCaseDeformerMap.clear();

		CValueArray args;
		CValue dummy;
		args.Resize(1);

		for (int i = 0; i < mIKSampledAnimations.GetCount(); ++i)
		{
			args[0] = mIKSampledAnimations[i];
			mXsiApp.ExecuteCommand(L"DeleteObj", args, dummy);
		}
		mIKSampledAnimations.Clear();

	}
}
