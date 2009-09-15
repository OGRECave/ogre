/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team

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
#include "OgrePose.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre {
	//---------------------------------------------------------------------
	Pose::Pose(ushort target, const String& name)
		: mTarget(target), mName(name)
	{
	}
	//---------------------------------------------------------------------
	Pose::~Pose()
	{
	}
	//---------------------------------------------------------------------
	void Pose::addVertex(size_t index, const Vector3& offset)
	{
		mVertexOffsetMap[index] = offset;
		mBuffer.setNull();
	}
	//---------------------------------------------------------------------
	void Pose::removeVertex(size_t index)
	{
		VertexOffsetMap::iterator i = mVertexOffsetMap.find(index);
		if (i != mVertexOffsetMap.end())
		{
			mVertexOffsetMap.erase(i);
			mBuffer.setNull();
		}
	}
	//---------------------------------------------------------------------
	void Pose::clearVertexOffsets(void)
	{
		mVertexOffsetMap.clear();
		mBuffer.setNull();
	}
	//---------------------------------------------------------------------
	Pose::ConstVertexOffsetIterator 
		Pose::getVertexOffsetIterator(void) const
	{
		return ConstVertexOffsetIterator(mVertexOffsetMap.begin(), mVertexOffsetMap.end());
	}
	//---------------------------------------------------------------------
	Pose::VertexOffsetIterator 
		Pose::getVertexOffsetIterator(void)
	{
		return VertexOffsetIterator(mVertexOffsetMap.begin(), mVertexOffsetMap.end());
	}
	//---------------------------------------------------------------------
	const HardwareVertexBufferSharedPtr& Pose::_getHardwareVertexBuffer(size_t numVertices) const
	{
		if (mBuffer.isNull())
		{
			// Create buffer
			mBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
				VertexElement::getTypeSize(VET_FLOAT3),
				numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

			float* pFloat = static_cast<float*>(
				mBuffer->lock(HardwareBuffer::HBL_DISCARD));
			// initialise
			memset(pFloat, 0, mBuffer->getSizeInBytes()); 
			// Set each vertex
			for (VertexOffsetMap::const_iterator i = mVertexOffsetMap.begin();
				i != mVertexOffsetMap.end(); ++i)
			{
				float* pDst = pFloat + (3 * i->first);
				*pDst++ = i->second.x;
				*pDst++ = i->second.y;
				*pDst++ = i->second.z;
			}
			mBuffer->unlock();
		}
		return mBuffer;
	}
	//---------------------------------------------------------------------
	Pose* Pose::clone(void) const
	{
		Pose* newPose = OGRE_NEW Pose(mTarget, mName);
		newPose->mVertexOffsetMap = mVertexOffsetMap;
		// Allow buffer to recreate itself, contents may change anyway
		return newPose;
	}

}

