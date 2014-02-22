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

#ifndef __OgreUniformScalableTask_H__
#define __OgreUniformScalableTask_H__

#include "OgrePrerequisites.h"

namespace Ogre
{
    /** A uniform task is a highly parallelizable task that can be divided
        in many threads where all threads take near the same amount of time to perform.
        They're very simple and not many jobs fit the needs (usually a task-based solution
        used thread pools is preferred) but there is a prime candidate for this type
        of solution: Updating all nodes' position & orientation copied from the
        physics engine.
        Use it wherever it is accepted in Ogre.
        For example @see SceneManager::processUserScalableTask
    */
    class _OgreExport UniformScalableTask
    {
    public:
        /** Overload this function to perform whatever you want. It will be
            called from all worker threads at the same time.
        @param threadId
            The index of the thread it is being called from. An index is
            guaranteed to be in range [0; numThreads)
        @param numThreads
            Number of total threads
        */
        virtual void execute( size_t threadId, size_t numThreads ) = 0;
    };
};

#endif
