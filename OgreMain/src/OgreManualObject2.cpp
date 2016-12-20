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

#include "OgreStableHeaders.h"
#include "OgreManualObject2.h"
#include "OgreException.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgreLogManager.h"

namespace Ogre {

#define TEMP_INITIAL_SIZE 50
#define TEMP_VERTEXSIZE_GUESS sizeof(float) * 12
#define TEMP_INITIAL_VERTEX_SIZE TEMP_VERTEXSIZE_GUESS * TEMP_INITIAL_SIZE
#define TEMP_INITIAL_INDEX_SIZE sizeof(uint32) * TEMP_INITIAL_SIZE
    //-----------------------------------------------------------------------------
    ManualObject::ManualObject( IdType id, ObjectMemoryManager *objectMemoryManager,
                                SceneManager *manager )
        : MovableObject( id, objectMemoryManager, manager, 0 ),
          mCurrentSection(0),
          mCurrentUpdating(false),
          mVertices(0), mIndices(0),
          mEstimatedVertices(0), mEstimatedIndices(0),
          mTempVertexBuffer(0), mTempVertexBufferSize(TEMP_INITIAL_VERTEX_SIZE),
          mTempIndexBuffer(0), mTempIndexBufferSize(TEMP_INITIAL_INDEX_SIZE),
          mVertexBuffer(0), mIndexBuffer(0),
          mDeclSize(0)
    {
        //Start with a null Aabb so that it can grow after calls to position()
        mObjectData.mLocalAabb->setFromAabb( Aabb::BOX_NULL, mObjectData.mIndex );
    }
    //-----------------------------------------------------------------------------
    ManualObject::~ManualObject()
    {
        clear();
    }
    //-----------------------------------------------------------------------------
    void ManualObject::clear(void)
    {
        resetBuffers();

        for (SectionList::iterator i = mSectionList.begin(); i != mSectionList.end(); ++i)
        {
            OGRE_DELETE *i;
        }

        mSectionList.clear();

        mObjectData.mLocalRadius[mObjectData.mIndex] = 0.0f;
        mObjectData.mLocalAabb->setFromAabb( Aabb::BOX_NULL, mObjectData.mIndex );

        mRenderables.clear();

        mCurrentSection = 0;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::resetBuffers(void)
    {
        OGRE_FREE(mTempVertexBuffer, MEMCATEGORY_GEOMETRY);
        OGRE_FREE(mTempIndexBuffer, MEMCATEGORY_GEOMETRY);
        mTempVertexBuffer = 0;
        mTempIndexBuffer = 0;
        mTempVertexBufferSize = TEMP_INITIAL_VERTEX_SIZE;
        mTempIndexBufferSize = TEMP_INITIAL_INDEX_SIZE;
        mVertexBuffer = mVertexBufferCursor  = 0;
        mIndexBuffer = mIndexBufferCursor = 0;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::resizeVertexBufferIfNeeded(size_t numVerts)
    {
        // Calculate byte size
        // Use decl if we know it by now, otherwise default size to pos/norm/texcoord*2
        size_t newSize;
        if (mVertices > 0)
        {
            newSize = mDeclSize * numVerts;
        }
        else
        {
            // estimate - size checks will deal for subsequent verts
            newSize = TEMP_VERTEXSIZE_GUESS * numVerts;
        }
        if (newSize > mTempVertexBufferSize || !mTempVertexBuffer)
        {
            if (!mTempVertexBuffer)
            {
                // init
                newSize = std::max(newSize, mTempVertexBufferSize);
            }
            else
            {
                // increase to at least double current
                newSize = std::max(newSize, mTempVertexBufferSize*2);
            }
            // copy old data
            float * tmp = mTempVertexBuffer;
            mTempVertexBuffer = OGRE_ALLOC_T(float, newSize / sizeof(float), MEMCATEGORY_GEOMETRY);
            if (tmp)
            {
                memcpy(mTempVertexBuffer, tmp, mTempVertexBufferSize);
                // delete old buffer
                OGRE_FREE(tmp, MEMCATEGORY_GEOMETRY);
            }

            mVertexBuffer = mTempVertexBuffer;
            mVertexBufferCursor = mVertexBuffer + mVertices * mDeclSize / sizeof(float);
            mTempVertexBufferSize = newSize;
        }
    }
    //-----------------------------------------------------------------------------
    void ManualObject::resizeIndexBufferIfNeeded(size_t numInds)
    {
        size_t newSize = numInds * sizeof(uint32);
        if (newSize > mTempIndexBufferSize || !mTempIndexBuffer)
        {
            if (!mTempIndexBuffer)
            {
                // init
                newSize = std::max(newSize, mTempIndexBufferSize);
            }
            else
            {
                // increase to at least double current
                newSize = std::max(newSize, mTempIndexBufferSize*2);
            }
            char * tmp = mTempIndexBuffer;
            mTempIndexBuffer = OGRE_ALLOC_T(char, newSize, MEMCATEGORY_GEOMETRY);
            if (tmp)
            {
                memcpy(mTempIndexBuffer, tmp, mTempIndexBufferSize);
                OGRE_FREE(tmp, MEMCATEGORY_GEOMETRY);
            }

            mIndexBuffer = mTempIndexBuffer;
            mIndexBufferCursor = mIndexBuffer + mIndices * sizeof(uint32);
            mTempIndexBufferSize = newSize;
        }
    }
    //-----------------------------------------------------------------------------
    void ManualObject::estimateVertexCount(size_t vcount)
    {
        resizeVertexBufferIfNeeded(vcount);
        mEstimatedVertices = vcount;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::estimateIndexCount(size_t icount)
    {
        resizeIndexBufferIfNeeded(icount);
        mEstimatedIndices = icount;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::begin(const String & datablockName, OperationType opType)
    {
        if (mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You cannot call begin() again until after you call end()",
                "ManualObject::begin");
        }

        mCurrentSection = OGRE_NEW ManualObjectSection(this, datablockName, opType);

        mCurrentSection->mVaoManager = mManager->getDestinationRenderSystem()->getVaoManager();

        mCurrentUpdating = false;

        mSectionList.push_back(mCurrentSection);

        mCurrentDatablockName = datablockName;

        mDeclSize = 0;

        mVertices = 0;
        mIndices = 0;

        // Will initialize to default size
        resizeVertexBufferIfNeeded(mEstimatedVertices);
        resizeIndexBufferIfNeeded(mEstimatedIndices);

        mVertexBuffer = mVertexBufferCursor = mTempVertexBuffer;
        mIndexBuffer = mIndexBufferCursor = mTempIndexBuffer;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::beginUpdate(size_t sectionIndex)
    {
        if (mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You cannot call begin() again until after you call end()",
                "ManualObject::beginUpdate");
        }

        if (sectionIndex >= mSectionList.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Invalid section index - out of range.",
                "ManualObject::beginUpdate");
        }

        mCurrentSection = mSectionList[sectionIndex];

        mCurrentUpdating = true;

        mVertices = 0;
        mIndices = 0;

        VertexBufferPacked * vertexBuffer = mCurrentSection->mVao->getVertexBuffers()[0];
        IndexBufferPacked * indexBuffer = mCurrentSection->mVao->getIndexBuffer();

        mVertexBuffer = mVertexBufferCursor = static_cast<float *>(vertexBuffer->map(0, vertexBuffer->getNumElements()));
        mIndexBuffer = mIndexBufferCursor = static_cast<char *>(indexBuffer->map(0, indexBuffer->getNumElements()));
    }
    //-----------------------------------------------------------------------------
    void ManualObject::position(const Vector3& pos)
    {
        position(pos.x, pos.y, pos.z);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::position(Real x, Real y, Real z)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::position");
        }

        // First vertex, update elements and declaration size
        // If updating section, no need to declare elements or resize buffers
        if (!mCurrentUpdating)
        {
            if (mVertices == 0)
            {
                // defining declaration
                VertexElement2 positionElement(VET_FLOAT3, VES_POSITION);
                mCurrentSection->mVertexElements.push_back(positionElement);

                mDeclSize += v1::VertexElement::getTypeSize(VET_FLOAT3);
            }
            else
            {
                // Subsequent vertices resize the vertex buffer if needed
                resizeVertexBufferIfNeeded(mVertices + 1);
            }
        }

        *mVertexBufferCursor++ = x;
        *mVertexBufferCursor++ = y;
        *mVertexBufferCursor++ = z;

        mVertices++;

        mCurrentSection->mAabb.merge(Vector3(x, y, z));
    }
    //-----------------------------------------------------------------------------
    void ManualObject::normal(const Vector3& norm)
    {
        normal(norm.x, norm.y, norm.z);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::normal(Real x, Real y, Real z)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::normal");
        }

        // First time a normal is being added
        if (mVertices == 1 &&
            !mCurrentUpdating)
        {
            // defining declaration
            VertexElement2 normalElement(VET_FLOAT3, VES_NORMAL);
            mCurrentSection->mVertexElements.push_back(normalElement);

            mDeclSize += v1::VertexElement::getTypeSize(VET_FLOAT3);
        }

        *mVertexBufferCursor++ = x;
        *mVertexBufferCursor++ = y;
        *mVertexBufferCursor++ = z;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::tangent(const Vector3& tan)
    {
        tangent(tan.x, tan.y, tan.z);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::tangent(Real x, Real y, Real z)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::tangent");
        }

        // First time a tangent is being added
        if (mVertices == 1 &&
            !mCurrentUpdating)
        {
            // defining declaration
            VertexElement2 tangentElement(VET_FLOAT3, VES_TANGENT);
            mCurrentSection->mVertexElements.push_back(tangentElement);

            mDeclSize += v1::VertexElement::getTypeSize(VET_FLOAT3);
        }

        *mVertexBufferCursor++ = x;
        *mVertexBufferCursor++ = y;
        *mVertexBufferCursor++ = z;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(Real u)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::textureCoord");
        }

