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
#include "OgreBspSceneNode.h"
#include "OgreBspSceneManager.h"

namespace Ogre {


    void BspSceneNode::_update(bool updateChildren, bool parentHasChanged)
    {
        bool checkMovables = false;

        if (mNeedParentUpdate || parentHasChanged)
        {
            // This means we've moved
            checkMovables = true;
        }

        // Call superclass
        SceneNode::_update(updateChildren, parentHasChanged);

        if (checkMovables)
        {
            // Check membership of attached objects
            for (auto mov : getAttachedObjects())
            {
                static_cast<BspSceneManager*>(getCreator())->_notifyObjectMoved(
                    mov, this->_getDerivedPosition());
            }
        }

    }
    //-------------------------------------------------------------------------
    MovableObject* BspSceneNode::detachObject(unsigned short index)
    {
        MovableObject* ret = SceneNode::detachObject(index);
        static_cast<BspSceneManager*>(getCreator())->_notifyObjectDetached(ret);
        return ret;
        
    }
    //-------------------------------------------------------------------------
    MovableObject* BspSceneNode::detachObject(const String& name)
    {
        MovableObject* ret = SceneNode::detachObject(name);
        static_cast<BspSceneManager*>(getCreator())->_notifyObjectDetached(ret);
        return ret;
    }
    //-------------------------------------------------------------------------
    void BspSceneNode::detachAllObjects(void)
    {
        for (auto o : getAttachedObjects())
        {
            static_cast<BspSceneManager*>(getCreator())
                ->_notifyObjectDetached(o);
        }
        SceneNode::detachAllObjects();
    }
    //-------------------------------------------------------------------------
    void BspSceneNode::setInSceneGraph(bool inGraph)
    {
        if (isInSceneGraph() != inGraph)
        {
            for (auto o : getAttachedObjects())
            {
                if (!inGraph)
                {
                    // Equivalent to detaching
                    static_cast<BspSceneManager*>(getCreator())->_notifyObjectDetached(o);
                }
                else
                {
                    // move deals with re-adding
                    static_cast<BspSceneManager*>(getCreator())
                        ->_notifyObjectMoved(o, this->_getDerivedPosition());
                }
            }
        }
        mIsInSceneGraph = inGraph;
    }

}



