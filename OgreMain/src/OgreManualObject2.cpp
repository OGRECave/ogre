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
          mVertices(0),
          mIndices(0),
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
                newSize = mTempVertexBufferSize;
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
                newSize = mTempIndexBufferSize;
            }
            else
            {
                // increase to at least double current
                newSize = std::max(newSize, mTempIndexBufferSize*2);
            }
            numInds = newSize / sizeof(uint32);
            uint32 * tmp = mTempIndexBuffer;
            mTempIndexBuffer = OGRE_ALLOC_T(uint32, numInds, MEMCATEGORY_GEOMETRY);
            if (tmp)
            {
                memcpy(mTempIndexBuffer, tmp, mTempIndexBufferSize);
                OGRE_FREE(tmp, MEMCATEGORY_GEOMETRY);
            }
            mTempIndexBufferSize = newSize;
        }
    }
    //-----------------------------------------------------------------------------
    void ManualObject::estimateVertexCount(size_t vcount)
    {
        resizeVertexBufferIfNeeded(vcount);
//        mEstVertexCount = vcount;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::estimateIndexCount(size_t icount)
    {
        resizeIndexBufferIfNeeded(icount);
//        mEstIndexCount = icount;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::begin(const String& materialName,
        v1::RenderOperation::OperationType opType, const String & groupName)
    {
        if (mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You cannot call begin() again until after you call end()",
                "ManualObject::begin");
        }

        // Check that a valid material was provided
        MaterialPtr material = MaterialManager::getSingleton().getByName(materialName, groupName);

        if( material.isNull() )
        {
            LogManager::getSingleton().logMessage("Can't assign material " + materialName +
                                                  " to the ManualObject " + mName + " because this "
                                                  "Material does not exist. Have you forgotten to define it in a "
                                                  ".material script?", LML_CRITICAL);

            material = MaterialManager::getSingleton().getByName("BaseWhite");

            if (material.isNull())
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Can't assign default material "
                            "to the ManualObject " + mName + ". Did "
                            "you forget to call MaterialManager::initialise()?",
                            "ManualObject::begin");
            }
        }

        mCurrentSection = OGRE_NEW ManualObjectSection(this, materialName, opType, groupName);

        mCurrentSection->mVaoManager = mManager->getDestinationRenderSystem()->getVaoManager();

        mCurrentUpdating = false;

        mSectionList.push_back(mCurrentSection);

        mDeclSize = 0;

        mVertices = 0;
        mIndices = 0;
