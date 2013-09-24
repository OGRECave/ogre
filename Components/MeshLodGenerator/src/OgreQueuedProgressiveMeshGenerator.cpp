/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2013 Torus Knot Software Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#include "OgreQueuedProgressiveMeshGenerator.h"

#include "OgreSubMesh.h"
#include "OgreHardwareBufferManager.h"
#include "OgreRoot.h"


namespace Ogre
{

//-----------------------------------------------------------------------
template<> PMWorker* Singleton<PMWorker>::msSingleton = 0;
PMWorker* PMWorker::getSingletonPtr(void)
{
    return msSingleton;
}
PMWorker& PMWorker::getSingleton(void)
{  
    assert( msSingleton );  return ( *msSingleton );  
}

//-----------------------------------------------------------------------
template<> PMInjector* Singleton<PMInjector>::msSingleton = 0;
PMInjector* PMInjector::getSingletonPtr(void)
{
    return msSingleton;
}
PMInjector& PMInjector::getSingleton(void)
{  
    assert( msSingleton );  return ( *msSingleton );  
}

PMGenRequest::~PMGenRequest()
{
	delete [] this->sharedVertexBuffer.vertexBuffer;
	delete [] this->sharedVertexBuffer.vertexNormalBuffer;
	int submeshCount = submesh.size(); 
	for(int i=0;i<submeshCount;i++){
		delete [] submesh[i].indexBuffer.indexBuffer;
		delete [] submesh[i].vertexBuffer.vertexBuffer;
		delete [] submesh[i].vertexBuffer.vertexNormalBuffer;
		vector<IndexBuffer>::iterator it2 = submesh[i].genIndexBuffers.begin();
		vector<IndexBuffer>::iterator it2End = submesh[i].genIndexBuffers.end();
		for (; it2 != it2End; it2++) {
			if(submesh[i].indexBuffer.indexBufferSize != 0 || i % 2 == 0){
				delete [] it2->indexBuffer;
			}
		}
	}
}


PMWorker::PMWorker() :
		mRequest(0)
{
	WorkQueue* wq = Root::getSingleton().getWorkQueue();
	mChannelID = wq->getChannel("PMGen");
	wq->addRequestHandler(mChannelID, this);
}


PMWorker::~PMWorker()
{
	Root* root = Root::getSingletonPtr();
	if (root) {
		WorkQueue* wq = root->getWorkQueue();
		if (wq) {
			wq->removeRequestHandler(mChannelID, this);
		}
	}
}

void PMWorker::addRequestToQueue( PMGenRequest* request )
{
	WorkQueue* wq = Root::getSingleton().getWorkQueue();
	wq->addRequest(mChannelID, 0, Any(request),0,false,true);
}

void PMWorker::clearPendingLodRequests()
{
	Ogre::WorkQueue* wq = Root::getSingleton().getWorkQueue();
	wq->abortPendingRequestsByChannel(mChannelID);
}

WorkQueue::Response* PMWorker::handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
{
	// Called on worker thread by WorkQueue.
	mRequest = any_cast<PMGenRequest*>(req->getData());
	buildRequest(mRequest->config);
	return OGRE_NEW WorkQueue::Response(req, true, req->getData());
}


void PMWorker::buildRequest(LodConfig& lodConfigs)
{
#if OGRE_DEBUG_MODE
	mMeshName = mRequest->meshName;
#endif
	mMeshBoundingSphereRadius = lodConfigs.mesh->getBoundingSphereRadius();
	mUseVertexNormals = lodConfigs.advanced.useVertexNormals;
	cleanupMemory();
	tuneContainerSize();
	initialize(lodConfigs); // Load vertices and triangles.
	computeCosts(); // Calculate all collapse costs.

#if OGRE_DEBUG_MODE
	assertValidMesh();
#endif

	computeLods(lodConfigs);
}

void PMWorker::tuneContainerSize()
{
	// Get Vertex count for container tuning.
	bool sharedVerticesAdded = false;
	size_t vertexCount = 0;
	size_t vertexLookupSize = 0;
	size_t sharedVertexLookupSize = 0;
	unsigned short submeshCount = mRequest->submesh.size();
	for (unsigned short i = 0; i < submeshCount; i++) {
		const PMGenRequest::SubmeshInfo& submesh = mRequest->submesh[i];
		if (!submesh.useSharedVertexBuffer) {
			size_t count = submesh.vertexBuffer.vertexCount;
			vertexLookupSize = std::max<size_t>(vertexLookupSize, count);
			vertexCount += count;
		} else if (!sharedVerticesAdded) {
			sharedVerticesAdded = true;
			sharedVertexLookupSize = mRequest->sharedVertexBuffer.vertexCount;
			vertexCount += sharedVertexLookupSize;
		}
	}

	// Tune containers:
	mUniqueVertexSet.rehash(4 * vertexCount); // less then 0.25 item/bucket for low collision rate

	// There are less triangles then 2 * vertexCount. Except if there are bunch of triangles,
	// where all vertices have the same position, but that would not make much sense.
	mTriangleList.reserve(2 * vertexCount);

	mVertexList.reserve(vertexCount);
	mSharedVertexLookup.reserve(sharedVertexLookupSize);
	mVertexLookup.reserve(vertexLookupSize);
	mIndexBufferInfoList.resize(submeshCount);
}

void PMWorker::initialize(LodConfig& lodConfig)
{
	unsigned short submeshCount = mRequest->submesh.size();
	for (unsigned short i = 0; i < submeshCount; ++i) {
		PMGenRequest::SubmeshInfo& submesh = mRequest->submesh[i];
		PMGenRequest::VertexBuffer& vertexBuffer =
		    (submesh.useSharedVertexBuffer ? mRequest->sharedVertexBuffer : submesh.vertexBuffer);
		addVertexBuffer(vertexBuffer, submesh.useSharedVertexBuffer);
		addIndexBuffer(submesh.indexBuffer, submesh.useSharedVertexBuffer, i);
	}

	injectProfile(lodConfig.advanced.profile);

	// These were only needed for addIndexData() and addVertexData().
	mSharedVertexLookup.clear();
	mVertexLookup.clear();
	mUniqueVertexSet.clear();
}

void PMWorker::addVertexBuffer(const PMGenRequest::VertexBuffer& vertexBuffer, bool useSharedVertexLookup)
{
	if (useSharedVertexLookup && !mSharedVertexLookup.empty()) {
		return; // We already loaded the shared vertex buffer.
	}

	VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;
	lookup.clear();

	Vector3* pNormalOut = vertexBuffer.vertexNormalBuffer;

	// Loop through all vertices and insert them to the Unordered Map.
	Vector3* pOut = vertexBuffer.vertexBuffer;
	Vector3* pEnd = vertexBuffer.vertexBuffer + vertexBuffer.vertexCount;
	for (; pOut < pEnd; pOut++) {
		mVertexList.push_back(PMVertex());
		PMVertex* v = &mVertexList.back();
		v->position = *pOut;
		std::pair<UniqueVertexSet::iterator, bool> ret;
		ret = mUniqueVertexSet.insert(v);
		if (!ret.second) {
			// Vertex position already exists.
			mVertexList.pop_back();
			v = *ret.first; // Point to the existing vertex.
			v->seam = true;
			if(mUseVertexNormals){
				if(v->normal.x != (*pNormalOut).x){
					v->normal += *pNormalOut;
					if(v->normal.isZeroLength()){
						v->normal = Vector3(1.0, 0.0, 0.0);
					}
					v->normal.normalise();
				}
				pNormalOut++;
			}
		} else {
#if OGRE_DEBUG_MODE
			v->costHeapPosition = mCollapseCostHeap.end();
#endif
			v->seam = false;
			v->hasProfile = false;
			if(mUseVertexNormals){
				v->normal = *pNormalOut;
				v->normal.normalise();
				pNormalOut++;
			}
		}
		lookup.push_back(v);
	}
}

void PMWorker::addIndexBuffer(PMGenRequest::IndexBuffer& indexBuffer, bool useSharedVertexLookup, unsigned short submeshID)
{
	size_t isize = indexBuffer.indexSize;
	mIndexBufferInfoList[submeshID].indexSize = isize;
	mIndexBufferInfoList[submeshID].indexCount = indexBuffer.indexCount;
	VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;

	// Lock the buffer for reading.
	unsigned char* iStart = indexBuffer.indexBuffer;
	if(!iStart) {
		return;
	}
	unsigned char* iEnd = iStart + indexBuffer.indexCount * isize;
	if (isize == sizeof(unsigned short)) {
		addIndexDataImpl<unsigned short>((unsigned short*) iStart, (unsigned short*) iEnd, lookup, submeshID);
	} else {
		// Unsupported index size.
		OgreAssert(isize == sizeof(unsigned int), "");
		addIndexDataImpl<unsigned int>((unsigned int*) iStart, (unsigned int*) iEnd, lookup, submeshID);
	}
}

void PMWorker::bakeDummyLods()
{
	// Used for manual Lod levels
	unsigned short submeshCount = mRequest->submesh.size();
	for (unsigned short i = 0; i < submeshCount; i++) {
		vector<PMGenRequest::IndexBuffer>::type& lods = mRequest->submesh[i].genIndexBuffers;
		lods.push_back(PMGenRequest::IndexBuffer());
		lods.back().indexStart = 0;
		lods.back().indexCount = 0;
		lods.back().indexSize = 2;
		lods.back().indexBuffer = NULL;
		lods.back().indexBufferSize = 0;
	}
}

void PMWorker::bakeLods()
{

	unsigned short submeshCount = mRequest->submesh.size();
	
	// Create buffers.
	for (unsigned short i = 0; i < submeshCount; i++) {
		vector<PMGenRequest::IndexBuffer>::type& lods = mRequest->submesh[i].genIndexBuffers;
		int indexCount = mIndexBufferInfoList[i].indexCount;
		OgreAssert(indexCount >= 0, "");

		lods.push_back(PMGenRequest::IndexBuffer());
		if (indexCount == 0) {
			//If the index is empty we need to create a "dummy" triangle, just to keep the index from beíng empty.
			//The main reason for this is that the OpenGL render system will crash with a segfault unless the index has some values.
			//This should hopefully be removed with future versions of Ogre. The most preferred solution would be to add the
			//ability for a submesh to be excluded from rendering for a given LOD (which isn't possible currently 2012-12-09).
			if ((!mRequest->submesh[i].useSharedVertexBuffer && mRequest->submesh[i].vertexBuffer.vertexCount == 0) ||
					(mRequest->submesh[i].useSharedVertexBuffer &&mRequest->sharedVertexBuffer.vertexCount == 0)) {
				//There's no vertex buffer and not much we can do.
				lods.back().indexCount = indexCount;
			} else {
				lods.back().indexCount = 3;
			}
		} else {
			lods.back().indexCount = indexCount;
		}
		lods.back().indexStart = 0;
		lods.back().indexSize = mRequest->submesh[i].indexBuffer.indexSize;
		lods.back().indexBufferSize = 0; // It means same as index count
		lods.back().indexBuffer = new unsigned char[lods.back().indexCount * lods.back().indexSize];
		if (mIndexBufferInfoList[i].indexSize == 2) {
			mIndexBufferInfoList[i].buf.pshort = (unsigned short*) lods.back().indexBuffer;
		} else {
			mIndexBufferInfoList[i].buf.pint = (unsigned int*) lods.back().indexBuffer;
		}

		if (indexCount == 0) {
			memset(mIndexBufferInfoList[i].buf.pshort, 0, 3 * mIndexBufferInfoList[i].indexSize);
		}
	}

	// Fill buffers.
	size_t triangleCount = mTriangleList.size();
	for (size_t i = 0; i < triangleCount; i++) {
		if (!mTriangleList[i].isRemoved) {
			if (mIndexBufferInfoList[mTriangleList[i].submeshID].indexSize == 2) {
				for (int m = 0; m < 3; m++) {
					*(mIndexBufferInfoList[mTriangleList[i].submeshID].buf.pshort++) =
					    static_cast<unsigned short>(mTriangleList[i].vertexID[m]);
				}
			} else {
				for (int m = 0; m < 3; m++) {
					*(mIndexBufferInfoList[mTriangleList[i].submeshID].buf.pint++) =
					    static_cast<unsigned int>(mTriangleList[i].vertexID[m]);
				}
			}
		}
	}
}

void PMWorker::bakeMergedLods(bool firstBufferPass)
{
	unsigned short submeshCount = mRequest->submesh.size();

	if(firstBufferPass){
		int indexCount = 0;
		for (unsigned short i = 0; i < submeshCount; i++) {
			mIndexBufferInfoList[i].prevIndexCount = mIndexBufferInfoList[i].indexCount;
			indexCount += mIndexBufferInfoList[i].indexCount;
			mIndexBufferInfoList[i].prevOnlyIndexCount = 0;
		}
		mTriangleCacheList.resize(0);
		mTriangleCacheList.reserve(indexCount);
		// TODO reset cache pointer
		size_t triangleCount = mTriangleList.size();
		for (size_t i = 0; i < triangleCount; i++) {
			mTriangleList[i].vertexChanged = false;
			if (!mTriangleList[i].isRemoved) {
				PMTriangleCache tri;
				tri.vertexID[0] = mTriangleList[i].vertexID[0];
				tri.vertexID[1] = mTriangleList[i].vertexID[1];
				tri.vertexID[2] = mTriangleList[i].vertexID[2];
				assert(mTriangleList.capacity() > mTriangleList.size()); // It should not reallocate!
				mTriangleCacheList.push_back(tri);
				mTriangleList[i].prevLod = &mTriangleCacheList.back();
			} else {
				mTriangleList[i].prevLod = NULL;
			}
		}
	} else {

		// Create buffers.
		for (unsigned short i = 0; i < submeshCount; i++) {
			vector<PMGenRequest::IndexBuffer>::type& lods = mRequest->submesh[i].genIndexBuffers;
			int indexCount = mIndexBufferInfoList[i].indexCount + mIndexBufferInfoList[i].prevOnlyIndexCount;
			assert(indexCount >= 0);

			lods.push_back(PMGenRequest::IndexBuffer());
			PMGenRequest::IndexBuffer& prevLod = lods.back();
			prevLod.indexStart = 0;
			prevLod.indexSize = mRequest->submesh[i].indexBuffer.indexSize;

			//If the index is empty we need to create a "dummy" triangle, just to keep the index buffer from being empty.
			//The main reason for this is that the OpenGL render system will crash with a segfault unless the index has some values.
			//This should hopefully be removed with future versions of Ogre. The most preferred solution would be to add the
			//ability for a submesh to be excluded from rendering for a given LOD (which isn't possible currently 2012-12-09).
			indexCount = std::max<size_t>(indexCount, 3);
			prevLod.indexCount = std::max<size_t>(mIndexBufferInfoList[i].prevIndexCount, 3u);
			prevLod.indexBufferSize = indexCount;
			prevLod.indexBuffer = new unsigned char[indexCount * mIndexBufferInfoList[i].indexSize];
			mIndexBufferInfoList[i].buf.pshort = (unsigned short*)prevLod.indexBuffer;

			//Check if we should fill it with a "dummy" triangle.
			if (indexCount == 3) {
				memset(mIndexBufferInfoList[i].buf.pshort, 0, 3 * mIndexBufferInfoList[i].indexSize);
			}

			lods.push_back(PMGenRequest::IndexBuffer());
			PMGenRequest::IndexBuffer& curLod = lods.back();
			curLod.indexSize = prevLod.indexSize;
			curLod.indexStart = indexCount - mIndexBufferInfoList[i].indexCount;
			curLod.indexCount = mIndexBufferInfoList[i].indexCount;
			curLod.indexBufferSize = prevLod.indexBufferSize;
			curLod.indexBuffer = prevLod.indexBuffer;
			if(curLod.indexCount == 0){
				curLod.indexStart-=3;
				curLod.indexCount=3;
			}

		}


		// Fill buffers.
		// Filling will be done in 3 parts.
		// 1. prevLod only indices.
		size_t triangleCount = mTriangleList.size();
		for (size_t i = 0; i < triangleCount; i++) {
			if (mTriangleList[i].vertexChanged) {
				assert(mIndexBufferInfoList[mTriangleList[i].submeshID].prevIndexCount != 0);
				if (mIndexBufferInfoList[mTriangleList[i].submeshID].indexSize == 2) {
					for (int m = 0; m < 3; m++) {
						*(mIndexBufferInfoList[mTriangleList[i].submeshID].buf.pshort++) =
							static_cast<unsigned short>(mTriangleList[i].prevLod->vertexID[m]);
					}
				} else {
					for (int m = 0; m < 3; m++) {
						*(mIndexBufferInfoList[mTriangleList[i].submeshID].buf.pint++) =
							static_cast<unsigned int>(mTriangleList[i].prevLod->vertexID[m]);
					}
				}
			}
		}


		// 2. shared indices.
		for (size_t i = 0; i < triangleCount; i++) {
			if (!mTriangleList[i].isRemoved && !mTriangleList[i].vertexChanged) {
				assert(mTriangleList[i].prevLod->vertexID[0] == mTriangleList[i].vertexID[0]);
				assert(mTriangleList[i].prevLod->vertexID[1] == mTriangleList[i].vertexID[1]);
				assert(mTriangleList[i].prevLod->vertexID[2] == mTriangleList[i].vertexID[2]);

				assert(mIndexBufferInfoList[mTriangleList[i].submeshID].indexCount != 0);
				assert(mIndexBufferInfoList[mTriangleList[i].submeshID].prevIndexCount != 0);
				if (mIndexBufferInfoList[mTriangleList[i].submeshID].indexSize == 2) {
					for (int m = 0; m < 3; m++) {
						*(mIndexBufferInfoList[mTriangleList[i].submeshID].buf.pshort++) =
							static_cast<unsigned short>(mTriangleList[i].vertexID[m]);
					}
				} else {
					for (int m = 0; m < 3; m++) {
						*(mIndexBufferInfoList[mTriangleList[i].submeshID].buf.pint++) =
							static_cast<unsigned int>(mTriangleList[i].vertexID[m]);
					}
				}
			}
		}

		// 3. curLod indices only.
		for (size_t i = 0; i < triangleCount; i++) {
			if (!mTriangleList[i].isRemoved && mTriangleList[i].vertexChanged) {
				assert(mIndexBufferInfoList[mTriangleList[i].submeshID].indexCount != 0);
				if (mIndexBufferInfoList[mTriangleList[i].submeshID].indexSize == 2) {
					for (int m = 0; m < 3; m++) {
						*(mIndexBufferInfoList[mTriangleList[i].submeshID].buf.pshort++) =
							static_cast<unsigned short>(mTriangleList[i].vertexID[m]);
					}
				} else {
					for (int m = 0; m < 3; m++) {
						*(mIndexBufferInfoList[mTriangleList[i].submeshID].buf.pint++) =
							static_cast<unsigned int>(mTriangleList[i].vertexID[m]);
					}
				}
			}
		}
	}
}

PMInjector::PMInjector() :
	mInjectorListener(0)
{
	WorkQueue* wq = Root::getSingleton().getWorkQueue();
	unsigned short workQueueChannel = wq->getChannel("PMGen");
	wq->addResponseHandler(workQueueChannel, this);
}

PMInjector::~PMInjector()
{
	Root* root = Root::getSingletonPtr();
	if (root) {
		WorkQueue* wq = root->getWorkQueue();
		if (wq) {
			unsigned short workQueueChannel = wq->getChannel("PMGen");
			wq->removeResponseHandler(workQueueChannel, this);
		}
	}
}

void PMInjector::handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
{
	PMGenRequest* request = any_cast<PMGenRequest*>(res->getData());
	if(!mInjectorListener){
		inject(request);
	} else {
		if(mInjectorListener->shouldInject(request)) {
			inject(request);
			mInjectorListener->injectionCompleted(request);
		}
	}
}

void PMInjector::inject(PMGenRequest* request)
{
	const MeshPtr& mesh = request->config.mesh;
	unsigned short submeshCount = request->submesh.size();
	OgreAssert(mesh->getNumSubMeshes() == submeshCount, "");
	mesh->removeLodLevels();
	for (unsigned short submeshID = 0; submeshID < submeshCount; submeshID++) {
		SubMesh::LODFaceList& lods = mesh->getSubMesh(submeshID)->mLodFaceList;
		typedef vector<PMGenRequest::IndexBuffer>::type GenBuffers;
		GenBuffers& buffers = request->submesh[submeshID].genIndexBuffers;

		int buffCount = buffers.size();
		for (int i=0; i<buffCount;i++) {
			PMGenRequest::IndexBuffer& buff = buffers[i];
			int indexCount = (buff.indexBufferSize ? buff.indexBufferSize : buff.indexCount);
			lods.push_back(OGRE_NEW IndexData());
			lods.back()->indexStart = buff.indexStart;
			lods.back()->indexCount = buff.indexCount;
			if(indexCount != 0) {
				if(i > 0 && buffers[i-1].indexBuffer == buff.indexBuffer){
					lods.back()->indexBuffer = (*(++lods.rbegin()))->indexBuffer;
				} else {
					lods.back()->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
						buff.indexSize == 2 ?
						HardwareIndexBuffer::IT_16BIT : HardwareIndexBuffer::IT_32BIT,
						indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
					int sizeInBytes = lods.back()->indexBuffer->getSizeInBytes();
					void* pOutBuff = lods.back()->indexBuffer->lock(0, sizeInBytes, HardwareBuffer::HBL_DISCARD);
					memcpy(pOutBuff, buff.indexBuffer, sizeInBytes);
					lods.back()->indexBuffer->unlock();
				}
			}
		}
	}
	mesh.get()->_configureMeshLodUsage(request->config);
}


void QueuedProgressiveMeshGenerator::generateLodLevels(LodConfig& lodConfig)
{
#if OGRE_DEBUG_MODE
	// Do not call this with empty Lod.
	OgreAssert(!lodConfig.levels.empty(), "");

	// Too many lod levels.
	OgreAssert(lodConfig.levels.size() <= 0xffff, "");

	// Lod distances needs to be sorted.
	Mesh::LodValueList values;
	for (size_t i = 0; i < lodConfig.levels.size(); i++) {
		values.push_back(lodConfig.levels[i].distance);
	}
	lodConfig.strategy->assertSorted(values);
#endif // if ifndef NDEBUG

	PMGenRequest* req = new PMGenRequest();
	req->meshName = lodConfig.mesh->getName();
	req->config = lodConfig;
	copyBuffers(lodConfig.mesh.get(), req);
	PMWorker::getSingleton().addRequestToQueue(req);
}

void QueuedProgressiveMeshGenerator::copyVertexBuffer(VertexData* data, PMGenRequest::VertexBuffer& out, LodConfig& config)
{
	// Locate position element and the buffer to go with it.
	const VertexElement* elemPos = data->vertexDeclaration->findElementBySemantic(VES_POSITION);

	// Only float supported.
	OgreAssert(elemPos->getSize() == 12, "");

	HardwareVertexBufferSharedPtr vbuf = data->vertexBufferBinding->getBuffer(elemPos->getSource());

	out.vertexCount = data->vertexCount;

	if (out.vertexCount > 0) {
		out.vertexBuffer = new Vector3[out.vertexCount];

		// Lock the buffer for reading.
		unsigned char* vStart = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
		unsigned char* vertex = vStart;
		int vSize = vbuf->getVertexSize();

		const VertexElement* elemNormal = 0;
		HardwareVertexBufferSharedPtr vNormalBuf;
		unsigned char* vNormal = NULL;
		Vector3* pNormalOut = NULL;
		int vNormalSize = 0;
		bool& useVertexNormals = config.advanced.useVertexNormals;

		if(useVertexNormals) {
			elemNormal = data->vertexDeclaration->findElementBySemantic(VES_NORMAL);
			useVertexNormals = (elemNormal != 0);
			if(useVertexNormals){
				out.vertexNormalBuffer = new Vector3[out.vertexCount];
				pNormalOut = out.vertexNormalBuffer;
				if(elemNormal->getSource() == elemPos->getSource()){
					vNormalBuf = vbuf;
					vNormal = vStart;
				}  else {
					vNormalBuf = data->vertexBufferBinding->getBuffer(elemNormal->getSource());
					assert(vNormalBuf->getSizeInBytes() == vbuf->getSizeInBytes());
					assert(vNormalBuf->getVertexSize() == vbuf->getVertexSize());
					vNormal = static_cast<unsigned char*>(vNormalBuf->lock(HardwareBuffer::HBL_READ_ONLY));
				}
				vNormalSize = vNormalBuf->getVertexSize();
			}
		}

		// Loop through all vertices and insert them to the Unordered Map.
		Vector3* pOut = out.vertexBuffer;
		Vector3* pEnd = out.vertexBuffer + out.vertexCount;
		for (; pOut < pEnd; pOut++) {
			float* pFloat;
			elemPos->baseVertexPointerToElement(vertex, &pFloat);
			pOut->x = *pFloat;
			pOut->y = *(++pFloat);
			pOut->z = *(++pFloat);
			vertex += vSize;
			if(elemNormal){
				elemNormal->baseVertexPointerToElement(vNormal, &pFloat);
				pNormalOut->x = *pFloat;
				pNormalOut->y = *(++pFloat);
				pNormalOut->z = *(++pFloat);
				pNormalOut++;
				vNormal += vNormalSize;
			}
		}
		vbuf->unlock();
		if(elemNormal && elemNormal->getSource() != elemPos->getSource()){
			vNormalBuf->unlock();
		}
	}
}

void QueuedProgressiveMeshGenerator::copyIndexBuffer(IndexData* data, PMGenRequest::IndexBuffer& out)
{
	const HardwareIndexBufferSharedPtr& indexBuffer = data->indexBuffer;
	out.indexSize = indexBuffer->getIndexSize();
	out.indexCount = data->indexCount;
	if (out.indexCount > 0) {
		unsigned char* pBuffer = (unsigned char*) indexBuffer->lock(HardwareBuffer::HBL_READ_ONLY);
		size_t offset = data->indexStart * out.indexSize;
		out.indexBuffer = new unsigned char[out.indexCount * out.indexSize];
		out.indexStart = 0;
		out.indexBufferSize = 0;
		memcpy(out.indexBuffer, pBuffer + offset, out.indexCount * out.indexSize);
		indexBuffer->unlock();
	}
}

void QueuedProgressiveMeshGenerator::copyBuffers(Mesh* mesh, PMGenRequest* req)
{
	// Get Vertex count for container tuning.
	bool sharedVerticesAdded = false;
	unsigned short submeshCount = mesh->getNumSubMeshes();
	req->submesh.resize(submeshCount);
	for (unsigned short i = 0; i < submeshCount; i++) {
		const SubMesh* submesh = mesh->getSubMesh(i);
		PMGenRequest::SubmeshInfo& outsubmesh = req->submesh[i];
		copyIndexBuffer(submesh->indexData, outsubmesh.indexBuffer);
		outsubmesh.useSharedVertexBuffer = submesh->useSharedVertices;
		if (!outsubmesh.useSharedVertexBuffer) {
			copyVertexBuffer(submesh->vertexData, outsubmesh.vertexBuffer, req->config);

		} else if (!sharedVerticesAdded) {
			sharedVerticesAdded = true;
			copyVertexBuffer(mesh->sharedVertexData, req->sharedVertexBuffer, req->config);
		}
	}
}

QueuedProgressiveMeshGenerator::~QueuedProgressiveMeshGenerator()
{
}

}
