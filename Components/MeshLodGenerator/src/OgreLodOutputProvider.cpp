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

#include "OgreLodOutputProvider.h"

namespace Ogre
{
    HardwareIndexBufferPtr LodOutputProvider::createIndexBuffer(size_t indexCount)
    {
        //If the index is empty we need to create a "dummy" triangle, just to keep the index buffer from being empty.
        //The main reason for this is that the OpenGL render system will crash with a segfault unless the index has some values.
        //This should hopefully be removed with future versions of Ogre. The most preferred solution would be to add the
        //ability for a submesh to be excluded from rendering for a given LOD (which isn't possible currently 2012-12-09).
        HardwareIndexBufferPtr buffer = createIndexBufferImpl(indexCount ? indexCount : 3);

        //Check if we should fill it with a "dummy" triangle.
        if (indexCount == 0)
        {
            void * addr = buffer->lock(HardwareBuffer::HBL_DISCARD);
            memset(addr, 0, 3 * buffer->getIndexSize());
            buffer->unlock();
        }

        return buffer;
    }

}
