/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#ifndef __Ogre_Volume_Chunk_Handler_H__
#define __Ogre_Volume_Chunk_Handler_H__

#include "OgreWorkQueue.h"

#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {
    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Volume
    *  @{
    */
    /** Forward declaration.
    */
    class Chunk;
    class MeshBuilder;
    class DualGridGenerator;
    class OctreeNode;

    /** Data being passed around while loading.
    */
    typedef struct ChunkRequest
    {

        /// The back lower left corner of the world.
        Vector3 totalFrom;

        /// The front upper rightcorner of the world.
        Vector3 totalTo;

        /// The current LOD level.
        size_t level;

        /// The maximum amount of levels.
        size_t maxLevels;

        /// The MeshBuilder to use.
        MeshBuilder *meshBuilder;

        /// The DualGridGenerator to use.
        DualGridGenerator *dualGridGenerator;

        /// The octree node to use.
        OctreeNode *root;

        /// The chunk which created this request.
        Chunk *origin;

        /// Whether this is an update of an existing tree
        bool isUpdate;

        OGRE_DEPRECATED _OgreVolumeExport friend std::ostream& operator<<(std::ostream& o, const ChunkRequest& r)
        { return o; }
    } ChunkRequest;
    
    /** Handles the WorkQueue management of the chunks.
    */
    class _OgreVolumeExport ChunkHandler : public WorkQueue::RequestHandler, public WorkQueue::ResponseHandler
    {
    protected:
        
        /// The workqueue load request.
        static const uint16 WORKQUEUE_LOAD_REQUEST;

        /// The workqueue.
        WorkQueue* mWQ;

        /// The workqueue channel.
        uint16 mWorkQueueChannel;
        
        /** Initializes the WorkQueue (once).
        */
        void init(void);

    public:
        
        /** Constructor
        */
        ChunkHandler(void);

        /** Destructor.
        */
        virtual ~ChunkHandler(void);
        
        /** Adds a new ChunkRequest to be loaded to the WorkQueue.
        @param req
            The ChunkRequest.
        */
        void addRequest(const ChunkRequest &req);

        /** Calls the process-update of the WorkQueue so it doesn't block.
        */
        void processWorkQueue(void);

        /// Implementation for WorkQueue::RequestHandler
        WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
        
        /// Implementation for WorkQueue::ResponseHandler
        void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);
        
    };
    /** @} */
    /** @} */
}
}

#endif
