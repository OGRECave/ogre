////////////////////////////////////////////////////////////////////////////////
// skeleton.cpp
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

#include "skeleton.h"
#include "submesh.h"
#include <maya/MFnMatrixData.h>
#include <maya/M3dView.h>

namespace OgreMayaExporter
{
    // Constructor
    Skeleton::Skeleton()
    {
        m_joints.clear();
        m_animations.clear();
        m_restorePose = "";
    }


    // Destructor
    Skeleton::~Skeleton()
    {
        clear();
    }


    // Clear skeleton data
    void Skeleton::clear()
    {
        m_joints.clear();
        m_animations.clear();
        m_restorePose = "";
    }


    // Load skeleton data from given skin cluster
    MStatus Skeleton::load(MFnSkinCluster* pSkinCluster,ParamList& params)
    {
        MStatus stat;
        //check for valid skin cluster pointer
        if (!pSkinCluster)
        {
            std::cout << "Could not load skeleton data, no skin cluster specified\n";
            std::cout.flush();
            return MS::kFailure;
        }
        //retrieve and load joints from the skin cluster
        MDagPath jointDag,rootDag;
        MDagPathArray influenceDags;
        int numInfluenceObjs = pSkinCluster->influenceObjects(influenceDags,&stat);
        std::cout << "num influence objects: " << numInfluenceObjs << "\n";
        std::cout.flush();
        for (int i=0; i<numInfluenceObjs; i++)
        {
            jointDag = influenceDags[i];
            if (influenceDags[i].hasFn(MFn::kJoint))
            {
                //retrieve root joint
                rootDag = jointDag;
                while (jointDag.length()>0)
                {
                    jointDag.pop();
                    if (jointDag.hasFn(MFn::kJoint) && jointDag.length()>0)
                        rootDag = jointDag;
                }
                //check if skeleton has already been loaded
                bool skip = false;
                for (int j=0; j<m_joints.size() && !skip; j++)
                {
                    //skip skeleton if already loaded
                    if (rootDag.partialPathName() == m_joints[j].name)
                    {
                        skip = true;
                    }
                }
                //load joints data from root
                if (!skip)
                {
                    // load the skeleton
                    std::cout <<  "Loading skeleton with root: " << rootDag.fullPathName().asChar() << "...\n";
                    std::cout.flush();
                    // save current selection list
                    MSelectionList selectionList;
                    MGlobal::getActiveSelectionList(selectionList);
                    // select the root joint dag
                    MGlobal::selectByName(rootDag.fullPathName(),MGlobal::kReplaceList);
                    //save current pose (if no pose has been saved yet)
                    if (m_restorePose == "")
                    {
                        MString poseName;
                        MGlobal::executeCommand("dagPose -s",poseName,true);
                        m_restorePose = poseName;
                    }
                    //set the skeleton to the desired neutral pose
                    if (params.neutralPoseType == NPT_BINDPOSE)
                    {
                        //disable constraints, IK, etc...
                        MGlobal::executeCommand("doEnableNodeItems false all",true);
                        // Note: we reset to the bind pose
                        MGlobal::executeCommand("dagPose -r -g -bp",true);
                    }
                    //load joints data
                    stat = loadJoint(rootDag,NULL,params,pSkinCluster);
                    if (MS::kSuccess == stat)
                    {
                        std::cout << "OK\n";
                        std::cout.flush();
                    }
                    else
                    {
                        std::cout << "Failed\n";
                        std::cout.flush();
                    }
                    //restore selection list
                    MGlobal::setActiveSelectionList(selectionList,MGlobal::kReplaceList);
                }
            }
        }

        return MS::kSuccess;
    }


