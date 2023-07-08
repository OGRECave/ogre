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

#ifndef _LodBuffer_H__
#define _LodBuffer_H__

#include "OgreLodPrerequisites.h"
#include "OgreSharedPtr.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
/** \addtogroup Optional
*  @{
*/
/** \addtogroup MeshLodGenerator
*  @{
*/
/// Thread-safe buffer for storing Hardware vertex buffer
struct LodVertexBuffer {
    size_t vertexCount;
    HardwareBufferPtr vertexBuffer;
    HardwareBufferPtr vertexNormalBuffer;
    void fillBuffer(Ogre::VertexData* data);
};
/// Data representing all required information from a Mesh. Used by LodInputProviderBuffer.
struct LodInputBuffer
{
    struct _OgreLodExport Submesh {
        IndexData indexBuffer;
        LodVertexBuffer vertexBuffer;
        bool useSharedVertexBuffer;
        RenderOperation::OperationType operationType;
    };
    std::vector<Submesh> submesh;
    LodVertexBuffer sharedVertexBuffer;
    String meshName;
    Real boundingSphereRadius;
    void fillBuffer(Ogre::MeshPtr mesh);
};
/// Data representing the output of the Mesh reduction. Used by LodOutputProviderBuffer.
struct LodOutputBuffer {

    struct _OgreLodExport Submesh {
        std::vector<IndexData> genIndexBuffers;
    };
    /// Contains every generated indexBuffer from every submesh. submeshCount*lodLevelCount buffers.
    std::vector<Submesh> submesh;
};
/** @} */
/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
