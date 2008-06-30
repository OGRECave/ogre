/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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