    // Load a joint
    MStatus Skeleton::loadJoint(MDagPath& jointDag,joint* parent,ParamList& params,MFnSkinCluster* pSkinCluster)
    {
        MStatus stat;
        joint newJoint;
        joint* parentJoint = parent;
        // if it is a joint node translate it and then proceed to child nodes, otherwise skip it
        // and proceed directly to child nodes
        if (jointDag.hasFn(MFn::kJoint))
        {
            MFnIkJoint jointFn(jointDag);
            // Display info
            std::cout << "Loading joint: " << jointFn.fullPathName().asChar();
            std::cout.flush();
            if (parent)
            {
                std::cout << " (parent: " << parent->name.asChar() << ")\n";
                std::cout.flush();
            }
            else
            {
                std::cout << "\n";
                std::cout.flush();
            }
            // Get parent index
            int idx=-1;
            if (parent)
            {
                for (int i=0; i<m_joints.size() && idx<0; i++)
                {
                    if (m_joints[i].name == parent->name)
                        idx=i;
                }
            }
            // Get world bind matrix
            MMatrix bindMatrix = jointDag.inclusiveMatrix();;
            // Calculate local bind matrix
            MMatrix localMatrix;
            if (parent)
                localMatrix = bindMatrix * parent->bindMatrix.inverse();
            else
            {   // root node of skeleton
                localMatrix = bindMatrix;
            }
            // Get translation
            MVector translation = ((MTransformationMatrix)localMatrix).translation(MSpace::kPostTransform);
            if (fabs(translation.x) < PRECISION)
                translation.x = 0;
            if (fabs(translation.y) < PRECISION)
                translation.y = 0;
            if (fabs(translation.z) < PRECISION)
                translation.z = 0;
            // Calculate rotation data
            double qx,qy,qz,qw;
            ((MTransformationMatrix)localMatrix).getRotationQuaternion(qx,qy,qz,qw);
            MQuaternion rotation(qx,qy,qz,qw);
            MVector axis;
            double theta;
            rotation.getAxisAngle(axis,theta);
            if (fabs(axis.x) < PRECISION)
                axis.x = 0;
            if (fabs(axis.y) < PRECISION)
                axis.y = 0;
            if (fabs(axis.z) < PRECISION)
                axis.z = 0;
            axis.normalize();
            if (fabs(theta) < PRECISION)
                theta = 0;
            if (axis.length() < 0.5)
            {
                axis.x = 0;
                axis.y = 1;
                axis.z = 0;
                theta = 0;
            }
            // Get joint scale
            double scale[3];
            ((MTransformationMatrix)localMatrix).getScale(scale,MSpace::kPostTransform);
            if (fabs(scale[0]) < PRECISION)
                scale[0] = 0;
            if (fabs(scale[1]) < PRECISION)
                scale[1] = 0;
            if (fabs(scale[2]) < PRECISION)
                scale[2] = 0;
            // Set joint info
            newJoint.name = jointFn.partialPathName();
            newJoint.id = m_joints.size();
            newJoint.parentIndex = idx;
            newJoint.bindMatrix = bindMatrix;
            newJoint.localMatrix = localMatrix;
            newJoint.posx = translation.x * params.lum;
            newJoint.posy = translation.y * params.lum;
            newJoint.posz = translation.z * params.lum;
            newJoint.angle = theta;
            newJoint.axisx = axis.x;
            newJoint.axisy = axis.y;
            newJoint.axisz = axis.z;
            newJoint.scalex = scale[0];
            newJoint.scaley = scale[1];
            newJoint.scalez = scale[2];
            newJoint.jointDag = jointDag;
            m_joints.push_back(newJoint);
            // If root is a root joint, save its index in the roots list
            if (idx < 0)
            {
                m_roots.push_back(m_joints.size() - 1);
            }
            // Get pointer to newly created joint
            parentJoint = &newJoint;
        }
        // Load child joints
        for (int i=0; i<jointDag.childCount();i++)
        {
            MObject child;
            child = jointDag.child(i);
            MDagPath childDag = jointDag;
            childDag.push(child);
            loadJoint(childDag,parentJoint,params,pSkinCluster);
        }
        return MS::kSuccess;
    }


    // Load animations
    MStatus Skeleton::loadAnims(ParamList& params)
    {
        MStatus stat;
        // save current time for later restore
        MTime curTime = MAnimControl::currentTime();
        std::cout << "Loading joint animations...\n";
        std::cout.flush();
        // clear animations list
        m_animations.clear();
        // load skeleton animation clips for the whole skeleton
        for (int i=0; i<params.skelClipList.size(); i++)
        {
            stat = loadClip(params.skelClipList[i].name,params.skelClipList[i].start,
                params.skelClipList[i].stop,params.skelClipList[i].rate,params);
            if (stat == MS::kSuccess)
            {
                std::cout << "Clip successfully loaded\n";
                std::cout.flush();
            }
            else
            {
                std::cout << "Failed loading clip\n";
                std::cout.flush();
            }
        }
        //restore current time
        MAnimControl::setCurrentTime(curTime);
        return MS::kSuccess;
    }

