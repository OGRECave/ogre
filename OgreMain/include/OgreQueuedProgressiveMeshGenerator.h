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

#ifndef __QueuedProgressiveMeshGenerator_H_
#define __QueuedProgressiveMeshGenerator_H_

#include "OgreProgressiveMeshGenerator.h"
#include "OgreSingleton.h"
#include "OgreWorkQueue.h"
#include "OgreFrameListener.h"

namespace Ogre
{

/**
 * @brief Request data structure.
 */
struct PMGenRequest {
	struct VertexBuffer {
		size_t vertexCount;
		Vector3* vertexBuffer;
		VertexBuffer() :
			vertexBuffer(0) { }
	};
	struct IndexBuffer {
		size_t indexSize;
		size_t indexCount;
		unsigned char* indexBuffer; // size in bytes = indexSize * indexCount
		IndexBuffer() :
			indexBuffer(0) { }
	};
	struct SubmeshInfo {
		vector<IndexBuffer>::type genIndexBuffers; // order: lodlevel/generated index buffer
		IndexBuffer indexBuffer;
		VertexBuffer vertexBuffer;
		bool useSharedVertexBuffer;
	};
	vector<SubmeshInfo>::type submesh;
	VertexBuffer sharedVertexBuffer;
	LodConfig config;
	String meshName;
	~PMGenRequest();
};

/**
 * @brief Processes requests.
 */
class _OgreExport PMWorker :
	public Singleton<PMWorker>,
	private WorkQueue::RequestHandler,
	private ProgressiveMeshGenerator,
	public LodAlloc
{
public:
	PMWorker();
	virtual ~PMWorker();
	void addRequestToQueue(PMGenRequest* request);
	void clearPendingLodRequests();
	
	/** Override standard Singleton retrieval.
  @remarks
  Why do we do this? Well, it's because the Singleton
  implementation is in a .h file, which means it gets compiled
  into anybody who includes it. This is needed for the
  Singleton template to work, but we actually only want it
  compiled into the implementation of the class based on the
  Singleton, not all of them. If we don't change this, we get
  link errors when trying to use the Singleton-based class from
  an outside dll.
  @par
  This method just delegates to the template version anyway,
  but the implementation stays in this single compilation unit,
  preventing link errors.
  */
  static PMWorker& getSingleton(void);
  /** Override standard Singleton retrieval.
  @remarks
  Why do we do this? Well, it's because the Singleton
  implementation is in a .h file, which means it gets compiled
  into anybody who includes it. This is needed for the
  Singleton template to work, but we actually only want it
  compiled into the implementation of the class based on the
  Singleton, not all of them. If we don't change this, we get
  link errors when trying to use the Singleton-based class from
  an outside dll.
  @par
  This method just delegates to the template version anyway,
  but the implementation stays in this single compilation unit,
  preventing link errors.
  */
  static PMWorker* getSingletonPtr(void);
        
private:
	PMGenRequest* mRequest; // This is a copy of the current processed request from stack. This is needed to pass it to overloaded functions like bakeLods().
	ushort mChannelID;

	WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
	void buildRequest(LodConfig& lodConfigs);
	void tuneContainerSize();
	void initialize();
	void addVertexBuffer(const PMGenRequest::VertexBuffer& vertexBuffer, bool useSharedVertexLookup);
	void addIndexBuffer(PMGenRequest::IndexBuffer& indexBuffer, bool useSharedVertexLookup, unsigned short submeshID);
	void bakeLods();
};

class _OgreExport PMInjectorListener
{
public:
	PMInjectorListener(){}
	virtual ~PMInjectorListener(){}
	virtual bool shouldInject(PMGenRequest* request) = 0;
	virtual void injectionCompleted(PMGenRequest* request) = 0;
};

/**
 * @brief Injects the output of a request to the mesh in a thread safe way.
 */
class _OgreExport PMInjector :
	public Singleton<PMInjector>,
	public WorkQueue::ResponseHandler,
	public LodAlloc
{
public:
	PMInjector();
	virtual ~PMInjector();
	
	/** Override standard Singleton retrieval.
  @remarks
  Why do we do this? Well, it's because the Singleton
  implementation is in a .h file, which means it gets compiled
  into anybody who includes it. This is needed for the
  Singleton template to work, but we actually only want it
  compiled into the implementation of the class based on the
  Singleton, not all of them. If we don't change this, we get
  link errors when trying to use the Singleton-based class from
  an outside dll.
  @par
  This method just delegates to the template version anyway,
  but the implementation stays in this single compilation unit,
  preventing link errors.
  */
  static PMInjector& getSingleton(void);
  /** Override standard Singleton retrieval.
  @remarks
  Why do we do this? Well, it's because the Singleton
  implementation is in a .h file, which means it gets compiled
  into anybody who includes it. This is needed for the
  Singleton template to work, but we actually only want it
  compiled into the implementation of the class based on the
  Singleton, not all of them. If we don't change this, we get
  link errors when trying to use the Singleton-based class from
  an outside dll.
  @par
  This method just delegates to the template version anyway,
  but the implementation stays in this single compilation unit,
  preventing link errors.
  */
  static PMInjector* getSingletonPtr(void);

	void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);

	void setInjectorListener(PMInjectorListener* injectorListener) {mInjectorListener = injectorListener;}
	void removeInjectorListener() {mInjectorListener = 0;}
protected:

	// Copies every generated LOD level to the mesh.
	void inject(PMGenRequest* request);

	PMInjectorListener* mInjectorListener;
};

/**
 * @brief Creates a request for the worker. The interface is compatible with ProgressiveMeshGenerator.
 */
class _OgreExport QueuedProgressiveMeshGenerator :
	public ProgressiveMeshGeneratorBase
{
public:

	/// @copydoc ProgressiveMeshGeneratorBase::generateLodLevels
	void generateLodLevels(LodConfig& lodConfig);

	virtual ~QueuedProgressiveMeshGenerator();

private:
	void copyVertexBuffer(VertexData* data, PMGenRequest::VertexBuffer& out);
	void copyIndexBuffer(IndexData* data, PMGenRequest::IndexBuffer& out);
	void copyBuffers(Mesh* mesh, PMGenRequest* req);
};

}
#endif
