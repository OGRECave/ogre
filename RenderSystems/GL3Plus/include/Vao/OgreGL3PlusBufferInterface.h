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

#ifndef _Ogre_GL3PlusBufferInterface_H_
#define _Ogre_GL3PlusBufferInterface_H_

#include "OgreGL3PlusPrerequisites.h"

#include "Vao/OgreBufferInterface.h"

namespace Ogre
{
    /** For GL3+, most (if not all) buffers, can be treated with the same code.
        Hence most equivalent functionality is encapsulated here.
    */
    class GL3PlusBufferInterface : public BufferInterface
    {
    protected:
        size_t  mVboPoolIdx;
        GLenum  mTarget;
        GLuint  mVboName;
        void    *mMappedPtr;

    public:
        GL3PlusBufferInterface( size_t vboPoolIdx, GLenum target, GLuint vboName );
        ~GL3PlusBufferInterface();

        size_t getVboPoolIndex(void)                { return mVboPoolIdx; }
        GLenum getTarget(void)                      { return mTarget; }
        GLuint getVboName(void) const               { return mVboName; }

        virtual void* map( size_t elementStart, size_t elementCount, MappingState prevMappingState );
        virtual void unmap( UnmapOptions unmapOption );
    };
}

#endif