    // Load an animation clip
    MStatus Skeleton::loadClip(MString clipName,float start,float stop,float rate,ParamList& params)
    {
        MStatus stat;
        MString msg;
        std::vector<float> times;
        // if skeleton has no joints we can't load the clip
        if (m_joints.size() < 0)
            return MS::kFailure;
        // display clip name
        std::cout << "clip \"" << clipName.asChar() << "\"\n";
        std::cout.flush();
        // calculate times from clip sample rate
        times.clear();
        if (rate <= 0)
        {
            std::cout << "invalid sample rate for the clip (must be >0), we skip it\n";
            std::cout.flush();
            return MS::kFailure;
        }
        for (float t=start; t<stop; t+=rate)
            times.push_back(t);
        times.push_back(stop);
        // get animation length
        float length=0;
        if (times.size() >= 0)
            length = times[times.size()-1] - times[0];
        if (length < 0)
        {
            std::cout << "invalid time range for the clip, we skip it\n";
            std::cout.flush();
            return MS::kFailure;
        }
        // create the animation
        Animation a;
        a.m_name = clipName.asChar();
        a.m_tracks.clear();
        a.m_length = length;
        m_animations.push_back(a);
        int animIdx = m_animations.size() - 1;
        // create a track for current clip for all joints
        std::vector<Track> animTracks;
        for (int i=0; i<m_joints.size(); i++)
        {
            Track t;
            t.m_type = TT_SKELETON;
            t.m_bone = m_joints[i].name;
            t.m_skeletonKeyframes.clear();
            animTracks.push_back(t);
        }
        // evaluate animation curves at selected times
        for (int i=0; i<times.size(); i++)
        {
            //set time to wanted sample time
            MAnimControl::setCurrentTime(MTime(times[i],MTime::kSeconds));
            //load a keyframe for every joint at current time
            for (int j=0; j<m_joints.size(); j++)
            {
                skeletonKeyframe key = loadKeyframe(m_joints[j],times[i]-times[0],params);
                //add keyframe to joint track
                animTracks[j].addSkeletonKeyframe(key);
            }
            if (params.skelBB)
            {
                // Update bounding boxes of loaded submeshes
                for (int j=0; j<params.loadedSubmeshes.size(); j++)
                {
                    MFnMesh mesh(params.loadedSubmeshes[j]->m_dagPath);
                    MPoint min = mesh.boundingBox().min();
                    MPoint max = mesh.boundingBox().max();
                    MBoundingBox bbox(min,max);
                    if (params.exportWorldCoords)
                        bbox.transformUsing(params.loadedSubmeshes[j]->m_dagPath.inclusiveMatrix());
                    min = bbox.min() * params.lum;
                    max = bbox.max() * params.lum;
                    MBoundingBox newbbox(min,max);
                    params.loadedSubmeshes[j]->m_boundingBox.expand(newbbox);
                }
            }
        }
        // add created tracks to current clip
        for (int i=0; i<animTracks.size(); i++)
        {
            m_animations[animIdx].addTrack(animTracks[i]);
        }
        // display info
        std::cout << "length: " << m_animations[animIdx].m_length << "\n";
        std::cout << "num keyframes: " << animTracks[0].m_skeletonKeyframes.size() << "\n";
        std::cout.flush();
        // clip successfully loaded
        return MS::kSuccess;
    }

