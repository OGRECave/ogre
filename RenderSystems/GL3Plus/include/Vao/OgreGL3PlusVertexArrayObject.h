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

#ifndef _Ogre_GL3PlusVertexArrayObject_H_
#define _Ogre_GL3PlusVertexArrayObject_H_

#include "OgreGL3PlusPrerequisites.h"

#include "Vao/OgreVertexArrayObject.h"

namespace Ogre
{
    struct _OgreGL3PlusExport GL3PlusVertexArrayObject : public VertexArrayObject
    {
        GLint   mPrimType[2];

        GL3PlusVertexArrayObject( GLuint vaoName, uint32 renderQueueId,
                                  const VertexBufferPackedVec &vertexBuffers,
                                  IndexBufferPacked *indexBuffer,
                                  OperationType opType ) :
            VertexArrayObject( vaoName, renderQueueId, 0, vertexBuffers, indexBuffer, opType )
        {
            switch( opType )
            {
            case OT_POINT_LIST:
                mPrimType[0] = GL_POINTS;
                mPrimType[1] = GL_POINTS;
                break;
            case OT_LINE_LIST:
                mPrimType[0] = GL_LINES;
                mPrimType[1] = GL_LINES_ADJACENCY;
                break;
            case OT_LINE_STRIP:
                mPrimType[0] = GL_LINE_STRIP;
                mPrimType[1] = GL_LINE_STRIP_ADJACENCY;
                break;
            default:
            case OT_TRIANGLE_LIST:
                mPrimType[0] = GL_TRIANGLES;
                mPrimType[1] = GL_TRIANGLES_ADJACENCY;
                break;
            case OT_TRIANGLE_STRIP:
                mPrimType[0] = GL_TRIANGLE_STRIP;
                mPrimType[1] = GL_TRIANGLE_STRIP_ADJACENCY;
                break;
            case OT_TRIANGLE_FAN:
                mPrimType[0] = GL_TRIANGLE_FAN;
                mPrimType[1] = GL_TRIANGLE_FAN;
                break;
            }
        }
    };
}

#endif
