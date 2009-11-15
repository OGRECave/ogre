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
#include "OgreTangentSpaceCalc.h"
#include "OgreHardwareBufferManager.h"
#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	TangentSpaceCalc::TangentSpaceCalc()
		: mVData(0)
		, mSplitMirrored(false)
		, mSplitRotated(false)
		, mStoreParityInW(false)
	{
	}
	//---------------------------------------------------------------------
	TangentSpaceCalc::~TangentSpaceCalc()
	{

	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::clear()
	{
		mIDataList.clear();
		mOpTypes.clear();
		mVData = 0;
	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::setVertexData(VertexData* v_in)
	{
		mVData = v_in;
	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::addIndexData(IndexData* i_in, RenderOperation::OperationType op)
	{
		if (op != RenderOperation::OT_TRIANGLE_FAN && 
			op != RenderOperation::OT_TRIANGLE_LIST && 
			op != RenderOperation::OT_TRIANGLE_STRIP)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Only indexed triangle (list, strip, fan) render operations are supported.",
				"TangentSpaceCalc::addIndexData");

		}
		mIDataList.push_back(i_in);
		mOpTypes.push_back(op);
	}
	//---------------------------------------------------------------------
	TangentSpaceCalc::Result TangentSpaceCalc::build(
		VertexElementSemantic targetSemantic,
		unsigned short sourceTexCoordSet, unsigned short index)
	{
		Result res;

		// Pull out all the vertex components we'll need
		populateVertexArray(sourceTexCoordSet);

		// Now process the faces and calculate / add their contributions
		processFaces(res);

		// Now normalise & orthogonalise
		normaliseVertices();

		// Create new final geometry
		// First extend existing buffers to cope with new vertices
		extendBuffers(res.vertexSplits);

		// Alter indexes
		remapIndexes(res);

		// Create / identify target & write tangents
		insertTangents(res, targetSemantic, sourceTexCoordSet, index);

		return res;


	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::extendBuffers(VertexSplits& vertexSplits)
	{
		if (!vertexSplits.empty())
		{
			// ok, need to increase the vertex buffer size, and alter some indexes

			// vertex buffers first
			VertexBufferBinding* newBindings = HardwareBufferManager::getSingleton().createVertexBufferBinding();
			const VertexBufferBinding::VertexBufferBindingMap& bindmap = 
				mVData->vertexBufferBinding->getBindings();
			for (VertexBufferBinding::VertexBufferBindingMap::const_iterator i = 
				bindmap.begin(); i != bindmap.end(); ++i)
			{
				HardwareVertexBufferSharedPtr srcbuf = i->second;
				// Derive vertex count from buffer not vertex data, in case using
				// the vertexStart option in vertex data
				size_t newVertexCount = srcbuf->getNumVertices() + vertexSplits.size();
				// Create new buffer & bind
				HardwareVertexBufferSharedPtr newBuf = 
					HardwareBufferManager::getSingleton().createVertexBuffer(
					srcbuf->getVertexSize(), newVertexCount, srcbuf->getUsage(), 
					srcbuf->hasShadowBuffer());
				newBindings->setBinding(i->first, newBuf);

				// Copy existing contents (again, entire buffer, not just elements referenced)
				newBuf->copyData(*(srcbuf.get()), 0, 0, srcbuf->getNumVertices() * srcbuf->getVertexSize(), true);

				// Split vertices, read / write from new buffer
				char* pBase = static_cast<char*>(newBuf->lock(HardwareBuffer::HBL_NORMAL));
				for (VertexSplits::iterator spliti = vertexSplits.begin(); 
					spliti != vertexSplits.end(); ++spliti)
				{
					const char* pSrcBase = pBase + spliti->first * newBuf->getVertexSize();
					char* pDstBase = pBase + spliti->second * newBuf->getVertexSize();
					memcpy(pDstBase, pSrcBase, newBuf->getVertexSize());
				}
				newBuf->unlock();

			}

			// Update vertex data
			// Increase vertex count according to num splits
			mVData->vertexCount += vertexSplits.size();
			// Flip bindings over to new buffers (old buffers released)
			HardwareBufferManager::getSingleton().destroyVertexBufferBinding(mVData->vertexBufferBinding);
			mVData->vertexBufferBinding = newBindings;
			
		}

	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::remapIndexes(Result& res)
	{
		for (size_t i = 0; i < mIDataList.size(); ++i)
		{

			IndexData* idata = mIDataList[i];
			// Now do index data
			// no new buffer required, same size but some triangles remapped
			uint16* p16 = 0;
			uint32* p32 = 0;
			if (idata->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
			{
				p32 = static_cast<uint32*>(idata->indexBuffer->lock(HardwareBuffer::HBL_NORMAL));
				remapIndexes(p32, i, res);
			}
			else
			{
				p16 = static_cast<uint16*>(idata->indexBuffer->lock(HardwareBuffer::HBL_NORMAL));
				remapIndexes(p16, i, res);
			}
			idata->indexBuffer->unlock();
		}

	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::normaliseVertices()
	{
		// Just run through our complete (possibly augmented) list of vertices
		// Normalise the tangents & binormals
		for (VertexInfoArray::iterator i = mVertexArray.begin(); i != mVertexArray.end(); ++i)
		{
			VertexInfo& v = *i;

			v.tangent.normalise();
			v.binormal.normalise();

			// Orthogonalise with the vertex normal since it's currently
			// orthogonal with the face normals, but will be close to ortho
			// Apply Gram-Schmidt orthogonalise
			Vector3 temp = v.tangent;
			v.tangent = temp - (v.norm * v.norm.dotProduct(temp));

			temp = v.binormal;
			v.binormal = temp - (v.norm * v.norm.dotProduct(temp));

			// renormalize 
			v.tangent.normalise();
			v.binormal.normalise();

		}
	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::processFaces(Result& result)
	{
		// Quick pre-check for triangle strips / fans
		for (OpTypeList::iterator ot = mOpTypes.begin(); ot != mOpTypes.end(); ++ot)
		{
			if (*ot != RenderOperation::OT_TRIANGLE_LIST)
			{
				// Can't split strips / fans
				setSplitMirrored(false);
				setSplitRotated(false);
			}
		}

		for (size_t i = 0; i < mIDataList.size(); ++i)
		{
			IndexData* i_in = mIDataList[i];
			RenderOperation::OperationType opType = mOpTypes[i];

			// Read data from buffers
			uint16 *p16 = 0;
			uint32 *p32 = 0;

			HardwareIndexBufferSharedPtr ibuf = i_in->indexBuffer;
			if (ibuf->getType() == HardwareIndexBuffer::IT_32BIT)
			{
				p32 = static_cast<uint32*>(
					ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
				// offset by index start
				p32 += i_in->indexStart;
			}
			else
			{
				p16 = static_cast<uint16*>(
					ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
				// offset by index start
				p16 += i_in->indexStart;
			}
			// current triangle
			size_t vertInd[3] = { 0, 0, 0 };
			// loop through all faces to calculate the tangents and normals
			size_t faceCount = opType == RenderOperation::OT_TRIANGLE_LIST ? 
				i_in->indexCount / 3 : i_in->indexCount - 2;
			for (size_t f = 0; f < faceCount; ++f)
			{
				bool invertOrdering = false;
				// Read 1 or 3 indexes depending on type
				if (f == 0 || opType == RenderOperation::OT_TRIANGLE_LIST)
				{
					vertInd[0] = p32? *p32++ : *p16++;
					vertInd[1] = p32? *p32++ : *p16++;
					vertInd[2] = p32? *p32++ : *p16++;
				}
				else if (opType == RenderOperation::OT_TRIANGLE_FAN)
				{
					// Element 0 always remains the same
					// Element 2 becomes element 1
					vertInd[1] = vertInd[2];
					// read new into element 2
					vertInd[2] = p32? *p32++ : *p16++;
				}
				else if (opType == RenderOperation::OT_TRIANGLE_STRIP)
				{
					// Shunt everything down one, but also invert the ordering on 
					// odd numbered triangles (== even numbered i's)
					// we interpret front as anticlockwise all the time but strips alternate
					if (f & 0x1)
					{
						// odd tris (index starts at 3, 5, 7)
						invertOrdering = true;
					}
					vertInd[0] = vertInd[1];
					vertInd[1] = vertInd[2];			
					vertInd[2] = p32? *p32++ : *p16++;
				}

				// deal with strip inversion of winding
				size_t localVertInd[3];
				localVertInd[0] = vertInd[0];
				if (invertOrdering)
				{
					localVertInd[1] = vertInd[2];
					localVertInd[2] = vertInd[1];
				}
				else
				{
					localVertInd[1] = vertInd[1];
					localVertInd[2] = vertInd[2];
				}


				// For each triangle
				//   Calculate tangent & binormal per triangle
				//   Note these are not normalised, are weighted by UV area
				Vector3 faceTsU, faceTsV, faceNorm;
				calculateFaceTangentSpace(localVertInd, faceTsU, faceTsV, faceNorm);

				// Skip invalid UV space triangles
				if (faceTsU.isZeroLength() || faceTsV.isZeroLength())
					continue;

				addFaceTangentSpaceToVertices(i, f, localVertInd, faceTsU, faceTsV, faceNorm, result);

			}


			ibuf->unlock();
		}

	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::addFaceTangentSpaceToVertices(
		size_t indexSet, size_t faceIndex, size_t *localVertInd, 
		const Vector3& faceTsU, const Vector3& faceTsV, const Vector3& faceNorm, 
		Result& result)
	{
		// Calculate parity for this triangle
		int faceParity = calculateParity(faceTsU, faceTsV, faceNorm);
		// Now add these to each vertex referenced by the face
		for (int v = 0; v < 3; ++v)
		{
			// index 0 is vertex we're calculating, 1 and 2 are the others

			// We want to re-weight these by the angle the face makes with the vertex
			// in order to obtain tesselation-independent results
			Real angleWeight = calculateAngleWeight(localVertInd[v], 
				localVertInd[(v+1)%3], localVertInd[(v+2)%3]);


			VertexInfo* vertex = &(mVertexArray[localVertInd[v]]);

			// check parity (0 means not set)
			// Locate parity-version of vertex index, or create if doesn't exist
			// If parity-version of vertex index was different, record alteration
			// in triangle remap
			// in vertex split list
			bool splitVertex = false;
			size_t reusedOppositeParity = 0;
			bool splitBecauseOfParity = false;
			bool newVertex = false;
			if (!vertex->parity)
			{
				// init
				vertex->parity = faceParity;
				newVertex = true;
			}
			if (mSplitMirrored)
			{
				if (!newVertex && faceParity != calculateParity(vertex->tangent, vertex->binormal, vertex->norm))//vertex->parity != faceParity)
				{
					// Check for existing alternative parity
					if (vertex->oppositeParityIndex)
					{
						// Ok, have already split this vertex because of parity
						// Use the same one again
						reusedOppositeParity = vertex->oppositeParityIndex;
						vertex = &(mVertexArray[reusedOppositeParity]);
					}
					else
					{
						splitVertex = true;
						splitBecauseOfParity = true;

						LogManager::getSingleton().stream(LML_TRIVIAL)
							<< "TSC parity split - Vpar: " << vertex->parity 
							<< " Fpar: " << faceParity
							<< " faceTsU: " << faceTsU
							<< " faceTsV: " << faceTsV
							<< " faceNorm: " << faceNorm
							<< " vertTsU:" << vertex->tangent
							<< " vertTsV:" << vertex->binormal
							<< " vertNorm:" << vertex->norm;

					}
				}
			}

			if (mSplitRotated)
			{

				// deal with excessive tangent space rotations as well as mirroring
				// same kind of split behaviour appropriate
				if (!newVertex && !splitVertex)
				{
					// If more than 90 degrees, split
					Vector3 uvCurrent = vertex->tangent + vertex->binormal;

					// project down to the plane (plane normal = face normal)
					Vector3 vRotHalf = uvCurrent - faceNorm;
					vRotHalf *= faceNorm.dotProduct(uvCurrent);

					if ((faceTsU + faceTsV).dotProduct(vRotHalf) < 0.0f)
					{
						splitVertex = true;
					}
				}
			}

			if (splitVertex)
			{
				size_t newVertexIndex = mVertexArray.size();
				VertexSplit splitInfo(localVertInd[v], newVertexIndex);
				result.vertexSplits.push_back(splitInfo);
				// re-point opposite parity
				if (splitBecauseOfParity)
				{
					vertex->oppositeParityIndex = newVertexIndex;
				}
				// copy old values but reset tangent space
				VertexInfo newVertex = *vertex;
				newVertex.tangent = Vector3::ZERO;
				newVertex.binormal = Vector3::ZERO;
				newVertex.parity = faceParity;
				mVertexArray.push_back(newVertex);
				result.indexesRemapped.push_back(IndexRemap(indexSet, faceIndex, splitInfo));

				vertex = &(mVertexArray[newVertexIndex]);

			}
			else if (reusedOppositeParity)
			{
				// didn't split again, but we do need to record the re-used remapping
				VertexSplit splitInfo(localVertInd[v], reusedOppositeParity);
				result.indexesRemapped.push_back(IndexRemap(indexSet, faceIndex, splitInfo));

			}

			// Add weighted tangent & binormal
			vertex->tangent += (faceTsU * angleWeight);
			vertex->binormal += (faceTsV * angleWeight);


		}

	}
	//---------------------------------------------------------------------
	int TangentSpaceCalc::calculateParity(const Vector3& u, const Vector3& v, const Vector3& n)
	{
		// Note that this parity is the reverse of what you'd expect - this is
		// because the 'V' texture coordinate is actually left handed
		if (u.crossProduct(v).dotProduct(n) >= 0.0f)
			return -1;
		else
			return 1;

	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::calculateFaceTangentSpace(const size_t* vertInd, 
		Vector3& tsU, Vector3& tsV, Vector3& tsN)
	{
		const VertexInfo& v0 = mVertexArray[vertInd[0]];
		const VertexInfo& v1 = mVertexArray[vertInd[1]];
		const VertexInfo& v2 = mVertexArray[vertInd[2]];
		Vector2 deltaUV1 = v1.uv - v0.uv;
		Vector2 deltaUV2 = v2.uv - v0.uv;
		Vector3 deltaPos1 = v1.pos - v0.pos;
		Vector3 deltaPos2 = v2.pos - v0.pos;

		// face normal
		tsN = deltaPos1.crossProduct(deltaPos2);
		tsN.normalise();


		Real uvarea = deltaUV1.crossProduct(deltaUV2) * 0.5f;
		if (Math::RealEqual(uvarea, 0.0f))
		{
			// no tangent, null uv area
			tsU = tsV = Vector3::ZERO;
		}
		else
		{

			// Normalise by uvarea
			Real a = deltaUV2.y / uvarea;
			Real b = -deltaUV1.y / uvarea;
			Real c = -deltaUV2.x / uvarea;
			Real d = deltaUV1.x / uvarea;

			tsU = (deltaPos1 * a) + (deltaPos2 * b);
			tsU.normalise();

			tsV = (deltaPos1 * c) + (deltaPos2 * d);
			tsV.normalise();

			Real abs_uvarea = Math::Abs(uvarea);
			tsU *= abs_uvarea;
			tsV *= abs_uvarea;

			// tangent (tsU) and binormal (tsV) are now weighted by uv area


		}

	}
	//---------------------------------------------------------------------
	Real TangentSpaceCalc::calculateAngleWeight(size_t vidx0, size_t vidx1, size_t vidx2)
	{
		const VertexInfo& v0 = mVertexArray[vidx0];
		const VertexInfo& v1 = mVertexArray[vidx1];
		const VertexInfo& v2 = mVertexArray[vidx2];

		Vector3 diff0 = v1.pos - v0.pos;
		Vector3 diff1 = v2.pos - v1.pos;

		// Weight is just the angle - larger == better
		return diff0.angleBetween(diff1).valueRadians();

	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::populateVertexArray(unsigned short sourceTexCoordSet)
	{
		// Just pull data out into more friendly structures
		VertexDeclaration *dcl = mVData->vertexDeclaration;
		VertexBufferBinding *bind = mVData->vertexBufferBinding;

		// Get the incoming UV element
		const VertexElement* uvElem = dcl->findElementBySemantic(
			VES_TEXTURE_COORDINATES, sourceTexCoordSet);

		if (!uvElem || uvElem->getType() != VET_FLOAT2)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"No 2D texture coordinates with selected index, cannot calculate tangents.",
				"TangentSpaceCalc::build");
		}

		HardwareVertexBufferSharedPtr uvBuf, posBuf, normBuf;
		unsigned char *pUvBase, *pPosBase, *pNormBase;
		size_t uvInc, posInc, normInc;

		uvBuf = bind->getBuffer(uvElem->getSource());
		pUvBase = static_cast<unsigned char*>(
			uvBuf->lock(HardwareBuffer::HBL_READ_ONLY));
		uvInc = uvBuf->getVertexSize();
		// offset for vertex start
		pUvBase += mVData->vertexStart * uvInc;

		// find position
		const VertexElement *posElem = dcl->findElementBySemantic(VES_POSITION);
		if (posElem->getSource() == uvElem->getSource())
		{
			pPosBase = pUvBase;
			posInc = uvInc;
		}
		else
		{
			// A different buffer
			posBuf = bind->getBuffer(posElem->getSource());
			pPosBase = static_cast<unsigned char*>(
				posBuf->lock(HardwareBuffer::HBL_READ_ONLY));
			posInc = posBuf->getVertexSize();
			// offset for vertex start
			pPosBase += mVData->vertexStart * posInc;
		}
		// find a normal buffer
		const VertexElement *normElem = dcl->findElementBySemantic(VES_NORMAL);
		if (!normElem)
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
			"No vertex normals found", 
			"TangentSpaceCalc::build");

		if (normElem->getSource() == uvElem->getSource())
		{
			pNormBase = pUvBase;
			normInc = uvInc;
		}
		else if (normElem->getSource() == posElem->getSource())
		{
			// normals are in the same buffer as position
			// this condition arises when an animated(skeleton) mesh is not built with 
			// an edge list buffer ie no shadows being used.
			pNormBase = pPosBase;
			normInc = posInc;
		}
		else
		{
			// A different buffer
			normBuf = bind->getBuffer(normElem->getSource());
			pNormBase = static_cast<unsigned char*>(
				normBuf->lock(HardwareBuffer::HBL_READ_ONLY));
			normInc = normBuf->getVertexSize();
			// offset for vertex start
			pNormBase += mVData->vertexStart * normInc;
		}

		// Preinitialise vertex info
		mVertexArray.clear();
		mVertexArray.resize(mVData->vertexCount);

		float* pFloat;
		VertexInfo* vInfo = &(mVertexArray[0]);
		for (size_t v = 0; v < mVData->vertexCount; ++v, ++vInfo)
		{
			posElem->baseVertexPointerToElement(pPosBase, &pFloat);
			vInfo->pos.x = *pFloat++;
			vInfo->pos.y = *pFloat++;
			vInfo->pos.z = *pFloat++;
			pPosBase += posInc;

			normElem->baseVertexPointerToElement(pNormBase, &pFloat);
			vInfo->norm.x = *pFloat++;
			vInfo->norm.y = *pFloat++;
			vInfo->norm.z = *pFloat++;
			pNormBase += normInc;

			uvElem->baseVertexPointerToElement(pUvBase, &pFloat);
			vInfo->uv.x = *pFloat++;
			vInfo->uv.y = *pFloat++;
			pUvBase += uvInc;


		}

		// unlock buffers
		uvBuf->unlock();
		if (!posBuf.isNull())
		{
			posBuf->unlock();
		}
		if (!normBuf.isNull())
		{
			normBuf->unlock();
		}

	}
	//---------------------------------------------------------------------
	void TangentSpaceCalc::insertTangents(Result& res,
		VertexElementSemantic targetSemantic, unsigned short sourceTexCoordSet, 
		unsigned short index)
	{
		// Make a new tangents semantic or find an existing one
		VertexDeclaration *vDecl = mVData->vertexDeclaration ;
		VertexBufferBinding *vBind = mVData->vertexBufferBinding ;

		const VertexElement *tangentsElem = vDecl->findElementBySemantic(targetSemantic, index);
		bool needsToBeCreated = false;
		VertexElementType tangentsType = mStoreParityInW ? VET_FLOAT4 : VET_FLOAT3;

		if (!tangentsElem)
		{ // no tex coords with index 1
			needsToBeCreated = true ;
		}
		else if (tangentsElem->getType() != tangentsType)
		{
			//  buffer exists, but not 3D
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Target semantic set already exists but is not of the right size, therefore "
				"cannot contain tangents. You should delete this existing entry first. ",
				"TangentSpaceCalc::insertTangents");
		}

		HardwareVertexBufferSharedPtr targetBuffer, origBuffer;
		unsigned char* pSrc = NULL;

		if (needsToBeCreated)
		{
			// To be most efficient with our vertex streams,
			// tack the new tangents onto the same buffer as the
			// source texture coord set
			const VertexElement* prevTexCoordElem =
				mVData->vertexDeclaration->findElementBySemantic(
				VES_TEXTURE_COORDINATES, sourceTexCoordSet);
			if (!prevTexCoordElem)
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
					"Cannot locate the first texture coordinate element to "
					"which to append the new tangents.", 
					"Mesh::orgagniseTangentsBuffer");
			}
			// Find the buffer associated with  this element
			origBuffer = mVData->vertexBufferBinding->getBuffer(
				prevTexCoordElem->getSource());
			// Now create a new buffer, which includes the previous contents
			// plus extra space for the 3D coords
			targetBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
				origBuffer->getVertexSize() + VertexElement::getTypeSize(tangentsType),
				origBuffer->getNumVertices(),
				origBuffer->getUsage(),
				origBuffer->hasShadowBuffer() );
			// Add the new element
			tangentsElem = &(vDecl->addElement(
				prevTexCoordElem->getSource(),
				origBuffer->getVertexSize(),
				tangentsType,
				targetSemantic,
				index));
			// Set up the source pointer
			pSrc = static_cast<unsigned char*>(
				origBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
			// Rebind the new buffer
			vBind->setBinding(prevTexCoordElem->getSource(), targetBuffer);
		}
		else
		{
			// space already there
			origBuffer = mVData->vertexBufferBinding->getBuffer(
				tangentsElem->getSource());
			targetBuffer = origBuffer;
		}


		unsigned char* pDest = static_cast<unsigned char*>(
			targetBuffer->lock(HardwareBuffer::HBL_DISCARD));
		size_t origVertSize = origBuffer->getVertexSize();
		size_t newVertSize = targetBuffer->getVertexSize();
		for (size_t v = 0; v < origBuffer->getNumVertices(); ++v)
		{
			if (needsToBeCreated)
			{
				// Copy original vertex data as well 
				memcpy(pDest, pSrc, origVertSize);
				pSrc += origVertSize;
			}
			// Write in the tangent
			float* pTangent;
			tangentsElem->baseVertexPointerToElement(pDest, &pTangent);
			VertexInfo& vertInfo = mVertexArray[v];
			*pTangent++ = vertInfo.tangent.x;
			*pTangent++ = vertInfo.tangent.y;
			*pTangent++ = vertInfo.tangent.z;
			if (mStoreParityInW)
				*pTangent++ = (float)vertInfo.parity;

			// Next target vertex
			pDest += newVertSize;

		}
		targetBuffer->unlock();

		if (needsToBeCreated)
		{
			origBuffer->unlock();
		}
	}

}