    // Load a keyframe for a given joint at current time
    skeletonKeyframe Skeleton::loadKeyframe(joint& j,float time,ParamList& params)
    {
        MVector position;
        int parentIdx = j.parentIndex;
        // Get joint matrix
        MMatrix worldMatrix = j.jointDag.inclusiveMatrix();
        // Calculate Local Matrix
        MMatrix localMatrix;
        if (parentIdx >= 0)
        {
            // Get parent joint
            MDagPath parentDag = m_joints[parentIdx].jointDag;
            localMatrix = worldMatrix * parentDag.inclusiveMatrixInverse();
        }
        else
        {   // Root node of skeleton
            if (params.exportWorldCoords)
                localMatrix = worldMatrix;
            else
                localMatrix = worldMatrix * j.jointDag.exclusiveMatrixInverse();
        }
        // Get relative transformation matrix
        MMatrix relMatrix = localMatrix * j.localMatrix.inverse();
        // Get relative translation
        MVector translation = ((MTransformationMatrix)localMatrix).translation(MSpace::kPostTransform) - 
            ((MTransformationMatrix)j.localMatrix).translation(MSpace::kPostTransform);
        if (fabs(translation.x) < PRECISION)
            translation.x = 0;
        if (fabs(translation.y) < PRECISION)
            translation.y = 0;
        if (fabs(translation.z) < PRECISION)
            translation.z = 0;
        // Get relative rotation
        double qx,qy,qz,qw;
        ((MTransformationMatrix)relMatrix).getRotationQuaternion(qx,qy,qz,qw);
        MQuaternion rotation(qx,qy,qz,qw);
        MVector axis;
        double theta;
        rotation.getAxisAngle(axis,theta);
        while (theta > Ogre::Math::PI) 
        { 
            theta -= Ogre::Math::TWO_PI; 
        } 
        while (theta < -Ogre::Math::PI) 
        { 
            theta += Ogre::Math::TWO_PI; 
        } 
        if (fabs(axis.x) < PRECISION)
            axis.x = 0;
        if (fabs(axis.y) < PRECISION)
            axis.y = 0;
        if (fabs(axis.z) < PRECISION)
            axis.z = 0;
        axis.normalize();
        if (fabs(theta) < PRECISION)
            theta = 0;
        if (axis.length() < 0.5)
        {
            axis.x = 0;
            axis.y = 1;
            axis.z = 0;
            theta = 0;
        }
        // Get relative scale
        double scale[3];
        ((MTransformationMatrix)relMatrix).getScale(scale,MSpace::kPostTransform);
        if (fabs(scale[0]) < PRECISION)
            scale[0] = 0;
        if (fabs(scale[1]) < PRECISION)
            scale[1] = 0;
        if (fabs(scale[2]) < PRECISION)
            scale[2] = 0;
        //create keyframe
        skeletonKeyframe key;
        key.time = time;
        key.tx = translation.x * params.lum;
        key.ty = translation.y * params.lum;
        key.tz = translation.z * params.lum;
        key.angle = theta;
        key.axis_x = axis.x;
        key.axis_y = axis.y;
        key.axis_z = axis.z;
        key.sx = scale[0];
        key.sy = scale[1];
        key.sz = scale[2];
        return key;
    }


    // Restore skeleton pose
    void Skeleton::restorePose()
    {
        //enable constraints, IK, etc...
        MGlobal::executeCommand("doEnableNodeItems true all",true);
        // save current selection list
        MSelectionList selectionList;
        MGlobal::getActiveSelectionList(selectionList);
        // restore the pose on all parts of the skeleton
        for (int i=0; i<m_roots.size(); i++)
        {
            MDagPath rootDag = m_joints[m_roots[i]].jointDag;
            // select the root joint dag
            MGlobal::selectByName(rootDag.fullPathName(),MGlobal::kReplaceList);
            // restore pose
            MString cmd = "dagPose -r -g -n \""+ m_restorePose;
            cmd += "\"";
            MGlobal::executeCommand(cmd,true);
        }
        //restore selection list
        MGlobal::setActiveSelectionList(selectionList,MGlobal::kReplaceList);
    }



    // Get joint list
    std::vector<joint>& Skeleton::getJoints()
    {
        return m_joints;
    }



    // Get animations
    std::vector<Animation>& Skeleton::getAnimations()
    {
        return m_animations;
    }



