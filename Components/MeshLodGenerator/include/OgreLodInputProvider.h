
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

#ifndef _LodInputProvider_H__
#define _LodInputProvider_H__

#include "OgreLodPrerequisites.h"
#include "OgreLodData.h"

namespace Ogre
{

class _OgreLodExport LodInputProvider {
public:
    virtual ~LodInputProvider() {}
    /// Called when the data should be filled with the input.
    virtual void initData(LodData* data);

protected:
    // This helps to find the vertex* in LodData for index buffer indices
    typedef std::vector<LodData::Vertex*> VertexLookupList;
    VertexLookupList mSharedVertexLookup;
    VertexLookupList mVertexLookup;

    virtual void addVertexData(LodData* data, size_t subMeshIndex) = 0;
    virtual const String & getMeshName() = 0;
    virtual size_t getMeshSharedVertexCount() = 0;
    virtual float getMeshBoundingSphereRadius() = 0;

    virtual size_t getSubMeshCount() = 0;

    virtual bool getSubMeshUseSharedVertices(size_t subMeshIndex) = 0;
    virtual size_t getSubMeshOwnVertexCount(size_t subMeshIndex) = 0;
    virtual size_t getSubMeshIndexCount(size_t subMeshIndex) = 0;
    virtual const IndexData* getSubMeshIndexData(size_t subMeshIndex) const = 0;
    virtual RenderOperation::OperationType getSubMeshRenderOp(size_t subMeshIndex) = 0;

private:
    void addIndexData(LodData* data, size_t subMeshIndex);
    void tuneContainerSize(LodData* data);
    void initialize(LodData* data);
};

}
#endif


