
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

#ifndef _LodInputProviderBuffer_H__
#define _LodInputProviderBuffer_H__

#include "OgreLodPrerequisites.h"
#include "OgreLodInputProvider.h"
#include "OgreLodData.h"
#include "OgreLodBuffer.h"
#include "OgreLogManager.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{

class LodInputProviderBuffer :
    public LodInputProvider
{
public:
    LodInputProviderBuffer(MeshPtr mesh);
    
protected:
    LodInputBuffer mBuffer;

    void addVertexData(LodData* data, size_t subMeshIndex) override;
    const IndexData* getSubMeshIndexData(size_t subMeshIndex) const override;

    const String & getMeshName() override;
    size_t getMeshSharedVertexCount() override;
    float getMeshBoundingSphereRadius() override;

    size_t getSubMeshCount() override;

    bool getSubMeshUseSharedVertices(size_t subMeshIndex) override;
    size_t getSubMeshOwnVertexCount(size_t subMeshIndex) override;
    size_t getSubMeshIndexCount(size_t subMeshIndex) override;
    RenderOperation::OperationType getSubMeshRenderOp(size_t subMeshIndex) override;
};

}

#include "OgreHeaderSuffix.h"

#endif