    // Write to an OGRE binary skeleton
    MStatus Skeleton::writeOgreBinary(ParamList &params)
    {
        MStatus stat;
        // Construct skeleton
        MString name = "exportSkeleton";
        Ogre::SkeletonPtr pSkeleton = Ogre::SkeletonManager::getSingleton().create(name.asChar(), 
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        // Create skeleton bones
        stat = createOgreBones(pSkeleton,params);
        if (stat != MS::kSuccess)
        {
            std::cout << "Error writing skeleton binary file\n";
            std::cout.flush();
        }
        // Create skeleton animation
        if (params.exportSkelAnims)
        {
            stat = createOgreSkeletonAnimations(pSkeleton,params);
            if (stat != MS::kSuccess)
            {
                std::cout << "Error writing ogre skeleton animations\n";
                std::cout.flush();
            }
        }
        pSkeleton->setBindingPose();
        // Optimise animations
        pSkeleton->optimiseAllAnimations();
        // Export skeleton binary
        Ogre::SkeletonSerializer serializer;
        serializer.exportSkeleton(pSkeleton.getPointer(),params.skeletonFilename.asChar());
        pSkeleton.setNull();
        // Skeleton successfully exported
        return MS::kSuccess;
    }

    // Write joints to an Ogre skeleton
    MStatus Skeleton::createOgreBones(Ogre::SkeletonPtr pSkeleton,ParamList& params)
    {
        // Create the bones
        for (int i=0; i<m_joints.size(); i++)
        {
            joint* j = &m_joints[i];
            // Create a new bone
            Ogre::String jointName = j->name.asChar();
            int jointId = j->id;
            Ogre::Bone* pBone = pSkeleton->createBone(jointName, jointId);
            // Set bone position (relative to it's parent)
            pBone->setPosition(j->posx,j->posy,j->posz);
            // Set bone orientation (relative to it's parent)
            Ogre::Quaternion orient;
            orient.FromAngleAxis(Ogre::Radian(j->angle),Ogre::Vector3(j->axisx,j->axisy,j->axisz));
            pBone->setOrientation(orient);
            // Set bone scale (relative to it's parent)
            pBone->setScale(j->scalex,j->scaley,j->scalez);
        }
        // Create the hierarchy
        for (int i=0; i<m_joints.size(); i++)
        {
            joint* j = &m_joints[i];
            int parentIdx = j->parentIndex;
            if (parentIdx >= 0)
            {
                // Get the parent joint
                joint* parentJoint = &m_joints[parentIdx];
                unsigned short parentId = m_joints[parentIdx].id;
                Ogre::String parentName = parentJoint->name.asChar();
                Ogre::Bone* pParent = pSkeleton->getBone(parentName);
                // Get current joint from skeleton
                unsigned short jointId = j->id;
                Ogre::String jointName = j->name.asChar();
                Ogre::Bone* pBone = pSkeleton->getBone(jointName);
                // Place current bone in the parent's child list
                pParent->addChild(pBone);
            }
        }
        return MS::kSuccess;
    }


    // Write skeleton animations to an Ogre skeleton
    MStatus Skeleton::createOgreSkeletonAnimations(Ogre::SkeletonPtr pSkeleton,ParamList& params)
    {
        // Read loaded skeleton animations
        for (int i=0; i<m_animations.size(); i++)
        {
            // Create a new animation
            Ogre::Animation* pAnimation = pSkeleton->createAnimation(m_animations[i].m_name.asChar(),
                m_animations[i].m_length);
            // Create tracks for current animation
            for (int j=0; j<m_animations[i].m_tracks.size(); j++)
            {
                Track* t = &m_animations[i].m_tracks[j];
                // Create a new track
                Ogre::NodeAnimationTrack* pTrack = pAnimation->createNodeTrack(j,
                    pSkeleton->getBone(t->m_bone.asChar()));
                // Create keyframes for current track
                for (int k=0; k<t->m_skeletonKeyframes.size(); k++)
                {
                    skeletonKeyframe* keyframe = &t->m_skeletonKeyframes[k];
                    // Create a new keyframe
                    Ogre::TransformKeyFrame* pKeyframe = pTrack->createNodeKeyFrame(keyframe->time);
                    // Set translation
                    pKeyframe->setTranslate(Ogre::Vector3(keyframe->tx,keyframe->ty,keyframe->tz));
                    // Set rotation
                    Ogre::Quaternion rot;
                    rot.FromAngleAxis(Ogre::Radian(keyframe->angle),
                        Ogre::Vector3(keyframe->axis_x,keyframe->axis_y,keyframe->axis_z));
                    pKeyframe->setRotation(rot);
                    // Set scale
                    pKeyframe->setScale(Ogre::Vector3(keyframe->sx,keyframe->sy,keyframe->sz));
                }
            }
        }
        return MS::kSuccess;
    }


};  //end namespace