//        mTexCoordIndex = 0;

        resizeVertexBufferIfNeeded(mVertices);
        resizeIndexBufferIfNeeded(mIndices);

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
//        mTexCoordIndex = 0;

        VertexBufferPacked * vertexBuffer = mCurrentSection->mVao->getVertexBuffers()[0];
        IndexBufferPacked * indexBuffer = mCurrentSection->mVao->getIndexBuffer();

        mVertexBuffer = mVertexBufferCursor = static_cast<float *>(vertexBuffer->map(0, vertexBuffer->getNumElements()));
        mIndexBuffer = mIndexBufferCursor = static_cast<uint32 *>(indexBuffer->map(0, indexBuffer->getNumElements()));
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

        mVertices++;

        // First vertex, update elements and declaration size
        // If updating section, no need to declare elements or resize buffers
        if (!mCurrentUpdating)
        {
            // First vertex was added, we could check for zero and increment,
            // but keeping it like this for consistency with subsequent elements
            if (mVertices == 1)
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

        // update bounds
        Aabb aabb;
        mObjectData.mLocalAabb->getAsAabb( aabb, mObjectData.mIndex );
        aabb.merge(Vector3(x, y, z));
        mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mLocalRadius[mObjectData.mIndex] = aabb.getRadius();

        // reset current texture coord
//        mTexCoordIndex = 0;
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
            VertexElement2 colorElement(VET_COLOUR, VES_DIFFUSE);
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

        if (idx >= 65536)
            mCurrentSection->set32BitIndices(true);

//        // make sure we have index data
//        RenderOperation* rop = mCurrentSection->getRenderOperation();
//        if (!rop->indexData)
//        {
//            rop->indexData = OGRE_NEW IndexData();
//            rop->indexData->indexCount = 0;
//        }
//        rop->useIndexes = true;

        if (!mCurrentUpdating)
        {
            resizeIndexBufferIfNeeded(mIndices + 1);
        }

        *mIndexBufferCursor++ = idx;

        mIndices++;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::triangle(uint32 i1, uint32 i2, uint32 i3)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::index");
        }

        if (mCurrentSection->mOperationType != v1::RenderOperation::OT_TRIANGLE_LIST)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is only valid on triangle lists",
                "ManualObject::index");
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
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You cannot call end() until after you call begin()",
                "ManualObject::end");
        }

        if (mCurrentSection->mVao)
        {
            mCurrentSection->clear();
        }

        if (! mVertices)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "No vertices have been defined in ManualObject.",
                "ManualObject::end");
        }

        if (! mIndices)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "No indices have been defined in ManualObject.",
                "ManualObject::end");
        }

        // pointer that will be returned
        ManualObjectSection * result = mCurrentSection;

        if (!mCurrentUpdating)
        {
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

            IndexBufferPacked *indexBuffer = vaoManager->createIndexBuffer(mCurrentSection->get32BitIndices() ? IndexBufferPacked::IT_32BIT :
                                                                                                                IndexBufferPacked::IT_16BIT,
                                                                           mIndices,
                                                                           BT_DYNAMIC_PERSISTENT_COHERENT,
                                                                           NULL,
                                                                           false);

            char * indexData = static_cast<char *>(indexBuffer->map(0, indexBuffer->getNumElements()));

            assert(indexData);

            if (mCurrentSection->get32BitIndices())
            {
                memcpy(indexData, mIndexBuffer, mIndices * 4);
            }
            else
            {
                for (size_t i = 0; i < mIndices; i++)
                {
                    *((uint16 *)indexData) = mIndexBuffer[i];
                    indexData += 2;
                }
            }

            indexBuffer->unmap(UO_KEEP_PERSISTENT);

            mCurrentSection->mVao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, mCurrentSection->mOperationType);

            mCurrentSection->finalize();

            mRenderables.push_back(mCurrentSection);

            resetBuffers();
        }
        else
        {
            VertexBufferPacked * vertexBuffer = mCurrentSection->mVao->getVertexBuffers()[0];
            IndexBufferPacked * indexBuffer = mCurrentSection->mVao->getIndexBuffer();

            vertexBuffer->unmap(UO_KEEP_PERSISTENT);
            indexBuffer->unmap(UO_KEEP_PERSISTENT);
        }

        mCurrentSection = 0;

        // will return the finished section or NULL if
        // the section was empty (i.e. zero vertices/indices)
        return result;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::setMaterialName(size_t idx, const String& name, const String& group)
    {
        if (idx >= mSectionList.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Index out of bounds!",
                "ManualObject::setMaterialName");
        }

        mSectionList[idx]->setMaterialName(name, group);

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
    unsigned int ManualObject::getNumSections(void) const
    {
        return static_cast< unsigned int >( mSectionList.size() );
    }
    //-----------------------------------------------------------------------------
    const String& ManualObject::getMovableType(void) const
    {
        return ManualObjectFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::_updateRenderQueue(RenderQueue* queue, Camera *camera, const Camera *lodCamera)
    {

    }
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::finalize()
    {
        mVaoPerLod.push_back(mVao);

        setMaterialName(mMaterialName, mGroupName);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::clear()
    {
        VertexArrayObject *vao = mVao;

        if (vao)
        {
            VaoManager *vaoManager = mVaoManager;

            const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();
            VertexBufferPackedVec::const_iterator itBuffers = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator enBuffers = vertexBuffers.end();

            while( itBuffers != enBuffers )
            {
                vaoManager->destroyVertexBuffer(*itBuffers);

                ++itBuffers;
            }

            if (vao->getIndexBuffer())
            {
                vaoManager->destroyIndexBuffer(vao->getIndexBuffer());
            }

            vaoManager->destroyVertexArrayObject(vao);
        }

        mVao = 0;
        mVaoPerLod.clear();
    }
    //-----------------------------------------------------------------------------
    ManualObject::ManualObjectSection::ManualObjectSection(ManualObject* parent,
        const String& materialName, v1::RenderOperation::OperationType opType, const String & groupName)
        : mParent(parent), mMaterialName(materialName), mGroupName(groupName), m32BitIndices(false),
          mVao(0), mOperationType(opType)
    {

    }
    //-----------------------------------------------------------------------------
    ManualObject::ManualObjectSection::~ManualObjectSection()
    {
        clear();
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::getRenderOperation(v1::RenderOperation& op)
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
