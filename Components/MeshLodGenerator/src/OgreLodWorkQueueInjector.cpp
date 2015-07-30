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

#include "OgreLodWorkQueueInjector.h"
#include "OgreLodWorkQueueInjectorListener.h"
#include "OgreLodWorkQueueRequest.h"
#include "OgreMeshLodGenerator.h"
#include "OgreRoot.h"

namespace Ogre
{
    template<> LodWorkQueueInjector* Singleton<LodWorkQueueInjector>::msSingleton = 0;
    LodWorkQueueInjector* LodWorkQueueInjector::getSingletonPtr(void)
    {
        return msSingleton;
    }
    LodWorkQueueInjector& LodWorkQueueInjector::getSingleton(void)
    {
        assert( msSingleton );
        return ( *msSingleton );
    }


    LodWorkQueueInjector::LodWorkQueueInjector() :
        mInjectorListener(0)
    {
        WorkQueue* wq = Root::getSingleton().getWorkQueue();
        unsigned short workQueueChannel = wq->getChannel("PMGen");
        wq->addResponseHandler(workQueueChannel, this);
    }

    LodWorkQueueInjector::~LodWorkQueueInjector()
    {
        Root* root = Root::getSingletonPtr();
        if (root)
        {
            WorkQueue* wq = root->getWorkQueue();
            if (wq)
            {
                unsigned short workQueueChannel = wq->getChannel("PMGen");
                wq->removeResponseHandler(workQueueChannel, this);
            }
        }
    }

    void LodWorkQueueInjector::handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
    {
        LodWorkQueueRequest* request = any_cast<LodWorkQueueRequest*>(res->getData());

        if(mInjectorListener)
        {
            if(!mInjectorListener->shouldInject(request))
            {
                return;
            }
        }

        request->output->inject();
        MeshLodGenerator::_configureMeshLodUsage(request->config);
        //lodConfig.mesh->buildEdgeList();

        if(mInjectorListener)
        {
            mInjectorListener->injectionCompleted(request);
        }
    }


}
