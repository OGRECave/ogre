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
		if (!mNormalsMap.empty())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Inconsistent calls to addVertex, must include normals always or never",
				"Pose::addVertex");
		mVertexOffsetMap[index] = offset;
		mBuffer.setNull();
	}
	//---------------------------------------------------------------------
	void Pose::addVertex(size_t index, const Vector3& offset, const Vector3& normal)
	{
		if (!mVertexOffsetMap.empty() && mNormalsMap.empty())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Inconsistent calls to addVertex, must include normals always or never",
				"Pose::addVertex");
		mVertexOffsetMap[index] = offset;
		mNormalsMap[index] = normal;
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
		NormalsMap::iterator j = mNormalsMap.find(index);
		if (j != mNormalsMap.end())
		{
			mNormalsMap.erase(j);
		}
	}
	//---------------------------------------------------------------------
	void Pose::clearVertices(void)
	{
		mVertexOffsetMap.clear();
		mNormalsMap.clear();
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
	Pose::ConstNormalsIterator Pose::getNormalsIterator(void) const
	{
		return ConstNormalsIterator(mNormalsMap.begin(), mNormalsMap.end());
	}
	//---------------------------------------------------------------------
	Pose::NormalsIterator Pose::getNormalsIterator(void)
	{
		return NormalsIterator(mNormalsMap.begin(), mNormalsMap.end());
	}
	//---------------------------------------------------------------------
	const HardwareVertexBufferSharedPtr& Pose::_getHardwareVertexBuffer(size_t numVertices) const
	{
		if (mBuffer.isNull())
		{
			// Create buffer
			size_t vertexSize = VertexElement::getTypeSize(VET_FLOAT3);
			bool normals = getIncludesNormals();
			if (normals)
				vertexSize += VertexElement::getTypeSize(VET_FLOAT3);
				
			mBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
				vertexSize,	numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

			float* pFloat = static_cast<float*>(
				mBuffer->lock(HardwareBuffer::HBL_DISCARD));
			// initialise
			memset(pFloat, 0, mBuffer->getSizeInBytes()); 
			// Set each vertex
			VertexOffsetMap::const_iterator v = mVertexOffsetMap.begin();
			NormalsMap::const_iterator n = mNormalsMap.begin();
			while(v != mVertexOffsetMap.end())
			{
				// Remember, vertex maps are *sparse* so may have missing entries
				// This is why we skip
				float* pDst = pFloat + (3 * v->first);
				*pDst++ = v->second.x;
				*pDst++ = v->second.y;
				*pDst++ = v->second.z;
				++v;
				if (normals)
				{
					*pDst++ = n->second.x;
					*pDst++ = n->second.y;
					*pDst++ = n->second.z;
					++n;
				}
				
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

