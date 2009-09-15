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
#include "OgreStableHeaders.h"

#include "OgreSkeletonFileFormat.h"
#include "OgreSkeletonSerializer.h"
#include "OgreSkeleton.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreBone.h"
#include "OgreString.h"
#include "OgreDataStream.h"
#include "OgreLogManager.h"




namespace Ogre {
    /// stream overhead = ID + size
    const long STREAM_OVERHEAD_SIZE = sizeof(uint16) + sizeof(uint32);
    //---------------------------------------------------------------------
    SkeletonSerializer::SkeletonSerializer()
    {
        // Version number
        // NB changed to include bone names in 1.1
        mVersion = "[Serializer_v1.10]";
    }
    //---------------------------------------------------------------------
    SkeletonSerializer::~SkeletonSerializer()
    {
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::exportSkeleton(const Skeleton* pSkeleton, 
		const String& filename, Endian endianMode)
    {
		// Decide on endian mode
		determineEndianness(endianMode);

        String msg;
        mpfFile = fopen(filename.c_str(), "wb");
		if (!mpfFile)
		{
			OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE,
				"Unable to open file " + filename + " for writing",
				"SkeletonSerializer::exportSkeleton");
		}

        writeFileHeader();

        // Write main skeleton data
        LogManager::getSingleton().logMessage("Exporting bones..");
        writeSkeleton(pSkeleton);
        LogManager::getSingleton().logMessage("Bones exported.");

        // Write all animations
        unsigned short numAnims = pSkeleton->getNumAnimations();
        LogManager::getSingleton().stream()
			<< "Exporting animations, count=" << numAnims;
        for (unsigned short i = 0; i < numAnims; ++i)
        {
            Animation* pAnim = pSkeleton->getAnimation(i);
			LogManager::getSingleton().stream()
				<< "Exporting animation: " << pAnim->getName();
            writeAnimation(pSkeleton, pAnim);
            LogManager::getSingleton().logMessage("Animation exported.");

        }

		// Write links
		Skeleton::LinkedSkeletonAnimSourceIterator linkIt = 
			pSkeleton->getLinkedSkeletonAnimationSourceIterator();
		while(linkIt.hasMoreElements())
		{
			const LinkedSkeletonAnimationSource& link = linkIt.getNext();
			writeSkeletonAnimationLink(pSkeleton, link);
		}

        fclose(mpfFile);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::importSkeleton(DataStreamPtr& stream, Skeleton* pSkel)
    {
		// Determine endianness (must be the first thing we do!)
		determineEndianness(stream);

		// Check header
        readFileHeader(stream);

        unsigned short streamID;
        while(!stream->eof())
        {
            streamID = readChunk(stream);
            switch (streamID)
            {
            case SKELETON_BONE:
                readBone(stream, pSkel);
                break;
            case SKELETON_BONE_PARENT:
                readBoneParent(stream, pSkel);
                break;
            case SKELETON_ANIMATION:
                readAnimation(stream, pSkel);
				break;
			case SKELETON_ANIMATION_LINK:
				readSkeletonAnimationLink(stream, pSkel);
				break;
            }
        }

        // Assume bones are stored in the binding pose
        pSkel->setBindingPose();


    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeSkeleton(const Skeleton* pSkel)
    {
        // Write each bone
        unsigned short numBones = pSkel->getNumBones();
        unsigned short i;
        for (i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            writeBone(pSkel, pBone);
        }
        // Write parents
        for (i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            unsigned short handle = pBone->getHandle();
            Bone* pParent = (Bone*)pBone->getParent(); 
            if (pParent != NULL) 
            {
                writeBoneParent(pSkel, handle, pParent->getHandle());             
            }
        }
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeBone(const Skeleton* pSkel, const Bone* pBone)
    {
        writeChunkHeader(SKELETON_BONE, calcBoneSize(pSkel, pBone));

        unsigned short handle = pBone->getHandle();
        // char* name
        writeString(pBone->getName());
        // unsigned short handle            : handle of the bone, should be contiguous & start at 0
        writeShorts(&handle, 1);
        // Vector3 position                 : position of this bone relative to parent 
        writeObject(pBone->getPosition());
        // Quaternion orientation           : orientation of this bone relative to parent 
        writeObject(pBone->getOrientation());
        // Vector3 scale                    : scale of this bone relative to parent 
        if (pBone->getScale() != Vector3::UNIT_SCALE)
        {
            writeObject(pBone->getScale());
        }
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeBoneParent(const Skeleton* pSkel, 
        unsigned short boneId, unsigned short parentId)
    {
        writeChunkHeader(SKELETON_BONE_PARENT, calcBoneParentSize(pSkel));

        // unsigned short handle             : child bone
        writeShorts(&boneId, 1);
        // unsigned short parentHandle   : parent bone
        writeShorts(&parentId, 1);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeAnimation(const Skeleton* pSkel, 
        const Animation* anim)
    {
        writeChunkHeader(SKELETON_ANIMATION, calcAnimationSize(pSkel, anim));

        // char* name                       : Name of the animation
        writeString(anim->getName());
        // float length                      : Length of the animation in seconds
        float len = anim->getLength();
        writeFloats(&len, 1);

        // Write all tracks
        Animation::NodeTrackIterator trackIt = anim->getNodeTrackIterator();
        while(trackIt.hasMoreElements())
        {
            writeAnimationTrack(pSkel, trackIt.getNext());
        }

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeAnimationTrack(const Skeleton* pSkel, 
        const NodeAnimationTrack* track)
    {
        writeChunkHeader(SKELETON_ANIMATION_TRACK, calcAnimationTrackSize(pSkel, track));

        // unsigned short boneIndex     : Index of bone to apply to
        Bone* bone = (Bone*)track->getAssociatedNode();
        unsigned short boneid = bone->getHandle();
        writeShorts(&boneid, 1);

        // Write all keyframes
        for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
        {
            writeKeyFrame(pSkel, track->getNodeKeyFrame(i));
        }

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeKeyFrame(const Skeleton* pSkel, 
        const TransformKeyFrame* key)
    {

        writeChunkHeader(SKELETON_ANIMATION_TRACK_KEYFRAME, 
            calcKeyFrameSize(pSkel, key));

        // float time                    : The time position (seconds)
        float time = key->getTime();
        writeFloats(&time, 1);
        // Quaternion rotate            : Rotation to apply at this keyframe
        writeObject(key->getRotation());
        // Vector3 translate            : Translation to apply at this keyframe
        writeObject(key->getTranslate());
        // Vector3 scale                : Scale to apply at this keyframe
        if (key->getScale() != Vector3::UNIT_SCALE)
        {
            writeObject(key->getScale());
        }
    }
    //---------------------------------------------------------------------
    size_t SkeletonSerializer::calcBoneSize(const Skeleton* pSkel, 
        const Bone* pBone)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // handle
        size += sizeof(unsigned short);

        // position
        size += sizeof(float) * 3;

        // orientation
        size += sizeof(float) * 4;

        // scale
        if (pBone->getScale() != Vector3::UNIT_SCALE)
        {
            size += sizeof(float) * 3;
        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t SkeletonSerializer::calcBoneSizeWithoutScale(const Skeleton* pSkel, 
        const Bone* pBone)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // handle
        size += sizeof(unsigned short);

        // position
        size += sizeof(float) * 3;

        // orientation
        size += sizeof(float) * 4;

        return size;
    }
    //---------------------------------------------------------------------
    size_t SkeletonSerializer::calcBoneParentSize(const Skeleton* pSkel)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // handle
        size += sizeof(unsigned short);

        // parent handle
        size += sizeof(unsigned short);

        return size;
    }
    //---------------------------------------------------------------------
    size_t SkeletonSerializer::calcAnimationSize(const Skeleton* pSkel, 
        const Animation* pAnim)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // Name, including terminator
        size += pAnim->getName().length() + 1;
        // length
        size += sizeof(float);

        // Nested animation tracks
		Animation::NodeTrackIterator trackIt = pAnim->getNodeTrackIterator();
		while(trackIt.hasMoreElements())
		{
            size += calcAnimationTrackSize(pSkel, trackIt.getNext());
        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t SkeletonSerializer::calcAnimationTrackSize(const Skeleton* pSkel, 
        const NodeAnimationTrack* pTrack)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // unsigned short boneIndex     : Index of bone to apply to
        size += sizeof(unsigned short);

        // Nested keyframes
        for (unsigned short i = 0; i < pTrack->getNumKeyFrames(); ++i)
        {
            size += calcKeyFrameSize(pSkel, pTrack->getNodeKeyFrame(i));
        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t SkeletonSerializer::calcKeyFrameSize(const Skeleton* pSkel, 
        const TransformKeyFrame* pKey)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // float time                    : The time position (seconds)
        size += sizeof(float);
        // Quaternion rotate            : Rotation to apply at this keyframe
        size += sizeof(float) * 4;
        // Vector3 translate            : Translation to apply at this keyframe
        size += sizeof(float) * 3;
        // Vector3 scale                : Scale to apply at this keyframe
        if (pKey->getScale() != Vector3::UNIT_SCALE)
        {
            size += sizeof(float) * 3;
        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t SkeletonSerializer::calcKeyFrameSizeWithoutScale(const Skeleton* pSkel, 
        const TransformKeyFrame* pKey)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // float time                    : The time position (seconds)
        size += sizeof(float);
        // Quaternion rotate            : Rotation to apply at this keyframe
        size += sizeof(float) * 4;
        // Vector3 translate            : Translation to apply at this keyframe
        size += sizeof(float) * 3;

        return size;
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readBone(DataStreamPtr& stream, Skeleton* pSkel)
    {
        // char* name
        String name = readString(stream);
        // unsigned short handle            : handle of the bone, should be contiguous & start at 0
        unsigned short handle;
        readShorts(stream, &handle, 1);

        // Create new bone
        Bone* pBone = pSkel->createBone(name, handle);

        // Vector3 position                 : position of this bone relative to parent 
        Vector3 pos;
        readObject(stream, pos);
        pBone->setPosition(pos);
        // Quaternion orientation           : orientation of this bone relative to parent 
        Quaternion q;
        readObject(stream, q);
        pBone->setOrientation(q);
        // Do we have scale?
        if (mCurrentstreamLen > calcBoneSizeWithoutScale(pSkel, pBone))
        {
            Vector3 scale;
            readObject(stream, scale);
            pBone->setScale(scale);
        }
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readBoneParent(DataStreamPtr& stream, Skeleton* pSkel)
    {
        // All bones have been created by this point
        Bone *child, *parent;
        unsigned short childHandle, parentHandle;

        // unsigned short handle             : child bone
        readShorts(stream, &childHandle, 1);
        // unsigned short parentHandle   : parent bone
        readShorts(stream, &parentHandle, 1);

        // Find bones
        parent = pSkel->getBone(parentHandle);
        child = pSkel->getBone(childHandle);

        // attach
        parent->addChild(child);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readAnimation(DataStreamPtr& stream, Skeleton* pSkel)
    {
        // char* name                       : Name of the animation
        String name;
        name = readString(stream);
        // float length                      : Length of the animation in seconds
        float len;
        readFloats(stream, &len, 1);

        Animation *pAnim = pSkel->createAnimation(name, len);

        // Read all tracks
        if (!stream->eof())
        {
            unsigned short streamID = readChunk(stream);
            while(streamID == SKELETON_ANIMATION_TRACK && !stream->eof())
            {
                readAnimationTrack(stream, pAnim, pSkel);

                if (!stream->eof())
                {
                    // Get next stream
                    streamID = readChunk(stream);
                }
            }
            if (!stream->eof())
            {
                // Backpedal back to start of this stream if we've found a non-track
                stream->skip(-STREAM_OVERHEAD_SIZE);
            }

        }



    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readAnimationTrack(DataStreamPtr& stream, Animation* anim, 
        Skeleton* pSkel)
    {
        // unsigned short boneIndex     : Index of bone to apply to
        unsigned short boneHandle;
        readShorts(stream, &boneHandle, 1);

        // Find bone
        Bone *targetBone = pSkel->getBone(boneHandle);

        // Create track
        NodeAnimationTrack* pTrack = anim->createNodeTrack(boneHandle, targetBone);

        // Keep looking for nested keyframes
        if (!stream->eof())
        {
            unsigned short streamID = readChunk(stream);
            while(streamID == SKELETON_ANIMATION_TRACK_KEYFRAME && !stream->eof())
            {
                readKeyFrame(stream, pTrack, pSkel);

                if (!stream->eof())
                {
                    // Get next stream
                    streamID = readChunk(stream);
                }
            }
            if (!stream->eof())
            {
                // Backpedal back to start of this stream if we've found a non-keyframe
                stream->skip(-STREAM_OVERHEAD_SIZE);
            }

        }


    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readKeyFrame(DataStreamPtr& stream, NodeAnimationTrack* track, 
        Skeleton* pSkel)
    {
        // float time                    : The time position (seconds)
        float time;
        readFloats(stream, &time, 1);

        TransformKeyFrame *kf = track->createNodeKeyFrame(time);

        // Quaternion rotate            : Rotation to apply at this keyframe
        Quaternion rot;
        readObject(stream, rot);
        kf->setRotation(rot);
        // Vector3 translate            : Translation to apply at this keyframe
        Vector3 trans;
        readObject(stream, trans);
        kf->setTranslate(trans);
        // Do we have scale?
        if (mCurrentstreamLen > calcKeyFrameSizeWithoutScale(pSkel, kf))
        {
            Vector3 scale;
            readObject(stream, scale);
            kf->setScale(scale);
        }
    }
	//---------------------------------------------------------------------
	void SkeletonSerializer::writeSkeletonAnimationLink(const Skeleton* pSkel, 
		const LinkedSkeletonAnimationSource& link)
	{
		writeChunkHeader(SKELETON_ANIMATION_LINK, 
			calcSkeletonAnimationLinkSize(pSkel, link));

		// char* skeletonName
		writeString(link.skeletonName);
		// float scale
		writeFloats(&(link.scale), 1);

	}
    //---------------------------------------------------------------------
	size_t SkeletonSerializer::calcSkeletonAnimationLinkSize(const Skeleton* pSkel, 
		const LinkedSkeletonAnimationSource& link)
	{
		size_t size = STREAM_OVERHEAD_SIZE;

		// char* skeletonName
		size += link.skeletonName.length() + 1;
		// float scale
		size += sizeof(float);

		return size;

	}
	//---------------------------------------------------------------------
	void SkeletonSerializer::readSkeletonAnimationLink(DataStreamPtr& stream, 
		Skeleton* pSkel)
	{
		// char* skeletonName
		String skelName = readString(stream);
		// float scale
		float scale;
		readFloats(stream, &scale, 1);

		pSkel->addLinkedSkeletonAnimationSource(skelName, scale);

	}



}


