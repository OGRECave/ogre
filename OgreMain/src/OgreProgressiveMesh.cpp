/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

// The algorithm in this file is based heavily on:
/*
*  Progressive Mesh type Polygon Reduction Algorithm
*  by Stan Melax (c) 1998
*/

#include "OgreProgressiveMesh.h"
#include "OgreLodStrategyManager.h"
#include "OgreMeshManager.h"
#include "OgreSubMesh.h"
#include "OgreString.h"
#include "OgreHardwareBufferManager.h"
#include "OgreLogManager.h"

#if OGRE_DEBUG_MODE 
#define LOG_PROGRESSIVE_MESH_GENERATION 1
#else
#define LOG_PROGRESSIVE_MESH_GENERATION 0
#endif

#if LOG_PROGRESSIVE_MESH_GENERATION 
#include <iostream>
std::ofstream ofdebug;
#endif 

namespace Ogre {

	const unsigned char BitArray::bit_count[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
	const unsigned char BitArray::bit_mask[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
	
//#define IGNORE_UV_AND_NORMAL_COSTS
//#define CHECK_CALCULATED_NORMALS
	
#define NEVER_COLLAPSE_COST 99999.9f
	
	class VertexDataVariant
	{
	private:
		unsigned char*	mBaseDataPointer;
		unsigned char*	mCurDataPointer;
		size_t			mOffsetSize;
		
		// source dependent part, only first buffer of several with shared mSource holds lock on hardware vertex buffer
		int mSource;
		HardwareVertexBufferSharedPtr mBuffer;
		std::auto_ptr<VertexBufferLockGuard> mLockGuard;	// only first by source takes lock
		
		friend class VertexDataVariantList;
		
		VertexDataVariant(const VertexData * vertexData, const VertexElement* vertexElement, VertexDataVariant* bufferOwner)
			: mOffsetSize(0)
			, mSource(-1)
		{
			static float fakeDataBuffer[3] = { 0.0f, 0.0f, 0.0f }; // 12 bytes, can be safely used with zero mOffsetSize
			mBaseDataPointer = mCurDataPointer = (unsigned char*)fakeDataBuffer;
			
			if(NULL != vertexElement)
			{
				mSource = vertexElement->getSource();
				mBuffer = vertexData->vertexBufferBinding->getBuffer(mSource);
				
				// only first VertexDataVariant really locks buffer and store pointer to raw data
				if(NULL == bufferOwner)
				{
					// buffer is not locked yet, so lock it and became buffer owner
					mLockGuard.reset(new VertexBufferLockGuard(mBuffer, HardwareBuffer::HBL_READ_ONLY));
					bufferOwner = this;
				}
				
				// adjust whole vertex pointer to vertex part pointer
				vertexElement->baseVertexPointerToElement(bufferOwner->mLockGuard->pData, &mBaseDataPointer);
				mCurDataPointer = mBaseDataPointer;
				mOffsetSize = mBuffer->getVertexSize();
			}
		}
		
	public:
		bool isValid() const						{ return mOffsetSize != 0; }
		
		int getSource() const						{ return mSource; }
		unsigned char* getBaseDataPointer() const	{ return mBaseDataPointer; }
		unsigned char* getCurDataPointer() const	{ return mCurDataPointer; }
		size_t getOffsetSize() const				{ return mOffsetSize; }
		
		void reset()								{ mCurDataPointer = mBaseDataPointer; }
		void offset()								{ mCurDataPointer += mOffsetSize; }
		void offsetToElement(int itemIndex)			{ mCurDataPointer = mBaseDataPointer + itemIndex * mOffsetSize;	}
		
		Vector3 getNextVector3() 
		{	
			float* v = (float*)mCurDataPointer;
			mCurDataPointer += mOffsetSize;
			return Vector3(v[0], v[1], v[2]);
		}
		
		Vector2 getNextVector2()
		{
			float* v = (float*)mCurDataPointer;
			mCurDataPointer += mOffsetSize;
			return Vector2(v[0], v[1]);
		}		
	};
	
	class VertexDataVariantList
	{
	public:
		VertexDataVariant* create(const VertexData * vertexData, VertexElementSemantic sem)
		{
			const VertexElement* vertexElement = vertexData->vertexDeclaration->findElementBySemantic(sem);
			VertexDataVariant* bufferOwner = vertexElement ? getBySource(vertexElement->getSource()) : NULL;
			mVdList.push_back(VertexDataVariantSharedPtr(new VertexDataVariant(vertexData, vertexElement, bufferOwner)));
			return mVdList.back().get();
		}
		
	private:
		VertexDataVariant* getBySource(int source)
		{
			for(vd_list_t::const_iterator it = mVdList.begin(); it != mVdList.end(); ++it)
				if((*it)->getSource() == source)
					return it->get();
			
			return NULL;
		}
		
	private:
		typedef SharedPtr<VertexDataVariant> VertexDataVariantSharedPtr;
		typedef vector<VertexDataVariantSharedPtr>::type vd_list_t;
		vd_list_t mVdList;
	};
	
	class IndexDataVariant;
	typedef SharedPtr<IndexDataVariant> IndexDataVariantSharedPtr;
	
	class IndexDataVariant
	{
	private:
		HardwareIndexBufferSharedPtr mBuffer;
		unsigned char* mBaseDataPointer;
		unsigned char* mCurDataPointer;
		size_t mIndexCount;
		size_t mOffsetSize;
		bool mUse32bitIndexes;
		std::auto_ptr<IndexBufferLockGuard> mLockGuard;
		
		IndexDataVariant(const IndexData * indexData, HardwareBuffer::LockOptions lockOpt)
			: mIndexCount(0)
			, mOffsetSize(0)
			, mUse32bitIndexes(false)
		{
			static int fakeIndexBuffer = 0;	// 4 bytes, can be safely used with zero mOffsetSize
			mBaseDataPointer = mCurDataPointer = (unsigned char*)&fakeIndexBuffer;
			
			if(NULL == indexData) return;
			
			mBuffer = indexData->indexBuffer;
			if(mBuffer.isNull()) return;
			
			mIndexCount = indexData->indexCount;
			if(0 == mIndexCount) return;
			
			mUse32bitIndexes = (mBuffer->getType() == HardwareIndexBuffer::IT_32BIT);
			mOffsetSize = mUse32bitIndexes ? sizeof(unsigned int) : sizeof(unsigned short);
			
			mLockGuard.reset(new IndexBufferLockGuard(mBuffer, lockOpt));
			mBaseDataPointer = (unsigned char*)mLockGuard->pData;
			
			reset();
		}
		
		bool isValid() const { return mOffsetSize != 0; }
		
	public:
		
		static IndexDataVariantSharedPtr create(const IndexData * indexData, HardwareBuffer::LockOptions lockOpt = HardwareBuffer::HBL_READ_ONLY)
		{
			IndexDataVariantSharedPtr p(new IndexDataVariant(indexData, lockOpt));
			return p->isValid() ? p : IndexDataVariantSharedPtr();	
		}
		
		unsigned char* getBaseDataPointer() const	{ return mBaseDataPointer; }
		unsigned char* getCurDataPointer() const	{ return mCurDataPointer; }
		size_t getOffsetSize() const				{ return mOffsetSize; }
		size_t getIndexCount() const				{ return mIndexCount; }
		bool is32bitIndexes() const					{ return mUse32bitIndexes; }
		
		void reset()								{ mCurDataPointer = mBaseDataPointer; }
		void offsetToElement(int itemIndex)			{ mCurDataPointer = getBaseDataPointer() + itemIndex * getOffsetSize();	}
		unsigned getNextIndex()						{ unsigned idx = mUse32bitIndexes ? *(unsigned int*)mCurDataPointer : *(unsigned short*)mCurDataPointer; mCurDataPointer += mOffsetSize; return idx; }
		
		void markUsedVertices(BitArray& bitmask) const
		{
			if(mUse32bitIndexes)
			{
				for(const unsigned int *ptr = (const unsigned int*)mBaseDataPointer, *end_ptr = ptr + mIndexCount; ptr < end_ptr; ++ptr)
					bitmask.setBit(*ptr);
			}
			else
			{
				for(const unsigned short *ptr = (const unsigned short*)mBaseDataPointer, *end_ptr = ptr + mIndexCount; ptr < end_ptr; ++ptr)
					bitmask.setBit(*ptr);
			}
		}
		
		void createIndexData(IndexData* pIndexData, bool use16bitIndexes, vector<unsigned>::type* indexMap)
		{
			size_t indexCount = getIndexCount();
			reset();
			
			pIndexData->indexStart = 0;
			pIndexData->indexCount = indexCount;
			pIndexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
					  use16bitIndexes ? HardwareIndexBuffer::IT_16BIT : HardwareIndexBuffer::IT_32BIT,
					  indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
			
			IndexBufferLockGuard outIdataLock(pIndexData->indexBuffer, HardwareBuffer::HBL_DISCARD);
			
			unsigned short* pShortOut = use16bitIndexes ? (unsigned short*)outIdataLock.pData : NULL;
			unsigned int* pIntOut = use16bitIndexes ? NULL : (unsigned int*)outIdataLock.pData;
			
			if(use16bitIndexes)
			{
				for(size_t n = 0; n < indexCount; ++n)
				{
					unsigned idx = getNextIndex();
					*pShortOut++ = indexMap ? (*indexMap)[idx] : idx;
				}
			}
			else
			{
				for(size_t n = 0; n < indexCount; ++n)
				{
					unsigned idx = getNextIndex();
					*pIntOut++ = indexMap ? (*indexMap)[idx] : idx;
				}
			}
		}
	};	
	
    //---------------------------------------------------------------------
	ProgressiveMesh::ProgressiveMesh(SubMesh* pSubMesh)
		: mSubMesh(pSubMesh)
		, mCurrNumIndexes(0)
		, mVertexComponentFlags(0)
    {
		// ignore un-indexed submeshes
		if(pSubMesh->indexData->indexCount == 0)
		{
			mSubMesh = NULL;
			return;
		}
		
		Ogre::Mesh* pMesh = pSubMesh->parent;
		Real sqrDiag = pMesh->getBounds().getSize().squaredLength();
		mInvSquaredBoundBoxDiagonal = (0.0 != sqrDiag) ? 1.0 / sqrDiag : 0.0;
		
		mNextWorstCostHint = 0;
		mInvalidCostCount = 0;
		mRemovedVertexDuplicatesCount = 0;
		
		mVertexData	= pSubMesh->useSharedVertices ? pMesh->sharedVertexData : pSubMesh->vertexData;
		mIndexData		= pSubMesh->indexData;
		
		mInvalidCostMask.resize(mVertexData->vertexCount);
        addWorkingData(mVertexData, mIndexData);
    }
    //---------------------------------------------------------------------
    ProgressiveMesh::~ProgressiveMesh()
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::addExtraVertexPositionBuffer(const VertexData* vertexData)
    {
        addWorkingData(vertexData, mIndexData);
    }
    //---------------------------------------------------------------------
	void ProgressiveMesh::initializeProgressiveMeshList(ProgressiveMeshList& pmList, Mesh* pMesh)
	{
		size_t subMeshCount = pMesh->getNumSubMeshes();
		pmList.reserve(subMeshCount);
		for(size_t i = 0; i < subMeshCount; ++i)
		{
			SubMesh* pSubMesh = pMesh->getSubMesh(i);
			pmList.push_back(OGRE_NEW ProgressiveMesh(pSubMesh));
		}		
	}
    //---------------------------------------------------------------------
	void ProgressiveMesh::freeProgressiveMeshList(ProgressiveMeshList* pmList)
	{
		for(ProgressiveMeshList::iterator it = pmList->begin(); it != pmList->end(); ++it)
		{
			OGRE_DELETE *it;
			*it = NULL;
		}
	}
	//---------------------------------------------------------------------
	bool ProgressiveMesh::generateLodLevels(Mesh* pMesh, const LodValueList& lodValues,
								 VertexReductionQuota reductionMethod, Real reductionValue)
	{
#if OGRE_DEBUG_MODE
		pMesh->getLodStrategy()->assertSorted(lodValues);
#endif
		
		pMesh->removeLodLevels();
		
		LogManager::getSingleton().stream()	<< "Generating " << lodValues.size()	<< " lower LODs for mesh " << pMesh->getName();
		
		// Set up data for reduction
		ProgressiveMeshList pmList;
		initializeProgressiveMeshList(pmList, pMesh);
		
		bool generated = build(pmList, pMesh->getLodStrategy(), lodValues, reductionMethod, reductionValue);
		
		if(generated)
		{
			// transfer all LODs from ProgressiveMesh to the real one
			size_t subMeshCount = pMesh->getNumSubMeshes();
			for(size_t i = 0; i < subMeshCount; ++i)
				pMesh->getSubMesh(i)->mLodFaceList.swap(pmList[i]->mLodFaceList);
			
			// register them
			LodStrategy *lodStrategy = LodStrategyManager::getSingleton().getStrategy(pMesh->getLodStrategy()->getName());
            bakeLodUsage(pMesh, lodStrategy, lodValues, false);
		}
		
		freeProgressiveMeshList(&pmList);
		
		return generated;
	}
    //---------------------------------------------------------------------
	MeshPtr ProgressiveMesh::generateSimplifiedMesh(const String& name, const String& groupName, Mesh* inMesh,
													bool dropOriginalGeometry, const LodValueList& lodValues,
													VertexReductionQuota reductionMethod, Real reductionValue,
													size_t* removedVertexDuplicatesCount)
	{
#if OGRE_DEBUG_MODE
		inMesh->getLodStrategy()->assertSorted(lodValues);
#endif
		LogManager::getSingleton().stream()	<< "Generating simplified mesh " << name << " for mesh " << inMesh->getName();

		// Set up data for reduction
		ProgressiveMeshList pmList;
		initializeProgressiveMeshList(pmList, inMesh);
		
		// Perform reduction
		build(pmList, inMesh->getLodStrategy(), lodValues, reductionMethod, reductionValue);

		// Bake new simplified mesh
		MeshPtr simplifiedMesh = MeshManager::getSingleton().createManual(name, groupName);
		bakeSimplifiedMesh(simplifiedMesh.get(), inMesh, pmList, dropOriginalGeometry);
		LodStrategy *lodStrategy = LodStrategyManager::getSingleton().getStrategy(inMesh->getLodStrategy()->getName());
        bakeLodUsage(simplifiedMesh.get(), lodStrategy, lodValues, dropOriginalGeometry);
		
		// Return some statistic
		if(removedVertexDuplicatesCount)
		{
			size_t duplicatesCount = 0;
			for(ProgressiveMeshList::iterator it = pmList.begin(); it != pmList.end(); ++it)
				duplicatesCount += (*it)->mRemovedVertexDuplicatesCount;
			*removedVertexDuplicatesCount = duplicatesCount;
		}
		
		freeProgressiveMeshList(&pmList);
		return simplifiedMesh;
	}
    //---------------------------------------------------------------------	
	bool ProgressiveMesh::build(ProgressiveMeshList& pmInList,
								const LodStrategy *lodStrategy, const LodValueList& lodValues,
								VertexReductionQuota quota, Real reductionValue)
	{		
		assert(!pmInList.empty());
		bool generated = false;
		
		size_t numVerts = 0;
		
		ProgressiveMeshList pmBuildList;
		
		for(ProgressiveMeshList::iterator i = pmInList.begin(); i != pmInList.end(); ++i)
		{
			ProgressiveMesh* p = *i;
			if(NULL == p->mSubMesh)
				continue; // dummy, skip it
			
			p->computeAllCosts();
			
			// Init
			p->mCurrNumIndexes = (Ogre::RenderOperation::OT_TRIANGLE_LIST == p->mSubMesh->operationType) ?
				p->mIndexData->indexCount : (p->mIndexData->indexCount - 2) * 3;
			
#if LOG_PROGRESSIVE_MESH_GENERATION			
			StringUtil::StrStreamType logname;
			logname << "pm_before_" << std::distance(pmInList.begin(), i) << ".log";
			(*i)->dumpContents(logname.str());
#endif
			numVerts += p->mWorstCostsSize;
			
			pmBuildList.push_back(p);
		}
		
		ProgressiveMeshList pmList(pmBuildList);
		
		// if any one of this two checks is failed - we complete one LOD generation
		size_t numCollapses = numVerts;			// unlimited
		Real costLimit = NEVER_COLLAPSE_COST;	// unlimited
		
		bool abandon = false;
				
		for (LodValueList::const_iterator lod = lodValues.begin(); lod != lodValues.end(); ++lod)
		{
			int level = std::distance(lodValues.begin(), lod);
			
			// adjust LOD target limits
			switch(quota)
			{
				case VRQ_CONSTANT:		
					numCollapses = static_cast<size_t>(reductionValue);
					break;
					
				case VRQ_PROPORTIONAL:
					numCollapses = static_cast<size_t>(numVerts * reductionValue);
					numVerts -= numCollapses;
					break;
					
				case VRQ_ERROR_COST:
					// we must increase cost limit with each next lod level proportionally to squared distance ratio or inverted pixel area ratio
					Real reductionValueMultiplier = lodStrategy->transformBias(lodStrategy->transformUserValue(lodValues[0]) / lodStrategy->transformUserValue(lodValues[level]));
					assert(level == 0 || reductionValueMultiplier > 1.0);
					costLimit = reductionValue * reductionValueMultiplier;
					break;
			}
						
			// NB if 'abandon' is set, we stop reducing 
			// However, we still bake the number of LODs requested, even if it 
			// means they are the same
			while(numCollapses > 0 && !abandon)
			{	
				ProgressiveMesh* pmCur;			//out
				CostIndexPair* collapseTarget;	// out
				getNextCollapser(pmList, pmCur, collapseTarget);

				// we found collapse target, but may be we must not collapse it
				if(collapseTarget != NULL)
				{
					assert(collapseTarget->first != NEVER_COLLAPSE_COST);
					if(VRQ_ERROR_COST == quota)
					{
						Real cost = collapseTarget->first;
						if(cost > costLimit)
							collapseTarget = NULL;
					}
					else // VRQ_CONSTANT, VRQ_PROPORTIONAL
					{
						if(getInvalidCostCount(pmList) >= numCollapses)
							collapseTarget = NULL; // need recalc
					}
				}
				
				// if we have not collapse target but have invalid costs - recalc them
				if(collapseTarget == NULL)
				{
					if(recomputeInvalidCosts(pmList))
					{
						// a some invalid costs was recomputed and we should try to continue collapsing
						// because the recomputed best cost can be less than level limit;
						continue;
					}
					else
					{
						abandon = pmList.empty();
						break; // an invalid costs is not found and we complete collapsing for the current LOD
					}
				}
				
				// OK, we decide to collapse this target
				assert(collapseTarget);				
				assert(pmCur);
				assert(numCollapses > 0);
				
				// Collapse on every buffer
				WorkingDataList::iterator idata, idataend;
				idataend = pmCur->mWorkingData.end();
				
				for (idata = pmCur->mWorkingData.begin(); idata != idataend; ++idata)
				{
					PMVertex* collapser = &(idata->mVertList[collapseTarget->second]);
					
					if(collapser->face.size() == pmCur->mCurrNumIndexes / 3
					|| collapser->collapseTo == NULL)
					{
						// Must have run out of valid collapsables
						pmList.erase(std::remove(pmList.begin(), pmList.end(), pmCur), pmList.end());
						abandon = pmList.empty();						
						break;
					}
										
#if LOG_PROGRESSIVE_MESH_GENERATION 
					ofdebug << "Collapsing index " << (unsigned int)collapser->index <<	"(border: "<< (collapser->mBorderStatus == PMVertex::BS_BORDER ? "yes" : "no") <<
					") to " << (unsigned int)collapser->collapseTo->index << "(border: "<< (collapser->collapseTo->mBorderStatus == PMVertex::BS_BORDER ? "yes" : "no") <<
					")" << std::endl;
#endif
					assert(collapser->collapseTo->removed == false);
					assert(pmCur->mCurrNumIndexes > 0);
					
					pmCur->collapse(collapser);
					
					assert(pmCur->mCurrNumIndexes > 0);
				}

				// we must never return to it
				collapseTarget->first = NEVER_COLLAPSE_COST;
				--numCollapses;
			}
			// end of one LOD collapsing loop
			
			// Bake a new LOD and add it to the list
			for(ProgressiveMeshList::iterator i = pmBuildList.begin(); i != pmBuildList.end(); ++i)
			{
				ProgressiveMesh* p = *i;
				assert(NULL != p->mSubMesh); //dummy can't happen here
				
#if LOG_PROGRESSIVE_MESH_GENERATION
				StringUtil::StrStreamType logname;
				ProgressiveMeshList::iterator t = std::find(pmInList.begin(), pmInList.end(), p);
				assert(t != pmInList.end());
				logname << "pm_" << std::distance(pmInList.begin(), t) << "__level_" << level << ".log";
				(*i)->dumpContents(logname.str());
#endif								
				IndexData* lodData = NULL;
				
				if(p->mCurrNumIndexes != p->mIndexData->indexCount)
				{
					assert(p->mCurrNumIndexes > 0);
					
					lodData = OGRE_NEW IndexData();
					p->bakeNewLOD(lodData);
					generated = true;
				}
				else
				{
					p->mRemovedVertexDuplicatesCount = 0;
					lodData = p->mSubMesh->indexData->clone();
				}
				
				assert(NULL != lodData);
				
				p->mLodFaceList.push_back(lodData);
			}
		}
		
		return generated;
	}	
	//---------------------------------------------------------------------
    void ProgressiveMesh::addWorkingData(const VertexData * vertexData, 
        const IndexData * indexData)
    {
		if(0 == vertexData->vertexCount || 0 == indexData->indexCount)
			return;
		
        // Insert blank working data, then fill 
        mWorkingData.push_back(PMWorkingData());
        PMWorkingData& work = mWorkingData.back();

        // Build vertex list
		// Resize face list (this will always be this big)
		work.mFaceVertList.resize(vertexData->vertexCount);
		// Also resize common vert list to max, to avoid reallocations
		work.mVertList.resize(vertexData->vertexCount);
		
		VertexDataVariantList vdVariantList;
		
		VertexDataVariant* vertexDataBuffer = vdVariantList.create(vertexData, VES_POSITION);
		VertexDataVariant* normalDataBuffer = vdVariantList.create(vertexData, VES_NORMAL);
		VertexDataVariant* uvDataBuffer = vdVariantList.create(vertexData, VES_TEXTURE_COORDINATES);
		
		mVertexComponentFlags |= (1 << VES_POSITION);
		mVertexComponentFlags |= (1 << VES_NORMAL);
		mVertexComponentFlags |= (1 << VES_TEXTURE_COORDINATES);
		
		IndexDataVariantSharedPtr indexDataVar(IndexDataVariant::create(indexData));
		
		if(indexDataVar.isNull())
			return;

		//make used index list
		BitArray usedVertices(vertexData->vertexCount);
		indexDataVar->markUsedVertices(usedVertices);
		
		mWorstCostsSize = usedVertices.getBitsCount();
		mWorstCosts.resize(mWorstCostsSize);
		
		PMVertex* vBase = &work.mVertList.front();
		WorstCostList::iterator it = mWorstCosts.begin();
		bool someVerticesWasSkipped = false;
		for(unsigned idx = 0, end_idx = vertexData->vertexCount; idx < end_idx; ++idx)
		{
			// skip unused vertices
			if(!usedVertices.getBit(idx))
			{
				someVerticesWasSkipped = true;
				continue;
			}

			// resync vertex data offset if some vertices was skipped
			if(someVerticesWasSkipped)
			{
				someVerticesWasSkipped = false;
				
				vertexDataBuffer->offsetToElement(idx);
				normalDataBuffer->offsetToElement(idx);
				uvDataBuffer->offsetToElement(idx);
			}
			
			// store reference to used vertex
			it->first = 0.0f;
			it->second = idx;
			++it;
			
			// read vertex data
			PMVertex* commonVert = vBase + idx;
			commonVert->setDetails(idx, vertexDataBuffer->getNextVector3(), normalDataBuffer->getNextVector3(), uvDataBuffer->getNextVector2());
        }
		
        // Build tri list
        size_t numTris = (Ogre::RenderOperation::OT_TRIANGLE_LIST == mSubMesh->operationType) ?
			mIndexData->indexCount / 3 : mIndexData->indexCount - 2;
		
        work.mTriList.reserve(numTris); // reserved tri list
		
		PMFaceVertex* fvBase = &work.mFaceVertList.front();
		if(Ogre::RenderOperation::OT_TRIANGLE_LIST == mSubMesh->operationType)
		{
			for (size_t i = 0, vi = 0; i < numTris; ++i)
			{
				unsigned int vindex;
				
				PMFaceVertex *v0, *v1, *v2;
				
				vindex = indexDataVar->getNextIndex();
				
				v0 = fvBase + vindex;
				v0->commonVertex = vBase + vindex;
				v0->realIndex = vindex;
				
				vindex = indexDataVar->getNextIndex();
				
				v1 = fvBase + vindex;
				v1->commonVertex = vBase + vindex;
				v1->realIndex = vindex;
				
				vindex = indexDataVar->getNextIndex();
				
				v2 = fvBase + vindex;
				v2->commonVertex = vBase + vindex;
				v2->realIndex = vindex;
				
				if(v0!=v1 && v1!=v2 && v2!=v0) // see assertion: OgreProgressiveMesh.cpp, line: 723, condition: v0!=v1 && v1!=v2 && v2!=v0
				{
					work.mTriList.push_back(PMTriangle());
					work.mTriList.back().setDetails(vi++, v0, v1, v2);
				}
			}
		}
		else if(Ogre::RenderOperation::OT_TRIANGLE_STRIP == mSubMesh->operationType)
		{
			bool tSign = true;
			for (size_t i = 0, vi = 0; i < numTris; ++i)
			{
				unsigned int vindex;
				
				PMFaceVertex *v0, *v1, *v2;
				
				indexDataVar->offsetToElement(i);
				vindex = indexDataVar->getNextIndex();
				
				v0 = fvBase + vindex;
				v0->commonVertex = vBase + vindex;
				v0->realIndex = vindex;
				
				vindex = indexDataVar->getNextIndex();
				
				v1 = fvBase + vindex;
				v1->commonVertex = vBase + vindex;
				v1->realIndex = vindex;
				
				vindex = indexDataVar->getNextIndex();
				
				v2 = fvBase + vindex;
				v2->commonVertex = vBase + vindex;
				v2->realIndex = vindex;
				
				if(v0!=v1 && v1!=v2 && v2!=v0) // see assertion: OgreProgressiveMesh.cpp, line: 723, condition: v0!=v1 && v1!=v2 && v2!=v0
				{
					work.mTriList.push_back(PMTriangle());
					work.mTriList.back().setDetails(vi++, tSign ? v0 : v1, tSign ? v1 : v0, v2);
				}
				
				tSign = !tSign;
			}			
		}
		else //FAN
		{
			for (size_t i = 0, vi = 0; i < numTris; ++i)
			{
				unsigned int vindex;
				
				PMFaceVertex *v0, *v1, *v2;
				
				indexDataVar->offsetToElement(0);
				vindex = indexDataVar->getNextIndex();
				
				v0 = fvBase + vindex;
				v0->commonVertex = vBase + vindex;
				v0->realIndex = vindex;
				
				indexDataVar->offsetToElement(i);
				vindex = indexDataVar->getNextIndex();
				
				v1 = fvBase + vindex;
				v1->commonVertex = vBase + vindex;
				v1->realIndex = vindex;
				
				vindex = indexDataVar->getNextIndex();
				
				v2 = fvBase + vindex;
				v2->commonVertex = vBase + vindex;
				v2->realIndex = vindex;
				
				if(v0!=v1 && v1!=v2 && v2!=v0) // see assertion: OgreProgressiveMesh.cpp, line: 723, condition: v0!=v1 && v1!=v2 && v2!=v0
				{
					work.mTriList.push_back(PMTriangle());
					work.mTriList.back().setDetails(vi++, v0, v1, v2);
				}
			}			
		}
		indexDataVar.setNull();
		
		// try to merge borders, to increase algorithm effectiveness
		mergeWorkingDataBorders();
    }
	
	/** Comparator for unique vertex list
	 */		
	struct ProgressiveMesh::vertexLess
	{
		bool operator()(PMVertex* v1, PMVertex* v2) const
		{
			if (v1->position.x < v2->position.x) return true;
			else if(v1->position.x > v2->position.x) return false;
			
			if (v1->position.y < v2->position.y) return true;
			else if(v1->position.y > v2->position.y) return false;
			
			if (v1->position.z < v2->position.z) return true;
			else if(v1->position.z > v2->position.z) return false;
			
			
			if (v1->normal.x < v2->normal.x) return true;
			else if(v1->normal.x > v2->normal.x) return false;
			
			if (v1->normal.y < v2->normal.y) return true;
			else if(v1->normal.y > v2->normal.y) return false;
			
			if (v1->normal.z < v2->normal.z) return true;
			else if(v1->normal.z > v2->normal.z) return false;
			
			
			if (v1->uv.x < v2->uv.x) return true;
			else if(v1->uv.x > v2->uv.x) return false;
			
			if (v1->uv.y < v2->uv.y) return true;
			else if(v1->uv.y > v2->uv.y) return false;
			
			return false;
		}
	};			
	
    void ProgressiveMesh::mergeWorkingDataBorders()
	{
		IndexDataVariantSharedPtr indexDataVar(IndexDataVariant::create(mIndexData));
		
		if(indexDataVar.isNull())
			return;
		
		PMWorkingData& work = mWorkingData.back();
				
		typedef std::map<PMVertex*, size_t, vertexLess> CommonVertexMap;
		CommonVertexMap commonBorderVertexMap;
		
		std::map<unsigned int /* original index */, unsigned int /* new index (after merge) */> mappedIndexes;
		
		// try to merge borders to borders
		WorstCostList newUniqueIndexes;

		// we will use border status from first buffer only - is this right?
		PMVertex* vBase = &work.mVertList.front();
		
		// iterate over used vertices
		for(WorstCostList::iterator it = mWorstCosts.begin(), end_it = mWorstCosts.end(); it != end_it; ++it)
        {
			unsigned idx = it->second;
			
			PMVertex* v = vBase + idx;
			v->initBorderStatus();
			
			CommonVertexMap::iterator iCommonBorderVertex = commonBorderVertexMap.end();
			if(PMVertex::BS_BORDER == v->mBorderStatus)
			{
				//vertex is a border, try to find it in the common border vertex map at first
				iCommonBorderVertex = commonBorderVertexMap.find(v);
			
				// if found but not near enough - ignore it
				if(iCommonBorderVertex != commonBorderVertexMap.end() && !iCommonBorderVertex->first->isNearEnough(v))
					iCommonBorderVertex = commonBorderVertexMap.end();
				
			}					
			
			//if vertex is not found in the common border vertex map
			if(iCommonBorderVertex == commonBorderVertexMap.end())
			{
				if(PMVertex::BS_BORDER == v->mBorderStatus)
					commonBorderVertexMap.insert(CommonVertexMap::value_type(v, idx));
				mappedIndexes[idx] = idx;       // old and new indexes should be equal
				newUniqueIndexes.push_back(*it); // update common unique index list
			}
			else // merge vertices
			{
				// vertex is found in the common border vertex map
				// the triangles will use [iCommonBorderVertex->second] instead of [*idx]
				mappedIndexes[idx] = iCommonBorderVertex->second;
					
				// set border status for the founded vertex as BF_UNKNOWN
				// (the border status will reinitialized at the end of function since we mark it as not defined)
				iCommonBorderVertex->first->mBorderStatus = PMVertex::BS_UNKNOWN;
				++mRemovedVertexDuplicatesCount;
			}
			
			//the neighbor & face must be cleaned now (it will be recreated after merging)
			v->neighbor.clear();
			v->face.clear();
        }
		
        // Rebuild tri list by mapped indexes
        size_t numTris = (Ogre::RenderOperation::OT_TRIANGLE_LIST == mSubMesh->operationType) ?
			mIndexData->indexCount / 3 : mIndexData->indexCount - 2;
		
		work.mTriList.clear();
        work.mTriList.reserve(numTris); // reserved tri list
		
		PMFaceVertex* fvBase = &work.mFaceVertList.front();
		if(Ogre::RenderOperation::OT_TRIANGLE_LIST == mSubMesh->operationType)
		{
			for (size_t i = 0, vi = 0; i < numTris; ++i)
			{
				unsigned int vindex;
				
				PMFaceVertex *v0, *v1, *v2;
				
				vindex = indexDataVar->getNextIndex(); //get original index from the index buffer
				vindex = mappedIndexes[vindex]; //vindex will replaced if needed
				
				v0 = fvBase + vindex;
				v0->commonVertex = vBase + vindex;
				v0->realIndex = vindex;
								
				vindex = indexDataVar->getNextIndex(); //get original index from the index buffer
				vindex = mappedIndexes[vindex]; //vindex will replaced if needed
				
				v1 = fvBase + vindex;
				v1->commonVertex = vBase + vindex;
				v1->realIndex = vindex;
								
				vindex = indexDataVar->getNextIndex(); //get original index from the index buffer
				vindex = mappedIndexes[vindex]; //vindex will replaced if needed
				
				v2 = fvBase + vindex;
				v2->commonVertex = vBase + vindex;
				v2->realIndex = vindex;
								
				if(v0!=v1 && v1!=v2 && v2!=v0) // see assertion: OgreProgressiveMesh.cpp, line: 723, condition: v0!=v1 && v1!=v2 && v2!=v0
				{
					work.mTriList.push_back(PMTriangle());
					work.mTriList.back().setDetails(vi++, v0, v1, v2);
				}
			}
		}
		else if(Ogre::RenderOperation::OT_TRIANGLE_STRIP == mSubMesh->operationType)
		{
			bool tSign = true;
			for (size_t i = 0, vi = 0; i < numTris; ++i)
			{
				unsigned int vindex;
				
				PMFaceVertex *v0, *v1, *v2;
				
				indexDataVar->offsetToElement(i);
				vindex = indexDataVar->getNextIndex(); //get original index from the index buffer
				vindex = mappedIndexes[vindex]; //vindex will replaced if needed
				
				v0 = fvBase + vindex;
				v0->commonVertex = vBase + vindex;
				v0->realIndex = vindex;
								
				vindex = indexDataVar->getNextIndex(); //get original index from the index buffer
				vindex = mappedIndexes[vindex]; //vindex will replaced if needed
				
				v1 = fvBase + vindex;
				v1->commonVertex = vBase + vindex;
				v1->realIndex = vindex;
								
				vindex = indexDataVar->getNextIndex(); //get original index from the index buffer
				vindex = mappedIndexes[vindex]; //vindex will replaced if needed
				
				v2 = fvBase + vindex;
				v2->commonVertex = vBase + vindex;
				v2->realIndex = vindex;
				
				if(v0!=v1 && v1!=v2 && v2!=v0) // see assertion: OgreProgressiveMesh.cpp, line: 723, condition: v0!=v1 && v1!=v2 && v2!=v0
				{
					work.mTriList.push_back(PMTriangle());
					work.mTriList.back().setDetails(vi++, tSign ? v0 : v1, tSign ? v1 : v0, v2);
				}
				
				tSign = !tSign;
			}			
		}
		else //FAN
		{
			for (size_t i = 0, vi = 0; i < numTris; ++i)
			{
				unsigned int vindex;
				
				PMFaceVertex *v0, *v1, *v2;
				
				indexDataVar->offsetToElement(0);
				vindex = indexDataVar->getNextIndex(); //get original index from the index buffer
				vindex = mappedIndexes[vindex]; //vindex will replaced if needed
				
				v0 = fvBase + vindex;
				v0->commonVertex = vBase + vindex;
				v0->realIndex = vindex;
				
				indexDataVar->offsetToElement(i);				
				vindex = indexDataVar->getNextIndex(); //get original index from the index buffer
				vindex = mappedIndexes[vindex]; //vindex will replaced if needed
				
				v1 = fvBase + vindex;
				v1->commonVertex = vBase + vindex;
				v1->realIndex = vindex;
								
				vindex = indexDataVar->getNextIndex(); //get original index from the index buffer
				vindex = mappedIndexes[vindex]; //vindex will replaced if needed
				
				v2 = fvBase + vindex;
				v2->commonVertex = vBase + vindex;
				v2->realIndex = vindex;
				
				if(v0!=v1 && v1!=v2 && v2!=v0) // see assertion: OgreProgressiveMesh.cpp, line: 723, condition: v0!=v1 && v1!=v2 && v2!=v0
				{
					work.mTriList.push_back(PMTriangle());
					work.mTriList.back().setDetails(vi++, v0, v1, v2);
				}
			}			
		}
		
		//update the common unique index list (it should be less than before merge)
		mWorstCosts = newUniqueIndexes;		
		mWorstCostsSize = mWorstCosts.size();
		
		for(WorstCostList::iterator it = mWorstCosts.begin(), end_it = mWorstCosts.end(); it != end_it; ++it)
        {
			PMVertex* v = vBase + it->second;
			if(PMVertex::BS_UNKNOWN == v->mBorderStatus)
				v->initBorderStatus();

			if(0 == (mVertexComponentFlags & (1 << VES_NORMAL)))
				v->calculateNormal();
#ifdef CHECK_CALCULATED_NORMALS
			else
			{
				Vector3 savedNormal = v->normal;
				v->calculateNormal();
				assert(v->normal.dotProduct(savedNormal.normalisedCopy()) > 0.5f);
				v->normal = savedNormal();
			}
#endif				
		}
	}
	//---------------------------------------------------------------------	
	bool ProgressiveMesh::collapseInvertsNormals(PMVertex *src, PMVertex *dest) const
	{
		// Degenerate case check
		// Are we going to invert a face normal of one of the neighbouring faces?
		// Can occur when we have a very small remaining edge and collapse crosses it
		// Look for a face normal changing by > 90 degrees
		for(PMVertex::FaceList::iterator srcface = src->face.begin(), srcfaceend = src->face.end(); srcface != srcfaceend; ++srcface)
		{
			// Ignore the deleted faces (those including src & dest)
			if( !(*srcface)->hasCommonVertex(dest) )
			{
				// Test the new face normal
				PMVertex *v0, *v1, *v2;
				// Replace src with dest wherever it is
				v0 = ( (*srcface)->vertex[0]->commonVertex == src) ? dest : (*srcface)->vertex[0]->commonVertex;
				v1 = ( (*srcface)->vertex[1]->commonVertex == src) ? dest : (*srcface)->vertex[1]->commonVertex;
				v2 = ( (*srcface)->vertex[2]->commonVertex == src) ? dest : (*srcface)->vertex[2]->commonVertex;
				
				// Cross-product 2 edges
				Vector3 e1 = v1->position - v0->position; 
				Vector3 e2 = v2->position - v1->position;
				
				Vector3 newNormal = e1.crossProduct(e2);
				
				// Dot old and new face normal
				// If < 0 then more than 90 degree difference
				if (newNormal.dotProduct( (*srcface)->normal ) < 0.0f )
					return true;
			}
		}
		return false;
	}
    //---------------------------------------------------------------------	
    Real ProgressiveMesh::computeEdgeCollapseCost(PMVertex *src, PMVertex *dest) const
    {
        // if we collapse edge uv by moving src to dest then how 
        // much different will the model change, i.e. how much "error".
        // The method of determining cost was designed in order 
        // to exploit small and coplanar regions for
        // effective polygon reduction.
        Vector3 edgeVector = src->position - dest->position;
		
        Real cost;

		PMVertex::FaceList::iterator srcfacebegin = src->face.begin();
		PMVertex::FaceList::iterator srcfaceend = src->face.end();
		
        // find the "sides" triangles that are on the edge uv
        PMVertex::FaceList& sides = mEdgeAdjacentSides;
		sides.clear();
		
        // Iterate over src's faces and find 'sides' of the shared edge which is being collapsed
		PMVertex::FaceList::iterator sidesEnd = sides.end();
		for(PMVertex::FaceList::iterator it = srcfacebegin; it != srcfaceend; ++it)
		{
			// Check if this tri also has dest in it (shared edge)
			PMTriangle* srcface = *it;
			if(srcface->hasCommonVertex(dest) && sidesEnd == std::find(sides.begin(), sidesEnd, srcface))
			{
				sides.push_back(srcface);
				sidesEnd = sides.end();
			}
		}

		// Special cases
		// If we're looking at a border vertex
        if(PMVertex::BS_BORDER == src->mBorderStatus)
        {
			// Check for singular triangle destruction
			if (src->face.size() == 1 && dest->face.size() == 1)
			{
				// If src and dest both only have 1 triangle (and it must be a shared one)
				// then this would destroy the shape, so don't do this
				return NEVER_COLLAPSE_COST;
			}
			else if(collapseInvertsNormals(src, dest))
			{
				// border edge collapse must not invert normals of neighbor faces
				return NEVER_COLLAPSE_COST;
			}
			else if (sides.size() > 1) 
			{
				// src is on a border, but the src-dest edge has more than one tri on it
				// So it must be collapsing inwards
				// Mark as very high-value cost
				cost = 1.0f + sides.size() * sides.size() * edgeVector.squaredLength() * mInvSquaredBoundBoxDiagonal;
			}
			else
			{
				// Collapsing ALONG a border
				// We can't use curvature to measure the effect on the model
				// Instead, see what effect it has on 'pulling' the other border edges
				// The more colinear, the less effect it will have
				// So measure the delta triangle area
				// Normally there can be at most 1 other border edge attached to this
				// However in weird cases there may be more, so find the worst
				Vector3 otherBorderEdge;
				Real kinkiness, maxKinkiness; // holds squared parallelogram area
				maxKinkiness = 0.0f;
				
				for (PMVertex::NeighborList::iterator n = src->neighbor.begin(), nend = src->neighbor.end(); n != nend; ++n)
				{
					PMVertex* third = *n;
					if (third != dest && src->isManifoldEdgeWith(third))
					{
						otherBorderEdge = src->position - third->position;
						kinkiness = otherBorderEdge.crossProduct(edgeVector).squaredLength();	// doubled triangle area
						maxKinkiness = std::max(kinkiness, maxKinkiness);
					}
				}

				cost = 0.5f * Math::Sqrt(maxKinkiness) * mInvSquaredBoundBoxDiagonal; // cost is equal to normalized triangle area
			}
        } 
		else // not a border
		{
			if(collapseInvertsNormals(src, dest))
			{
				// border edge collapse must not invert normals of neighbor faces
				return NEVER_COLLAPSE_COST;
			}

			// each neighbour face sweep some tetrahedron duaring collapsing, we will try to minimize sum of their side areas
			// benefit of this metric is that it`s zero on any ridge, if collapsing along it
			// V = H * S / 3
			Real tripleVolumeSum = 0.0;
			Real areaSum = 0.0;
			for(PMVertex::FaceList::iterator srcface = srcfacebegin; srcface != srcfaceend; ++srcface)
			{
				PMTriangle* t = *srcface;
				tripleVolumeSum += Math::Abs(t->normal.dotProduct(edgeVector)) * t->area;
				areaSum += t->area;
			}
			
			// visible error is tetrahedron side area, so taking A = sqrt(S) we use such approximation
			// E = H * sqrt(S) / 2			(side triangle area)
			// and since H = 3 * V / S		(pyramid volume)
			// E = 3 * V / (2 * sqrt(S))
			cost = areaSum > 1e-06 ? tripleVolumeSum / (2.0 * Math::Sqrt(areaSum)) * mInvSquaredBoundBoxDiagonal : 0.0;
		}
		
		// take into consideration texture coordinates and normal direction
		if(mVertexComponentFlags & (1 << VES_NORMAL))
		{
			float uvCost = 1.0f, normalCost = 1.0f;
			
			for(PMVertex::NeighborList::iterator ni = src->neighbor.begin(), nend = src->neighbor.end(); ni != nend; ++ni)
			{
				PMVertex* v0 = *ni;
				PMVertex::NeighborList::iterator inext = ni + 1;
				PMVertex* v1 = (inext != nend) ? *inext : src->neighbor.front(); // next or first
				
				Vector3 en(v0->position - dest->position);
				Vector3 en1(v1->position - dest->position);
				if(en == Vector3::ZERO || en1 == Vector3::ZERO || v0 == v1)
					continue; //skip collapser
				
				// get first plane
				Plane p(src->normal.crossProduct(en), dest->position);
				
				// get second plane
				Plane p1(src->normal.crossProduct(en1), dest->position);
				
				Real pDistance = p.getDistance(src->position);
				Real p1Distance = p1.getDistance(src->position);
				
				if((pDistance > 0.0f && p1Distance > 0.0f) || (pDistance < 0.0f && p1Distance < 0.0f))
					continue; //collapser NOT between two planes
				
				//calculate barycentric coordinates (b1, b2, b3)
				
				Real& h1 = pDistance;
				Real H1 = p.getDistance(v1->position);
				Real b1 = h1 / H1;
				
				Real& h2 = p1Distance;
				Real H2 = p1.getDistance(v0->position);
				Real b2 = h2 / H2;
				
				Real b3 = 1.0f - b1 - b2;
				
				if(b1 < 0.0f || b2 < 0.0f || b3 < 0.0f) continue;
				
				//three points: dest, *i, v1
				if(mVertexComponentFlags & (1 << VES_TEXTURE_COORDINATES))
				{
					Vector2 newUV = v1->uv * b1 + v0->uv * b2 + dest->uv * b3;
					uvCost = std::max(Real(1.0f), (newUV - src->uv).squaredLength());
				}

				Vector3 newNormal = (b1 * v1->normal + b2 * v0->normal + b3 * dest->normal).normalisedCopy();					
				normalCost = std::max(Real(0.0f), 0.5f - 0.5f * newNormal.dotProduct(src->normal));

				assert(cost >= 0.0f && uvCost >= 0.0f && normalCost >= 0.0f);
				break;
			}

#ifdef IGNORE_UV_AND_NORMAL_COSTS			
			uvCost = normalCost = 0.0f;
#endif			
			
			const static float posWeight = 10.0f;
			const static float normWeight = 1.0f;
			const static float texWeight = 1.0f;
			const static float invSumWeight = 1.0f / (posWeight + normWeight + texWeight);
			
			
			cost = invSumWeight * (posWeight * cost + texWeight * uvCost + normWeight * normalCost);
			
		}	
		
		assert (cost >= 0);
		
		return cost;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::initialiseEdgeCollapseCosts(void)
    {
        for(WorkingDataList::iterator i = mWorkingData.begin(), iend = mWorkingData.end(); i != iend; ++i)
        {
			PMVertex* vFront = &i->mVertList.front();
			for(WorstCostList::iterator it = mWorstCosts.begin(), end_it = mWorstCosts.end(); it != end_it; ++it)
            {
				PMVertex* v = vFront + it->second;
                v->collapseTo = NULL;
                v->collapseCost = NEVER_COLLAPSE_COST;
            }
        }
    }
    //---------------------------------------------------------------------
    Real ProgressiveMesh::computeEdgeCostAtVertexForBuffer(PMVertex* v)
    {
        // compute the edge collapse cost for all edges that start
        // from vertex v.  Since we are only interested in reducing
        // the object by selecting the min cost edge at each step, we
        // only cache the cost of the least cost edge at this vertex
        // (in member variable collapse) as well as the value of the 
        // cost (in member variable objdist).
		
        if(v->neighbor.empty()) {
            // v doesn't have neighbors so nothing to collapse
            v->notifyRemoved();
            return v->collapseCost;
        }

        // Init metrics
		Real collapseCost = NEVER_COLLAPSE_COST;
		PMVertex* collapseTo = NULL;
		
        // search all neighboring edges for "least cost" edge
        for(PMVertex **n = &v->neighbor.front(), **nend = n + v->neighbor.size(); n != nend; ++n)
        {
			PMVertex* candidate = *n;
            Real cost = computeEdgeCollapseCost(v, candidate);
            if( (!collapseTo) || cost < collapseCost) 
            {
                collapseTo = candidate;  // candidate for edge collapse
                collapseCost = cost;     // cost of the collapse
            }
        }

        v->collapseCost = collapseCost;
        v->collapseTo = collapseTo;
        return collapseCost;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::computeAllCosts(void)
    {
        initialiseEdgeCollapseCosts();

		assert(!mWorstCosts.empty());
		for(WorstCostList::iterator it = mWorstCosts.begin(), end_it = mWorstCosts.end(); it != end_it; ++it)
			it->first = computeEdgeCostAtVertex(it->second);
		
		sortIndexesByCost();
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::collapse(ProgressiveMesh::PMVertex *src)
    {
        PMVertex *dest = src->collapseTo;
			
		if(PMVertex::BS_BORDER == src->mBorderStatus)
			dest->mBorderStatus = PMVertex::BS_BORDER;
			
		// Abort if we're never supposed to collapse
		if (src->collapseCost == NEVER_COLLAPSE_COST) 
			return;

		// Remove this vertex from the running for the next check
		src->collapseTo = NULL;
		src->collapseCost = NEVER_COLLAPSE_COST;

		// Collapse the edge uv by moving vertex u onto v
	    // Actually remove tris on uv, then update tris that
	    // have u to have v, and then remove u.
	    if(!dest) {
		    // src is a vertex all by itself 
#if LOG_PROGRESSIVE_MESH_GENERATION 
			ofdebug << "Aborting collapse, orphan vertex. " << std::endl;
#endif
			return;
	    }

		// Add dest and all the neighbours of source and dest to recompute list
		typedef vector<PMVertex*>::type RecomputeSet;
		RecomputeSet recomputeSet;
		recomputeSet.reserve(1 + src->neighbor.size() + dest->neighbor.size());
		recomputeSet.push_back(dest);
		recomputeSet.insert(recomputeSet.end(), src->neighbor.begin(), src->neighbor.end());
		recomputeSet.insert(recomputeSet.end(), dest->neighbor.begin(), dest->neighbor.end());
	
	    // delete triangles on edge src-dest
        // Notify others to replace src with dest
		// Queue of faces for removal / replacement
		// prevents us screwing up the iterators while we parse
		PMVertex::FaceList faceRemovalList, faceReplacementList;
		
		for(PMVertex::FaceList::iterator f = src->face.begin(), fend = src->face.end(); f != fend; ++f)
		{			
			if((*f)->hasCommonVertex(dest)) 
			{				
				// Tri is on src-dest therefore is gone
				if(faceRemovalList.end() == std::find(faceRemovalList.begin(), faceRemovalList.end(), *f))
					faceRemovalList.push_back(*f);
				// Reduce index count by 3 (useful for quick allocation later)

				mCurrNumIndexes -= 3;
			}
			else
			{
				// Only src involved, replace with dest
				if(faceReplacementList.end() == std::find(faceReplacementList.begin(), faceReplacementList.end(), *f))
					faceReplacementList.push_back(*f);
			}
		}

		src->toBeRemoved = true;
		// Replace all the faces queued for replacement
		
		for(PMVertex::FaceList::iterator f = faceReplacementList.begin(), fend = faceReplacementList.end(); f != fend; ++f)
		{
			/* Locate the face vertex which corresponds with the common 'dest' vertex
			To to this, find a removed face which has the FACE vertex corresponding with
			src, and use it's FACE vertex version of dest.
			*/
			PMFaceVertex* srcFaceVert = (*f)->getFaceVertexFromCommon(src);
			PMFaceVertex* destFaceVert = NULL;
			
			PMVertex::FaceList::iterator frlend = faceRemovalList.end();
			
			for(PMVertex::FaceList::iterator iremoved = faceRemovalList.begin(); iremoved != frlend; ++iremoved)
			{
				//if ( (*iremoved)->hasFaceVertex(srcFaceVert) )
				//{
					destFaceVert = (*iremoved)->getFaceVertexFromCommon(dest); 
				//}
			}
			
			assert(destFaceVert);

#if LOG_PROGRESSIVE_MESH_GENERATION 
			ofdebug << "Replacing vertex on face " << (unsigned int)(*f)->index << std::endl;
#endif
			(*f)->replaceVertex(srcFaceVert, destFaceVert);
		}

		// Remove all the faces queued for removal	
		for(PMVertex::FaceList::iterator f = faceRemovalList.begin(), fend = faceRemovalList.end(); f != fend; ++f)
		{
#if LOG_PROGRESSIVE_MESH_GENERATION 
			ofdebug << "Removing face " << (unsigned int)(*f)->index << std::endl;
#endif
			(*f)->notifyRemoved();
		}

        // Notify the vertex that it is gone
        src->notifyRemoved();
				
        // invalidate neighbour`s costs
		for(RecomputeSet::iterator it = recomputeSet.begin(), it_end = recomputeSet.end(); it != it_end; ++it)
		{
			PMVertex* v = *it;
			if(!mInvalidCostMask.getBit(v->index))
			{
				++mInvalidCostCount;
				mInvalidCostMask.setBit(v->index);
			}
		}
    }
    //---------------------------------------------------------------------
    Real ProgressiveMesh::computeEdgeCostAtVertex(size_t vertIndex)
    {
		// Call computer for each buffer on this vertex
        Real worstCost = -0.01f;
        WorkingDataList::iterator i, iend;
        iend = mWorkingData.end();
        for (i = mWorkingData.begin(); i != iend; ++i)
        {
            worstCost = std::max(worstCost, 
                computeEdgeCostAtVertexForBuffer(&(*i).mVertList[vertIndex]));
        }

		// Return the worst cost
		return worstCost;
    }
    //---------------------------------------------------------------------
	size_t ProgressiveMesh::getInvalidCostCount(ProgressiveMesh::ProgressiveMeshList& pmList)
	{	
		size_t invalidCostCount = 0;
		
		for(ProgressiveMeshList::iterator pmItr = pmList.begin(); pmItr != pmList.end(); ++pmItr)
			invalidCostCount += (*pmItr)->mInvalidCostCount;
		
		return invalidCostCount;
	}
    //---------------------------------------------------------------------
	bool ProgressiveMesh::recomputeInvalidCosts(ProgressiveMeshList& pmList)
	{
		bool isRecomputed = false;
		for(ProgressiveMeshList::iterator n = pmList.begin(); n != pmList.end(); ++n)
		{
			if((*n)->mInvalidCostCount != 0)
			{
				(*n)->recomputeInvalidCosts();
				isRecomputed = true;
			}
		}
		return isRecomputed;
	}
    //---------------------------------------------------------------------
	void ProgressiveMesh::recomputeInvalidCosts()
	{
		if(mInvalidCostCount > 0)
		{
			for(WorstCostList::iterator it = mWorstCosts.begin(), end_it = mWorstCosts.end(); it != end_it; ++it)
			{
				unsigned idx = it->second;
				if(mInvalidCostMask.getBit(idx))
					it->first = computeEdgeCostAtVertex(idx);
			}
			sortIndexesByCost();
			mInvalidCostMask.clearAllBits();
			mInvalidCostCount = 0;
		}
		mNextWorstCostHint = 0;
	}
    //---------------------------------------------------------------------
	int ProgressiveMesh::cmpByCost(const void* p1, const void* p2)
	{
		Real c1 = ((CostIndexPair*)p1)->first;
		Real c2 = ((CostIndexPair*)p2)->first;
		return (c1 < c2) ? -1 : c1 > c2;
	}
    //---------------------------------------------------------------------
	void ProgressiveMesh::sortIndexesByCost()
	{
		// half of total collapsing time is spended in this function for 700 000 triangles mesh, other
		// half in computeEdgeCostAtVertex. qsort is used instead of std::sort due to performance reasons
		qsort(&mWorstCosts.front(), mWorstCostsSize, sizeof(CostIndexPair), cmpByCost);
		
		// remove all vertices with NEVER_COLLAPSE_COST to reduce active vertex set
		while(mWorstCostsSize > 0 && mWorstCosts.back().first == NEVER_COLLAPSE_COST)
		{
			mWorstCosts.pop_back();
			--mWorstCostsSize;
		}
	}
    //---------------------------------------------------------------------
    ProgressiveMesh::CostIndexPair* ProgressiveMesh::getNextCollapser()
    {
		// as array is sorted by cost and only partially invalidated  - return first valid cost, it would be the best
		for(CostIndexPair* ptr = &mWorstCosts.front() + mNextWorstCostHint; mNextWorstCostHint  < mWorstCostsSize; ++ptr, ++mNextWorstCostHint)
			if(!mInvalidCostMask.getBit(ptr->second))
				return ptr;
		
		// no valid costs
		static CostIndexPair dontCollapse(NEVER_COLLAPSE_COST, 0);
		return &dontCollapse;
    }
    //---------------------------------------------------------------------
	void ProgressiveMesh::getNextCollapser(ProgressiveMesh::ProgressiveMeshList& pmList, ProgressiveMesh*& pm, CostIndexPair*& bestCollapser)
	{	
		pm = NULL;
		bestCollapser = NULL;
		Real bestCost = NEVER_COLLAPSE_COST;
		
		for(ProgressiveMeshList::iterator pmItr = pmList.begin(); pmItr != pmList.end(); )
		{		
			ProgressiveMesh* pmCur = *pmItr;
			
			CostIndexPair* collapser = pmCur->getNextCollapser();
			Real cost = collapser->first;

			if(NEVER_COLLAPSE_COST == cost)
			{
				if(pmCur->mInvalidCostCount != 0)
				{
					++pmItr;
				}
				else
				{
					pmItr = pmList.erase(pmItr);
				}
			}
			else
			{			
				if(cost < bestCost)
				{
					bestCost = cost;
					bestCollapser = collapser;
					pm = pmCur;
				}
				
				++pmItr;				
			}
		}
	}		
    //---------------------------------------------------------------------	
    void ProgressiveMesh::bakeNewLOD(IndexData* pData)
    {
        assert(mCurrNumIndexes > 0 && "No triangles to bake!");
        // Zip through the tri list of any working data copy and bake
        pData->indexCount = mCurrNumIndexes;
		pData->indexStart = 0;
		// Base size of indexes on original 
		bool use32bitindexes = 
			(mIndexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);

		// Create index buffer, we don't need to read it back or modify it a lot
		pData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
			use32bitindexes? HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT,
			pData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		
		IndexBufferLockGuard lockGuard(
			pData->indexBuffer, 0, pData->indexBuffer->getSizeInBytes(), HardwareBuffer::HBL_DISCARD
				);

        TriangleList::iterator tri, triend;
        // Use the first working data buffer, they are all the same index-wise
        WorkingDataList::iterator pWork = mWorkingData.begin();
        triend = pWork->mTriList.end();

		if (use32bitindexes)
		{
			unsigned int* pInt = static_cast<unsigned int*>(lockGuard.pData);
			for (tri = pWork->mTriList.begin(); tri != triend; ++tri)
			{
				if (!tri->removed)
				{
					*pInt++ = static_cast<unsigned int>(tri->vertex[0]->realIndex);
					*pInt++ = static_cast<unsigned int>(tri->vertex[1]->realIndex);
					*pInt++ = static_cast<unsigned int>(tri->vertex[2]->realIndex);
				}
			}
		}
		else
		{
			unsigned short* pShort = static_cast<unsigned short*>(lockGuard.pData);
			for (tri = pWork->mTriList.begin(); tri != triend; ++tri)
			{
				if (!tri->removed)
				{
					*pShort++ = static_cast<unsigned short>(tri->vertex[0]->realIndex);
					*pShort++ = static_cast<unsigned short>(tri->vertex[1]->realIndex);
					*pShort++ = static_cast<unsigned short>(tri->vertex[2]->realIndex);
				}
			}			
		}
    }
	//---------------------------------------------------------------------
	void ProgressiveMesh::bakeLodUsage(Mesh* pMesh, LodStrategy *lodStrategy, const LodValueList& lodValues, bool skipFirstLodLevel /* = false */)
	{
		// Iterate over the lods and record usage
		LodValueList::const_iterator ivalue = lodValues.begin(), ivalueend = lodValues.end();
		if(skipFirstLodLevel && ivalue != ivalueend) 
			++ivalue;
		
		pMesh->_setLodInfo(1 + (ivalueend - ivalue), false);
		
		unsigned short lodLevel = 1;
		for (; ivalue != ivalueend; ++ivalue)
		{
			// Record usage
			MeshLodUsage lodUsage;
			lodUsage.userValue = *ivalue;
			lodUsage.value = 0;
			lodUsage.edgeData = 0;
			lodUsage.manualMesh.setNull();
			
			pMesh->_setLodUsage(lodLevel++, lodUsage);
		}

		pMesh->setLodStrategy(lodStrategy);
	}
	//---------------------------------------------------------------------
	void ProgressiveMesh::createSimplifiedVertexData(vector<IndexVertexPair>::type& usedVertices, VertexData* inVData, VertexData*& outVData, AxisAlignedBox& aabox)
	{				
		outVData = NULL;
		
		VertexDataVariantList vdInVariantList;
		
		VertexDataVariant* inVertexDataBuffer = vdInVariantList.create(inVData, VES_POSITION);
		assert(inVertexDataBuffer->isValid());
		if(!inVertexDataBuffer->isValid())
			return;
		VertexDataVariant* inNormalDataBuffer = vdInVariantList.create(inVData, VES_NORMAL);
		VertexDataVariant* inUvDataBuffer = vdInVariantList.create(inVData, VES_TEXTURE_COORDINATES);
		
		static const unsigned short source = 0;
		
		outVData = new VertexData();
		outVData->vertexStart = 0;
		outVData->vertexCount = usedVertices.size();	
		
		VertexDeclaration* vertexDeclaration = outVData->vertexDeclaration;
		
		size_t offset = 0;
		
		offset += vertexDeclaration->addElement(source, offset, VET_FLOAT3, VES_POSITION).getSize();		
		if(inNormalDataBuffer->isValid())
			offset += vertexDeclaration->addElement(source, offset, VET_FLOAT3, VES_NORMAL).getSize();
		if(inUvDataBuffer->isValid())
			offset += vertexDeclaration->addElement(source, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES).getSize();
		
		assert(0 != offset);
		
		HardwareVertexBufferSharedPtr outVbuffer =
		HardwareBufferManager::getSingleton().createVertexBuffer(
																 vertexDeclaration->getVertexSize(source), outVData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		
		VertexBufferLockGuard outVdataLock(outVbuffer, HardwareBuffer::HBL_DISCARD);
		float* p = (float*)outVdataLock.pData;
		
		assert(outVData->vertexCount == usedVertices.size());
		
		for (vector<IndexVertexPair>::type::iterator it = usedVertices.begin(); it != usedVertices.end(); ++it)
		{
			unsigned idx = it->first;
			
			Vector3 v3;
			Vector2 uv;
			
			inVertexDataBuffer->offsetToElement(idx);
			v3 = inVertexDataBuffer->getNextVector3();
			*p++ = v3.x;
			*p++ = v3.y;
			*p++ = v3.z;
			aabox.merge(v3);
			
#ifndef CHECK_CALCULATED_NORMALS		
			if(inNormalDataBuffer->isValid())
			{
				//store original normals
				inNormalDataBuffer->offsetToElement(idx);
				v3 = inNormalDataBuffer->getNextVector3();
				*p++ = v3.x;
				*p++ = v3.y;
				*p++ = v3.z;				
			}
			else
			{
				//store calculated normals
				PMVertex* v = it->second;
				*p++ = v->normal.x;
				*p++ = v->normal.y;
				*p++ = v->normal.z;				
			}
#else		
			assert(inNormalDataBuffer->isValid()); //test is possible only if normals is present in input data
			if(inNormalDataBuffer->isValid())
			{
				//store original normals
				inNormalDataBuffer->offsetToElement(idx);
				v3 = inNormalDataBuffer->getNextVector3();
			}				
			
			//store calculated normals
			PMVertex* v = it->second;
			assert(v3.dotProduct(v->normal.normalisedCopy()) > 0.5f);
			*p++ = v->normal.x;
			*p++ = v->normal.y;
			*p++ = v->normal.z;
#endif			
			if(inUvDataBuffer->isValid())
			{
				inUvDataBuffer->offsetToElement(idx);					
				uv = inUvDataBuffer->getNextVector2();
				*p++ = uv.x;
				*p++ = uv.y;
			}
		}
		
		outVData->vertexBufferBinding->setBinding(source, outVbuffer);		
	}
	//---------------------------------------------------------------------
	void ProgressiveMesh::createIndexMap(vector<IndexVertexPair>::type& usedVertices, unsigned allVertexCount, vector<unsigned>::type& indexMap)
	{
		// we re-index used vertices and store old-to-new mapping in index map
		indexMap.resize(allVertexCount);
		memset(&indexMap[0], 0, allVertexCount * sizeof(unsigned));
		
		size_t n = 0;
		for(vector<ProgressiveMesh::IndexVertexPair>::type::iterator it = usedVertices.begin(); it != usedVertices.end(); ++it)
			indexMap[it->first] = n++;
	}
	//---------------------------------------------------------------------
	void ProgressiveMesh::bakeSimplifiedMesh(Ogre::Mesh* outMesh, Ogre::Mesh* inMesh, ProgressiveMeshList& pmList, bool dropFirstLodLevel)
	{
		assert(inMesh && outMesh);
		
		AxisAlignedBox outAabox;
		
		vector<IndexVertexPair>::type inCommonIndexes;
		BitArray usedVertices, usedBySubMesh;
		if(inMesh->sharedVertexData)
		{
			usedVertices.resize(inMesh->sharedVertexData->vertexCount);
			usedBySubMesh.resize(inMesh->sharedVertexData->vertexCount);
		}
		
		for(size_t i = 0; i < inMesh->getNumSubMeshes(); ++i)
		{
			SubMesh* inSubMesh = inMesh->getSubMesh(i);
			if(!inSubMesh->useSharedVertices)
				continue;
			
			ProgressiveMesh* pm = pmList[i];
			if(NULL == pm->mSubMesh)
				continue; // dummy, skip it
			
			IndexDataVariantSharedPtr inSubMeshIndexDataVar(IndexDataVariant::create(dropFirstLodLevel ? pm->mLodFaceList.front() : inSubMesh->indexData));
			assert(!inSubMeshIndexDataVar.isNull());
			
			usedBySubMesh.clearAllBits();
			inSubMeshIndexDataVar->markUsedVertices(usedBySubMesh);
			
			for(unsigned idx = 0, end_idx = inMesh->sharedVertexData->vertexCount; idx < end_idx; ++idx)
				if(usedBySubMesh.getBit(idx) && !usedVertices.getBit(idx))
				{
					usedVertices.setBit(idx);
					inCommonIndexes.push_back(std::make_pair(idx, &pm->mWorkingData.front().mVertList.front() + idx));
				}
		}
		
		// note - we don`t preserve initial vertex order, but group vertices by submesh instead, improving locality
		vector<unsigned>::type sharedIndexMap; // between submeshes
		
		if(!inCommonIndexes.empty())
		{
			createSimplifiedVertexData(inCommonIndexes, inMesh->sharedVertexData, outMesh->sharedVertexData, outAabox);
			createIndexMap(inCommonIndexes, inMesh->sharedVertexData->vertexCount, sharedIndexMap);
		}
		
		for(size_t i = 0; i < inMesh->getNumSubMeshes(); ++i)
		{
			SubMesh* inSubMesh = inMesh->getSubMesh(i);
			
			SubMesh* outSubMesh = outMesh->createSubMesh();
			outSubMesh->setMaterialName(inSubMesh->getMaterialName());
			
			outSubMesh->useSharedVertices = inSubMesh->useSharedVertices;
			
			ProgressiveMesh* pm = pmList[i];
			if(NULL == pm->mSubMesh)
				continue; // dummy, skip it
			
			IndexDataVariantSharedPtr inSubMeshIndexDataVar(IndexDataVariant::create(dropFirstLodLevel ? pm->mLodFaceList.front() : inSubMesh->indexData));
			assert(!inSubMeshIndexDataVar.isNull());
			
			vector<unsigned>::type localIndexMap; // to submesh
			vector<unsigned>::type* indexMap = &sharedIndexMap;
			
			if(!outSubMesh->useSharedVertices)
			{
				usedVertices.resize(inSubMesh->vertexData->vertexCount);
				inSubMeshIndexDataVar->markUsedVertices(usedVertices);
				
				vector<IndexVertexPair>::type inSubMeshIndexes;
				for(unsigned idx = 0, end_idx = inSubMesh->vertexData->vertexCount; idx < end_idx; ++idx)
					if(usedVertices.getBit(idx))
						inSubMeshIndexes.push_back(std::make_pair(idx, &pm->mWorkingData.front().mVertList.front() + idx));				
				
				createSimplifiedVertexData(inSubMeshIndexes, inSubMesh->vertexData, outSubMesh->vertexData, outAabox);
				
				indexMap = &localIndexMap;
				createIndexMap(inSubMeshIndexes, inSubMesh->vertexData->vertexCount, localIndexMap);
			}

			if(!outSubMesh->useSharedVertices && !outSubMesh->vertexData)
            {
                OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
                            String("Submesh does not have any vertex data"),
                            "ProgressiveMesh::bakeSimplifiedMesh");
            }

			bool outUse16bitIndexes = (outSubMesh->useSharedVertices ? outMesh->sharedVertexData : outSubMesh->vertexData)->vertexCount <= 0xFFFF;
			
			inSubMeshIndexDataVar->createIndexData(outSubMesh->indexData, outUse16bitIndexes, indexMap);
			inSubMeshIndexDataVar.setNull();
			
			// clone all LODs, may be without first
			vector<IndexData*>::type::iterator n = pm->mLodFaceList.begin();
			if(dropFirstLodLevel && n != pm->mLodFaceList.end()) 
				++n;
			
			for(; n != pm->mLodFaceList.end(); ++n)
			{
				IndexDataVariantSharedPtr lodIndexDataVar(IndexDataVariant::create(*n, HardwareBuffer::HBL_DISCARD));
				
				IndexData* lod = OGRE_NEW IndexData;
				lodIndexDataVar->createIndexData(lod, outUse16bitIndexes, indexMap);
				outSubMesh->mLodFaceList.push_back(lod);
			}
		}
		
		// Store bbox and sphere
		outMesh->_setBounds(outAabox, false);
		outMesh->_setBoundingSphereRadius((outAabox.getMaximum() - outAabox.getMinimum()).length() / 2.0f);
	}
    //---------------------------------------------------------------------
    ProgressiveMesh::PMTriangle::PMTriangle() : removed(false)
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::setDetails(size_t newindex, 
		ProgressiveMesh::PMFaceVertex *v0, ProgressiveMesh::PMFaceVertex *v1, 
        ProgressiveMesh::PMFaceVertex *v2)
    {
        assert(v0!=v1 && v1!=v2 && v2!=v0);

		removed = false;
		
        index = newindex;
		vertex[0]=v0;
        vertex[1]=v1;
        vertex[2]=v2;

        computeNormal();

        // Add tri to vertices
        // Also tell vertices they are neighbours
        for(int i=0;i<3;i++)
		{
			PMVertex* vv = vertex[i]->commonVertex;
		
			if(vv->face.end() == std::find(vv->face.begin(), vv->face.end(), this))
				vv->face.push_back(this);
            
			for(int j=0;j<3;j++) if(i!=j) {
				if(vv->neighbor.end() == std::find(vv->neighbor.begin(), vv->neighbor.end(), vertex[j]->commonVertex))
					vv->neighbor.push_back(vertex[j]->commonVertex);
            }
        }
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::notifyRemoved(void)
    {
        int i;
        for(i=0; i<3; i++) {
            // remove this tri from the vertices
            if(vertex[i])
				vertex[i]->commonVertex->face.erase(
							std::remove(vertex[i]->commonVertex->face.begin(), vertex[i]->commonVertex->face.end(), this),
							vertex[i]->commonVertex->face.end()
													);
        }
        for(i=0; i<3; i++) {
            int i2 = (i+1)%3;
            if(!vertex[i] || !vertex[i2]) continue;
            // Check remaining vertices and remove if not neighbours anymore
            // NB May remain neighbours if other tris link them
            vertex[i ]->commonVertex->removeIfNonNeighbor(vertex[i2]->commonVertex);
            vertex[i2]->commonVertex->removeIfNonNeighbor(vertex[i ]->commonVertex);
        }

        removed = true;
    }
    //---------------------------------------------------------------------
    bool ProgressiveMesh::PMTriangle::hasCommonVertex(ProgressiveMesh::PMVertex *v) const
    {
        return (v == vertex[0]->commonVertex ||
			v == vertex[1]->commonVertex || 
			v == vertex[2]->commonVertex);
    }
    //---------------------------------------------------------------------
	bool ProgressiveMesh::PMTriangle::hasFaceVertex(ProgressiveMesh::PMFaceVertex *v) const
	{
		return (v == vertex[0] ||
				v == vertex[1] || 
				v == vertex[2]);
	}
    //---------------------------------------------------------------------
	ProgressiveMesh::PMFaceVertex* 
	ProgressiveMesh::PMTriangle::getFaceVertexFromCommon(ProgressiveMesh::PMVertex* commonVert)
	{
		if (vertex[0]->commonVertex == commonVert) return vertex[0];
		if (vertex[1]->commonVertex == commonVert) return vertex[1];
		if (vertex[2]->commonVertex == commonVert) return vertex[2];

		return NULL;

	}
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::computeNormal()
    {
        Vector3 v0=vertex[0]->commonVertex->position;
        Vector3 v1=vertex[1]->commonVertex->position;
        Vector3 v2=vertex[2]->commonVertex->position;
        // Cross-product 2 edges
        Vector3 e1 = v1 - v0; 
        Vector3 e2 = v2 - v1;

        normal = e1.crossProduct(e2);
        area = normal.normalise();
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::replaceVertex(
		ProgressiveMesh::PMFaceVertex *vold, ProgressiveMesh::PMFaceVertex *vnew) 
    {
        assert(vold && vnew);
        assert(vold==vertex[0] || vold==vertex[1] || vold==vertex[2]);
        assert(vnew!=vertex[0] && vnew!=vertex[1] && vnew!=vertex[2]);
        if(vold==vertex[0]){
            vertex[0]=vnew;
        }
        else if(vold==vertex[1]){
            vertex[1]=vnew;
        }
        else {
            assert(vold==vertex[2]);
            vertex[2]=vnew;
        }
        int i;
        vold->commonVertex->face.erase(std::remove(vold->commonVertex->face.begin(), vold->commonVertex->face.end(), this), vold->commonVertex->face.end());
		if(vnew->commonVertex->face.end() == std::find(vnew->commonVertex->face.begin(), vnew->commonVertex->face.end(), this))
			vnew->commonVertex->face.push_back(this);
        for(i=0;i<3;i++) {
            vold->commonVertex->removeIfNonNeighbor(vertex[i]->commonVertex);
            vertex[i]->commonVertex->removeIfNonNeighbor(vold->commonVertex);
        }
        for(i=0;i<3;i++) {
            assert(std::find(vertex[i]->commonVertex->face.begin(), vertex[i]->commonVertex->face.end(), this) != vertex[i]->commonVertex->face.end());
            for(int j=0;j<3;j++) if(i!=j) {
#if LOG_PROGRESSIVE_MESH_GENERATION 
				ofdebug << "Adding vertex " << (unsigned int)vertex[j]->commonVertex->index << " to the neighbor list "
					"of vertex " << (unsigned int)vertex[i]->commonVertex->index << std::endl;
#endif 
				if(vertex[i]->commonVertex->neighbor.end() ==
				   std::find(vertex[i]->commonVertex->neighbor.begin(), vertex[i]->commonVertex->neighbor.end(), vertex[j]->commonVertex))
					vertex[i]->commonVertex->neighbor.push_back(vertex[j]->commonVertex);
            }
        }
        computeNormal();
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::setDetails(size_t newindex, const Vector3& pos, const Vector3& n, const Vector2& texUV)
    {
        position = pos;
		normal = n;
		uv = texUV;
        index = newindex;
		
		removed = false;
		toBeRemoved = false;
    }
    //---------------------------------------------------------------------
	void ProgressiveMesh::PMVertex::calculateNormal()
	{		
		Vector3 weightedSum(Vector3::ZERO);	// sum of neighbour face normals weighted with neighbour area
		
		for(PMVertex::FaceList::iterator i = face.begin(), iend = face.end(); i != iend; ++i)
		{
			PMTriangle* t = *i;
			weightedSum += t->normal * t->area;	// accumulate neighbour face normals weighted with neighbour area
		}
		
		// there is small possibility, that weigtedSum is very close to zero, use arbitrary normal value in such case
		normal = weightedSum.normalise() > 1e-08 ? weightedSum : Vector3::UNIT_Z;
	}
    //---------------------------------------------------------------------
	bool ProgressiveMesh::PMVertex::isNearEnough(PMVertex* other) const
	{
		return	position == other->position && normal.dotProduct(other->normal) > 0.8f && (uv - other->uv).squaredLength() < 0.01f;
	}	
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::notifyRemoved(void)
    {
        // Removes possible self listing as neighbor first
        neighbor.erase(std::remove(neighbor.begin(), neighbor.end(), this), neighbor.end());
        PMVertex::NeighborList::iterator nend = neighbor.end();
        for (PMVertex::NeighborList::iterator n = neighbor.begin(); n != nend; ++n)
        {
            // Remove me from neighbor
            (*n)->neighbor.erase(std::remove((*n)->neighbor.begin(), (*n)->neighbor.end(), this), (*n)->neighbor.end());
        }
        removed = true;
        collapseTo = NULL;
        collapseCost = NEVER_COLLAPSE_COST;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::initBorderStatus() 
    {
		assert(mBorderStatus == BS_UNKNOWN);
		
        // Look for edges which only have one tri attached, this is a border
		
		NeighborList::iterator nend = neighbor.end();
		
		// Loop for each neighbor
		for(NeighborList::iterator n = neighbor.begin(); n != nend; ++n) 
		{
			// Count of tris shared between the edge between this and neighbor
			ushort count = 0;
			// Loop over each face, looking for shared ones
			FaceList::iterator fend = face.end();
			for(FaceList::iterator j = face.begin(); j != fend; ++j) 
			{
				if((*j)->hasCommonVertex(*n))
				{
					// Shared tri
					++count;
				}
			}
			//assert(count>0); // Must be at least one!
			// This edge has only 1 tri on it, it's a border
			if(count == 1)
			{
				mBorderStatus = BS_BORDER;
				return;
			}
		}
		
		mBorderStatus = BS_NOT_BORDER;
    }
	//---------------------------------------------------------------------
	bool ProgressiveMesh::PMVertex::isManifoldEdgeWith(ProgressiveMesh::PMVertex* v)
	{
		// Check the sides involving both these verts
		// If there is only 1 this is a manifold edge
		ushort sidesCount = 0;

		FaceList::iterator fend = face.end();
		for (FaceList::iterator i = face.begin(); i != fend; ++i)
		{
			if ((*i)->hasCommonVertex(v))
			{
				sidesCount++;
			}
		}

		return (sidesCount == 1);
	}
	//---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::removeIfNonNeighbor(ProgressiveMesh::PMVertex *n) 
    {
        // removes n from neighbor list if n isn't a neighbor.
        NeighborList::iterator i = std::find(neighbor.begin(), neighbor.end(), n);
        if (i == neighbor.end())
            return; // Not in neighbor list anyway

        FaceList::iterator f, fend;
        fend = face.end();
        for(f = face.begin(); f != fend; ++f) 
        {
            if((*f)->hasCommonVertex(n)) return; // Still a neighbor
        }

#if LOG_PROGRESSIVE_MESH_GENERATION 
		ofdebug << "Vertex " << (unsigned int)n->index << " is no longer a neighbour of vertex " << (unsigned int)this->index <<
			" so has been removed from the latter's neighbor list." << std::endl;
#endif

        neighbor.erase(std::remove(neighbor.begin(), neighbor.end(), n), neighbor.end());

		if (neighbor.empty() && !toBeRemoved)
		{
			// This vertex has been removed through isolation (collapsing around it)
			this->notifyRemoved();
		}
    }
    //---------------------------------------------------------------------
	void ProgressiveMesh::dumpContents(const String& log)
	{
		std::ofstream ofdump(log.c_str());

		// Just dump 1st working data for now
		WorkingDataList::iterator worki = mWorkingData.begin();

		CommonVertexList::iterator vi, vend;
		vend = worki->mVertList.end();
		ofdump << "-------== VERTEX LIST ==-----------------" << std::endl;

		for(WorstCostList::iterator it = mWorstCosts.begin(), end_it = mWorstCosts.end(); it != end_it; ++it)
		{
			PMVertex* vert = &worki->mVertList[it->second];
			
			const char* isBorder = (vert->mBorderStatus == PMVertex::BS_BORDER) ? "yes" : "no";
			
			ofdump << "Vertex " << (unsigned int)vert->index << " pos: " << vert->position << " removed: " 
				<< vert->removed << " isborder: " << isBorder << std::endl;
			ofdump << "    Faces:" << std::endl;
			
			PMVertex::FaceList::iterator fend = vert->face.end();
			for(PMVertex::FaceList::iterator f = vert->face.begin(); f != fend; ++f)
			{
				ofdump << "    Triangle index " << (unsigned int)(*f)->index << std::endl;
			}
			
			ofdump << "    Neighbours:" << std::endl;			
			PMVertex::NeighborList::iterator nend = vert->neighbor.end();
			for (PMVertex::NeighborList::iterator n = vert->neighbor.begin(); n != nend; ++n)
			{
				ofdump << "    Vertex index " << (unsigned int)(*n)->index << std::endl;
			}
		}

		TriangleList::iterator ti, tend;
		tend = worki->mTriList.end();
		ofdump << "-------== TRIANGLE LIST ==-----------------" << std::endl;
		for(ti = worki->mTriList.begin(); ti != tend; ++ti)
		{
			ofdump << "Triangle " << (unsigned int)ti->index << " norm: " << ti->normal << " removed: " << ti->removed << std::endl;
			ofdump << "    Vertex 0: " << (unsigned int)ti->vertex[0]->realIndex << std::endl;
			ofdump << "    Vertex 1: " << (unsigned int)ti->vertex[1]->realIndex << std::endl;
			ofdump << "    Vertex 2: " << (unsigned int)ti->vertex[2]->realIndex << std::endl;
		}

		ofdump << "-------== COLLAPSE COST LIST ==-----------------" << std::endl;
		
		for(WorstCostList::iterator it = mWorstCosts.begin(), end_it = mWorstCosts.end(); it != end_it; ++it)
		{
			PMVertex* vert = &worki->mVertList[it->second];
			
			const char* isBorder = (vert->mBorderStatus == PMVertex::BS_BORDER) ? "yes" : "no";
			
			ofdump << "Vertex " << (unsigned int)vert->index << ", pos: " << vert->position << ", cost: " << vert->collapseCost << ", removed: " 
			<< vert->removed << ", isborder: " << isBorder << std::endl;
			
		}

		ofdump.close();
	}
}
