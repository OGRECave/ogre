/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreRoot.h"

#include "OgreVolumeChunkHandler.h"
#include "OgreVolumeChunk.h"
#include "OgreVolumeMeshBuilder.h"
#include "OgreVolumeOctreeNode.h"
#include "OgreVolumeDualGridGenerator.h"

namespace Ogre {
namespace Volume {

    const uint16 ChunkHandler::WORKQUEUE_LOAD_REQUEST = 1;
    
    //-----------------------------------------------------------------------
    
    void ChunkHandler::init(void)
    {
        if (!mWQ)
        {
            mWQ = Root::getSingleton().getWorkQueue();
            mWorkQueueChannel = mWQ->getChannel("Ogre/VolumeRendering");
            mWQ->addResponseHandler(mWorkQueueChannel, this);
            mWQ->addRequestHandler(mWorkQueueChannel, this);
        }
    }

    //-----------------------------------------------------------------------
    
    ChunkHandler::ChunkHandler(void) : mWQ(0), mWorkQueueChannel(0)
    {
    }

    //-----------------------------------------------------------------------
    
    ChunkHandler::~ChunkHandler(void)
    {
        // Root might already be shutdown.
        if (mWQ && Root::getSingletonPtr())
        {
            mWQ->removeRequestHandler(mWorkQueueChannel, this);
            mWQ->removeResponseHandler(mWorkQueueChannel, this);
        }
    }

    //-----------------------------------------------------------------------
  
    void ChunkHandler::addRequest(const ChunkRequest &req)
    {
        init();
        mWQ->addRequest(mWorkQueueChannel, WORKQUEUE_LOAD_REQUEST, req);
    }
    
    //-----------------------------------------------------------------------
  
    void ChunkHandler::processWorkQueue(void)
    {
        mWQ->processResponses();
    }

    //-----------------------------------------------------------------------
  
    WorkQueue::Response* ChunkHandler::handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
    {
        ChunkRequest cReq = any_cast<ChunkRequest>(req->getData());
        cReq.origin->prepareGeometry(cReq.level, cReq.root, cReq.dualGridGenerator, cReq.meshBuilder, cReq.totalFrom, cReq.totalTo);
        return OGRE_NEW WorkQueue::Response(req, true, Any());
    }
    
    //-----------------------------------------------------------------------

    void ChunkHandler::handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
    {
        if (res->succeeded())
        {
            ChunkRequest cReq = any_cast<ChunkRequest>(res->getRequest()->getData());
            cReq.origin->loadGeometry(cReq.meshBuilder, cReq.dualGridGenerator, cReq.root, cReq.level, cReq.isUpdate);
            OGRE_DELETE cReq.root;
            OGRE_DELETE cReq.dualGridGenerator;
            OGRE_DELETE cReq.meshBuilder;
        }
    }
}
}
