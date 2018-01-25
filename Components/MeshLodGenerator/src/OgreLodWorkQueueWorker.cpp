/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2014 Torus Knot Software Ltd
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
#include "OgreMeshLodPrecompiledHeaders.h"

namespace Ogre
{
    template<> LodWorkQueueWorker* Singleton<LodWorkQueueWorker>::msSingleton = 0;
    LodWorkQueueWorker* LodWorkQueueWorker::getSingletonPtr(void)
    {
        return msSingleton;
    }
    LodWorkQueueWorker& LodWorkQueueWorker::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }

    LodWorkQueueWorker::LodWorkQueueWorker()
    {
        WorkQueue* wq = Root::getSingleton().getWorkQueue();
        mChannelID = wq->getChannel("PMGen");
        wq->addRequestHandler(mChannelID, this);
    }

    void LodWorkQueueWorker::addRequestToQueue( LodWorkQueueRequest* request )
    {
        WorkQueue* wq = Root::getSingleton().getWorkQueue();
        wq->addRequest(mChannelID, 0, Any(request),0,false,true);
    }

    void LodWorkQueueWorker::addRequestToQueue( LodConfig& lodConfig, LodCollapseCostPtr& cost, LodDataPtr& data, LodInputProviderPtr& input, LodOutputProviderPtr& output, LodCollapserPtr& collapser )
    {
        LodWorkQueueRequest* req = new LodWorkQueueRequest();
        req->config = lodConfig;
        req->cost = cost;
        req->data = data;
        req->input = input;
        req->output = output;
        req->collapser = collapser;
        addRequestToQueue(req);
    }

    LodWorkQueueWorker::~LodWorkQueueWorker()
    {
        Root* root = Root::getSingletonPtr();
        if (root) {
            WorkQueue* wq = root->getWorkQueue();
            if (wq) {
                wq->removeRequestHandler(mChannelID, this);
            }
        }
    }

    void LodWorkQueueWorker::clearPendingLodRequests()
    {
        Ogre::WorkQueue* wq = Root::getSingleton().getWorkQueue();
        wq->abortPendingRequestsByChannel(mChannelID);
    }

    WorkQueue::Response* LodWorkQueueWorker::handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
    {
        // Called on worker thread by WorkQueue.
        LodWorkQueueRequest* request = any_cast<LodWorkQueueRequest*>(req->getData());
        MeshLodGenerator::getSingleton()._process(request->config, request->cost.get(), request->data.get(), request->input.get(), request->output.get(), request->collapser.get());
        return OGRE_NEW WorkQueue::Response(req, true, req->getData());
    }
}
