
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

#ifndef _LodOutputProvider_H__
#define _LodOutputProvider_H__

#include "OgreLodPrerequisites.h"
#include "OgreLodData.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre
{

class _OgreLodExport LodOutputProvider {
public:
    LodOutputProvider(bool useCompression = false);
    virtual ~LodOutputProvider() {}
    virtual void prepare(LodData* data);
    virtual void finalize(LodData* data);
    virtual void bakeManualLodLevel(LodData* data, String& manualMeshName, int lodIndex);
    virtual void bakeLodLevel(LodData* data, int lodIndex);
    virtual void triangleRemoved(LodData* data, LodData::Triangle* tri);
    virtual void triangleChanged(LodData* data, LodData::Triangle* tri);
    virtual void lineRemoved(LodData* data, LodData::Line* tri);
    virtual void lineChanged(LodData* data, LodData::Line* tri);
    virtual void inject(){}

protected:
    bool mUseCompression;

    struct TriangleCache {
        unsigned int vertexID[3];
        bool vertexChanged;
    };
    struct LineCache {
        unsigned int vertexID[2];
        bool vertexChanged;
    };

    typedef std::vector<TriangleCache> TriangleCacheList;
    typedef std::vector<LineCache> LineCacheList;

    /// First pass will create the mTriangleCacheList and second pass will use it.
    /// This is required, because the triangles from first pass will be changed and we need to keep the information.
    TriangleCacheList mTriangleCacheList;
    LineCacheList mLineCacheList;

    bool mFirstBufferPass;
    int mLastIndexBufferID;

    void bakeUncompressed(LodData* data, int lodIndex);
    void bakeFirstPass(LodData* data, int lodIndex);
    void bakeSecondPass(LodData* data, int lodIndex);


    // TODO: remove implementation and make pure virtual. These are just to make the compressed version work.
    virtual size_t getSubMeshCount() { return 0; } // = 0;

    HardwareIndexBufferPtr createIndexBuffer(size_t indexCount);
    // TODO: remove implementation and make pure virtual. These are just to make the compressed version work.
    virtual HardwareIndexBufferPtr createIndexBufferImpl(size_t indexCount) { return nullptr; } // = 0;

    // TODO: remove implementation and make pure virtual. These are just to make the compressed version work.
    virtual void createSubMeshLodIndexData(size_t subMeshIndex, int lodIndex, const HardwareIndexBufferPtr & indexBuffer, size_t indexStart, size_t indexCount) {} // = 0;
};
}
#endif