        if (mVertices == 1 &&
            !mCurrentUpdating)
        {
            // defining declaration
            VertexElement2 texCoordElement(VET_FLOAT1, VES_TEXTURE_COORDINATES);
            mCurrentSection->mVertexElements.push_back(texCoordElement);

            mDeclSize += v1::VertexElement::getTypeSize(VET_FLOAT1);
        }

        *mVertexBufferCursor++ = u;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(Real u, Real v)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::textureCoord");
        }

        if (mVertices == 1 &&
            !mCurrentUpdating)
        {
            // defining declaration
            VertexElement2 texCoordElement(VET_FLOAT2, VES_TEXTURE_COORDINATES);
            mCurrentSection->mVertexElements.push_back(texCoordElement);

            mDeclSize += v1::VertexElement::getTypeSize(VET_FLOAT2);
        }

        *mVertexBufferCursor++ = u;
        *mVertexBufferCursor++ = v;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(Real u, Real v, Real w)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::textureCoord");
        }

        if (mVertices == 1 &&
            !mCurrentUpdating)
        {
            // defining declaration
            VertexElement2 texCoordElement(VET_FLOAT3, VES_TEXTURE_COORDINATES);
            mCurrentSection->mVertexElements.push_back(texCoordElement);

            mDeclSize += v1::VertexElement::getTypeSize(VET_FLOAT3);
        }

        *mVertexBufferCursor++ = u;
        *mVertexBufferCursor++ = v;
        *mVertexBufferCursor++ = w;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(Real x, Real y, Real z, Real w)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::textureCoord");
        }

        if (mVertices == 1 &&
            !mCurrentUpdating)
        {
            // defining declaration
            VertexElement2 texCoordElement(VET_FLOAT4, VES_TEXTURE_COORDINATES);
            mCurrentSection->mVertexElements.push_back(texCoordElement);

            mDeclSize += v1::VertexElement::getTypeSize(VET_FLOAT4);
        }

        *mVertexBufferCursor++ = x;
        *mVertexBufferCursor++ = y;
        *mVertexBufferCursor++ = z;
        *mVertexBufferCursor++ = w;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(const Vector2& uv)
    {
        textureCoord(uv.x, uv.y);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(const Vector3& uvw)
    {
        textureCoord(uvw.x, uvw.y, uvw.z);
    }
    //---------------------------------------------------------------------
    void ManualObject::textureCoord(const Vector4& xyzw)
    {
        textureCoord(xyzw.x, xyzw.y, xyzw.z, xyzw.w);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::colour(const ColourValue& col)
    {
        colour(col.r, col.g, col.b, col.a);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::colour(Real r, Real g, Real b, Real a)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::colour");
        }

        if (mVertices == 1 &&
            !mCurrentUpdating)
        {
            // defining declaration
            VertexElement2 colorElement(VET_FLOAT4, VES_DIFFUSE);
            mCurrentSection->mVertexElements.push_back(colorElement);

            mDeclSize += v1::VertexElement::getTypeSize(VET_FLOAT4);
        }

        *mVertexBufferCursor++ = r;
        *mVertexBufferCursor++ = g;
        *mVertexBufferCursor++ = b;
        *mVertexBufferCursor++ = a;

    }
    //-----------------------------------------------------------------------------
    void ManualObject::specular(const ColourValue & col)
    {
        specular(col.r, col.g, col.b, col.a);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::specular(Real r, Real g, Real b, Real a)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::specular");
        }

        if (mVertices == 1 &&
            !mCurrentUpdating)
        {
            // defining declaration
            VertexElement2 colorElement(VET_COLOUR, VES_SPECULAR);
            mCurrentSection->mVertexElements.push_back(colorElement);

            mDeclSize += v1::VertexElement::getTypeSize(VET_COLOUR);
        }

        *mVertexBufferCursor++ = r;
        *mVertexBufferCursor++ = g;
        *mVertexBufferCursor++ = b;
        *mVertexBufferCursor++ = a;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::index(uint32 idx)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::index");
        }

        if (!mCurrentUpdating)
        {
            if (idx >= 65536)
                mCurrentSection->m32BitIndices = true;

            resizeIndexBufferIfNeeded(mIndices + 1);

            *((uint32 *)mIndexBufferCursor) = idx;
            mIndexBufferCursor += sizeof(uint32);
        }
        else if (mCurrentSection->m32BitIndices)
        {
            *((uint32 *)mIndexBufferCursor) = idx;
            mIndexBufferCursor += sizeof(uint32);
        }
        else
        {
            *((uint16 *)mIndexBufferCursor) = idx;
            mIndexBufferCursor += sizeof(uint16);
        }

        mIndices++;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::line(uint32 i1, uint32 i2)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::line");
        }

        if (mCurrentSection->mOperationType != OT_LINE_LIST)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is only valid on line lists",
                "ManualObject::line");
        }

        index(i1);
        index(i2);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::triangle(uint32 i1, uint32 i2, uint32 i3)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::triangle");
        }

        if (mCurrentSection->mOperationType != OT_TRIANGLE_LIST)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is only valid on triangle lists",
                "ManualObject::triangle");
        }

        index(i1);
        index(i2);
        index(i3);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::quad(uint32 i1, uint32 i2, uint32 i3, uint32 i4)
    {
        // first tri
        triangle(i1, i2, i3);
        // second tri
        triangle(i3, i4, i1);
    }
    //-----------------------------------------------------------------------------
    size_t ManualObject::getCurrentVertexCount() const
    {
        if (!mCurrentSection)
            return 0;
        
        return mVertices;
    }
    //-----------------------------------------------------------------------------
    size_t ManualObject::getCurrentIndexCount() const
    {
        if (!mCurrentSection)
            return 0;

        return mIndices;
    }
    //-----------------------------------------------------------------------------
    ManualObject::ManualObjectSection* ManualObject::end(void)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                "You cannot call end() until after you call begin()",
                "ManualObject::end");
        }

        // pointer that will be returned
        ManualObjectSection * result = mCurrentSection;

        // Support calling begin() and end() without defining any geometry
        if (! mVertices)
        {
            mCurrentSection = 0;
            return result;
        }

        if (! mIndices)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                "No indices have been defined in ManualObject. This is not supported.",
                "ManualObject::end");
        }

        if (!mCurrentUpdating)
        {
            if (mCurrentSection->mVao)
            {
                mCurrentSection->clear();
            }

            VaoManager *vaoManager = mCurrentSection->mVaoManager;
            VertexBufferPackedVec vertexBuffers;

            //Create the vertex buffer
            VertexBufferPacked * vertexBuffer = vaoManager->createVertexBuffer(mCurrentSection->mVertexElements,
                                                                               mVertices,
                                                                               BT_DYNAMIC_PERSISTENT_COHERENT,
                                                                               NULL, false);
            vertexBuffers.push_back(vertexBuffer);

            char * vertexData = static_cast<char *>(vertexBuffer->map(0, vertexBuffer->getNumElements()));

            assert(vertexData);

            memcpy(vertexData, mVertexBuffer, mDeclSize * mVertices);

            vertexBuffer->unmap(UO_KEEP_PERSISTENT);

            IndexBufferPacked *indexBuffer = vaoManager->createIndexBuffer(mCurrentSection->m32BitIndices ? IndexBufferPacked::IT_32BIT :
                                                                                                            IndexBufferPacked::IT_16BIT,
                                                                           mIndices,
                                                                           BT_DYNAMIC_PERSISTENT_COHERENT,
                                                                           NULL,
                                                                           false);

            char * indexData = static_cast<char *>(indexBuffer->map(0, indexBuffer->getNumElements()));

            assert(indexData);

            if (mCurrentSection->m32BitIndices)
            {
                memcpy(indexData, mIndexBuffer, mIndices * sizeof(uint32));
            }
            else
            {
                uint32 * l32Buffer = (uint32 *)mIndexBuffer;

                for (size_t i = 0; i < mIndices; i++)
                {
                    *((uint16 *)indexData) = l32Buffer[i];
                    indexData += sizeof(uint16);
                }
            }

            indexBuffer->unmap(UO_KEEP_PERSISTENT);

            mCurrentSection->mVao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, mCurrentSection->mOperationType);

            mCurrentSection->mVaoPerLod[0].push_back(mCurrentSection->mVao);
            mCurrentSection->mVaoPerLod[1].push_back(mCurrentSection->mVao);
            mCurrentSection->setDatablock(mCurrentDatablockName);

            mRenderables.push_back(mCurrentSection);

            resetBuffers();

            mCurrentDatablockName.clear();
        }
        else
        {
            const VertexBufferPackedVec &vertexBuffers = mCurrentSection->mVao->getVertexBuffers();
            VertexBufferPackedVec::const_iterator itBuffers = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator endBuffers = vertexBuffers.end();

            while (itBuffers != endBuffers)
            {
                VertexBufferPacked * vertexBuffer = *itBuffers;
                vertexBuffer->unmap(UO_KEEP_PERSISTENT);
                itBuffers++;
            }

            IndexBufferPacked * indexBuffer = mCurrentSection->mVao->getIndexBuffer();
            indexBuffer->unmap(UO_KEEP_PERSISTENT);

            mVertexBuffer = mVertexBufferCursor = 0;
            mIndexBuffer = mIndexBufferCursor = 0;
        }

        // update bounds
        Aabb aabb;
        mObjectData.mLocalAabb->getAsAabb(aabb, mObjectData.mIndex);
        aabb.merge(mCurrentSection->mAabb);
        mObjectData.mLocalAabb->setFromAabb(aabb, mObjectData.mIndex);
        mObjectData.mLocalRadius[mObjectData.mIndex] = aabb.getRadius();

        mCurrentSection = 0;

        // will return the finished section or NULL if
        // the section was empty (i.e. zero vertices/indices)
        return result;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::setDatablock(size_t idx, const String& name)
    {
        if (idx >= mSectionList.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Index out of bounds!",
                "ManualObject::setMaterialName");
        }

        mSectionList[idx]->setDatablock(name);
    }
    //-----------------------------------------------------------------------
    ManualObject::ManualObjectSection* ManualObject::getSection(unsigned int inIndex) const
    {
        if (inIndex >= mSectionList.size())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "Index out of bounds.",
            "ManualObject::getSection");
        return mSectionList[inIndex];
    }
    //-----------------------------------------------------------------------
    ManualObject::ManualObjectSection* ManualObject::getSection(const String & name) const
    {
        for (SectionList::const_iterator i = mSectionList.begin();
             i != mSectionList.end(); ++i)
        {
            if ((*i)->mName == name)
            {
                return *i;
            }
        }

        return 0;
    }
    //-----------------------------------------------------------------------
    unsigned int ManualObject::getNumSections(void) const
    {
        return static_cast< unsigned int >( mSectionList.size() );
    }
    //-----------------------------------------------------------------------
    void ManualObject::removeSection(unsigned int idx)
    {
        if (idx >= mSectionList.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Index out of bounds!",
                "ManualObject::removeSection");
        }

        SectionList::iterator it = mSectionList.begin() + idx;

        if (*it == mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                "Can't remove section while building it's geometry!",
                "ManualObject::removeSection");
        }

        OGRE_DELETE *it;

        mSectionList.erase(it);
        mRenderables.erase(mRenderables.begin() + idx);

        Aabb aabb;

        for (SectionList::iterator i = mSectionList.begin(); i != mSectionList.end(); ++i)
        {
            aabb.merge((*i)->mAabb);
        }

        mObjectData.mLocalAabb->setFromAabb(aabb, mObjectData.mIndex);
        mObjectData.mLocalRadius[mObjectData.mIndex] = aabb.getRadius();
    }
    //-----------------------------------------------------------------------------
    size_t ManualObject::currentIndexCount()
    {
        return mIndices;
    }
    //-----------------------------------------------------------------------------
    size_t ManualObject::currentVertexCount()
    {
        return mVertices;
    }
    //-----------------------------------------------------------------------------
    const String& ManualObject::getMovableType(void) const
    {
        return ManualObjectFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::clear()
    {
        VertexArrayObject *vao = mVao;

        if (vao)
        {
            VaoManager *vaoManager = mVaoManager;

            const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();
            VertexBufferPackedVec::const_iterator itBuffers = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator endBuffers = vertexBuffers.end();

            while (itBuffers != endBuffers)
            {
                VertexBufferPacked * vertexBuffer = *itBuffers;

                if (vertexBuffer->getMappingState() != Ogre::MS_UNMAPPED)
                {
                    vertexBuffer->unmap(UO_UNMAP_ALL);
                }

                vaoManager->destroyVertexBuffer(vertexBuffer);

                ++itBuffers;
            }

            IndexBufferPacked * indexBuffer = vao->getIndexBuffer();

            if (indexBuffer)
            {
                if (indexBuffer->getMappingState() != Ogre::MS_UNMAPPED)
                {
                    indexBuffer->unmap(UO_UNMAP_ALL);
                }

                vaoManager->destroyIndexBuffer(indexBuffer);
            }

            vaoManager->destroyVertexArrayObject(vao);
        }

        mVao = 0;
        mVaoPerLod[0].clear();
        mVaoPerLod[1].clear();
    }
    //-----------------------------------------------------------------------------
    ManualObject::ManualObjectSection::ManualObjectSection(ManualObject* parent,
        const String& datablockName, OperationType opType)
        : mParent(parent), mVao(0), mOperationType(opType), m32BitIndices(false)
    {

    }
    //-----------------------------------------------------------------------------
    ManualObject::ManualObjectSection::~ManualObjectSection()
    {
        clear();
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::getRenderOperation(v1::RenderOperation& op, bool casterPass)
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "ManualObject does not implement getRenderOperation. "
                     "Use MovableObject::setRenderQueueGroup to change the group.",
                     "ManualObjectSection::getRenderOperation" );
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::getWorldTransforms(Matrix4* xform) const
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "ManualObject does not implement getWorldTransforms. "
                     "Use MovableObject::setRenderQueueGroup to change the group.",
                     "ManualObjectSection::getWorldTransforms" );
    }
    //-----------------------------------------------------------------------------
    const LightList& ManualObject::ManualObjectSection::getLights(void) const
    {
        return mParent->queryLights();
    }
    //-----------------------------------------------------------------------------
    bool ManualObject::ManualObjectSection::getCastsShadows(void) const
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "ManualObject do not implement getCastsShadows.",
                     "ManualObjectSection::getCastsShadows" );
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::setName(const String & name)
    {
        mName = name;
    }
    //-----------------------------------------------------------------------------
    const String &ManualObject::ManualObjectSection::getName()
    {
        return mName;
    }
    //-----------------------------------------------------------------------------
    String ManualObjectFactory::FACTORY_TYPE_NAME = "ManualObject2";
    //-----------------------------------------------------------------------------
    const String& ManualObjectFactory::getType(void) const
    {
        return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------------
    MovableObject* ManualObjectFactory::createInstanceImpl( IdType id,
                                                            ObjectMemoryManager *objectMemoryManager,
                                                            SceneManager *manager,
                                                            const NameValuePairList* params )
    {
        return OGRE_NEW ManualObject( id, objectMemoryManager, manager );
    }
    //-----------------------------------------------------------------------------
    void ManualObjectFactory::destroyInstance( MovableObject* obj)
    {
        OGRE_DELETE obj;
    }
}
