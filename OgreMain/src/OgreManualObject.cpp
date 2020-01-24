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
#include "OgreEdgeListBuilder.h"

namespace Ogre {

#define TEMP_INITIAL_SIZE 50
#define TEMP_VERTEXSIZE_GUESS sizeof(float) * 12
#define TEMP_INITIAL_VERTEX_SIZE TEMP_VERTEXSIZE_GUESS * TEMP_INITIAL_SIZE
#define TEMP_INITIAL_INDEX_SIZE sizeof(uint32) * TEMP_INITIAL_SIZE
    //-----------------------------------------------------------------------------
    ManualObject::ManualObject(const String& name)
        : MovableObject(name),
          mDynamic(false), mCurrentSection(0), mCurrentUpdating(false), mFirstVertex(true),
          mTempVertexPending(false),
          mTempVertexBuffer(0), mTempVertexSize(TEMP_INITIAL_VERTEX_SIZE),
          mTempIndexBuffer(0), mTempIndexSize(TEMP_INITIAL_INDEX_SIZE),
          mDeclSize(0), mEstVertexCount(0), mEstIndexCount(0), mTexCoordIndex(0), 
          mRadius(0), mAnyIndexed(false), mEdgeList(0), 
          mUseIdentityProjection(false), mUseIdentityView(false), mKeepDeclarationOrder(false)
    {
    }
    //-----------------------------------------------------------------------------
    ManualObject::~ManualObject()
    {
        clear();
    }
    //-----------------------------------------------------------------------------
    void ManualObject::clear(void)
    {
        resetTempAreas();
        for (SectionList::iterator i = mSectionList.begin(); i != mSectionList.end(); ++i)
        {
            OGRE_DELETE *i;
        }
        mSectionList.clear();
        mRadius = 0;
        mAABB.setNull();
        OGRE_DELETE mEdgeList;
        mEdgeList = 0;
        mAnyIndexed = false;

        clearShadowRenderableList(mShadowRenderables);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::resetTempAreas(void)
    {
        OGRE_FREE(mTempVertexBuffer, MEMCATEGORY_GEOMETRY);
        OGRE_FREE(mTempIndexBuffer, MEMCATEGORY_GEOMETRY);
        mTempVertexBuffer = 0;
        mTempIndexBuffer = 0;
        mTempVertexSize = TEMP_INITIAL_VERTEX_SIZE;
        mTempIndexSize = TEMP_INITIAL_INDEX_SIZE;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::resizeTempVertexBufferIfNeeded(size_t numVerts)
    {
        // Calculate byte size
        // Use decl if we know it by now, otherwise default size to pos/norm/texcoord*2
        size_t newSize = (mFirstVertex ? TEMP_VERTEXSIZE_GUESS : mDeclSize) * numVerts;

        if(newSize <= mTempVertexSize && mTempVertexBuffer) {
            return;
        }

        // init or increase to at least double current
        newSize = std::max(newSize, mTempVertexBuffer ? mTempVertexSize*2 : mTempVertexSize);

        // copy old data
        char* tmp = mTempVertexBuffer;
        mTempVertexBuffer = OGRE_ALLOC_T(char, newSize, MEMCATEGORY_GEOMETRY);
        if (tmp)
        {
            memcpy(mTempVertexBuffer, tmp, mTempVertexSize);
            // delete old buffer
            OGRE_FREE(tmp, MEMCATEGORY_GEOMETRY);
        }
        mTempVertexSize = newSize;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::resizeTempIndexBufferIfNeeded(size_t numInds)
    {
        size_t newSize = numInds * sizeof(uint32);

        if(newSize <= mTempIndexSize && mTempIndexBuffer) {
            return;
        }

        // init or increase to at least double current
        newSize = std::max(newSize, mTempIndexBuffer ? mTempIndexSize*2 : mTempIndexSize);

        numInds = newSize / sizeof(uint32);
        uint32* tmp = mTempIndexBuffer;
        mTempIndexBuffer = OGRE_ALLOC_T(uint32, numInds, MEMCATEGORY_GEOMETRY);
        if (tmp)
        {
            memcpy(mTempIndexBuffer, tmp, mTempIndexSize);
            OGRE_FREE(tmp, MEMCATEGORY_GEOMETRY);
        }
        mTempIndexSize = newSize;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::estimateVertexCount(size_t vcount)
    {
        resizeTempVertexBufferIfNeeded(vcount);
        mEstVertexCount = vcount;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::estimateIndexCount(size_t icount)
    {
        resizeTempIndexBufferIfNeeded(icount);
        mEstIndexCount = icount;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::begin(const String& materialName,
        RenderOperation::OperationType opType, const String& groupName)
    {
        if (mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You cannot call begin() again until after you call end()",
                "ManualObject::begin");
        }

        // Check that a valid material was provided
        MaterialPtr material = MaterialManager::getSingleton().getByName(materialName, groupName);

        if(!material)
        {
            LogManager::getSingleton().logMessage("Can't assign material " + materialName +
                                                  " to the ManualObject " + mName + " because this "
                                                  "Material does not exist in group " + groupName +
                                                  ". Have you forgotten to define it in a "
                                                  ".material script?", LML_CRITICAL);

            material = MaterialManager::getSingleton().getDefaultMaterial();
        }

        mCurrentSection = OGRE_NEW ManualObjectSection(this, material, opType);
        mCurrentUpdating = false;
        mCurrentSection->setUseIdentityProjection(mUseIdentityProjection);
        mCurrentSection->setUseIdentityView(mUseIdentityView);
        mSectionList.push_back(mCurrentSection);
        mFirstVertex = true;
        mDeclSize = 0;
        mTexCoordIndex = 0;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::begin(const MaterialPtr& mat, RenderOperation::OperationType opType)
    {
      if (mCurrentSection)
      {
          OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
              "You cannot call begin() again until after you call end()",
              "ManualObject::begin");
      }

      if (mat)
      {
          mCurrentSection = OGRE_NEW ManualObjectSection(this, mat, opType);
      }
      else
      {
          LogManager::getSingleton().logMessage("Can't assign null material", LML_CRITICAL);
          const MaterialPtr defaultMat = MaterialManager::getSingleton().getDefaultMaterial();
          mCurrentSection = OGRE_NEW ManualObjectSection(this, defaultMat, opType);
      }

      mCurrentUpdating = false;
      mCurrentSection->setUseIdentityProjection(mUseIdentityProjection);
      mCurrentSection->setUseIdentityView(mUseIdentityView);
      mSectionList.push_back(mCurrentSection);
      mFirstVertex = true;
      mDeclSize = 0;
      mTexCoordIndex = 0;
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
        mFirstVertex = true;
        mTexCoordIndex = 0;
        // reset vertex & index count
        RenderOperation* rop = mCurrentSection->getRenderOperation();
        rop->vertexData->vertexCount = 0;
        if (rop->indexData)
            rop->indexData->indexCount = 0;
        rop->useIndexes = false;
        mDeclSize = rop->vertexData->vertexDeclaration->getVertexSize(0);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::position(const Vector3& pos)
    {
        position(pos.x, pos.y, pos.z);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::position(float x, float y, float z)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::position");
        }
        if (mTempVertexPending)
        {
            // bake current vertex
            copyTempVertexToBuffer();
            mFirstVertex = false;
        }

        if (mFirstVertex && !mCurrentUpdating)
        {
            // defining declaration
            mDeclSize += mCurrentSection->getRenderOperation()->vertexData->vertexDeclaration
                ->addElement(0, mDeclSize, VET_FLOAT3, VES_POSITION).getSize();
        }

        mTempVertex.position.x = x;
        mTempVertex.position.y = y;
        mTempVertex.position.z = z;

        // update bounds
        mAABB.merge(mTempVertex.position);
        mRadius = std::max(mRadius, mTempVertex.position.length());

        // reset current texture coord
        mTexCoordIndex = 0;

        mTempVertexPending = true;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::normal(const Vector3& norm)
    {
        normal(norm.x, norm.y, norm.z);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::normal(float x, float y, float z)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::normal");
        }
        if (mFirstVertex && !mCurrentUpdating)
        {
            // defining declaration
            mDeclSize += mCurrentSection->getRenderOperation()->vertexData->vertexDeclaration
                ->addElement(0, mDeclSize, VET_FLOAT3, VES_NORMAL).getSize();
        }
        mTempVertex.normal.x = x;
        mTempVertex.normal.y = y;
        mTempVertex.normal.z = z;
    }

    //-----------------------------------------------------------------------------
    void ManualObject::tangent(const Vector3& tan)
    {
        tangent(tan.x, tan.y, tan.z);
    }
    //-----------------------------------------------------------------------------
    void ManualObject::tangent(float x, float y, float z)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::tangent");
        }
        if (mFirstVertex && !mCurrentUpdating)
        {
            // defining declaration
            mDeclSize += mCurrentSection->getRenderOperation()->vertexData->vertexDeclaration
                ->addElement(0, mDeclSize, VET_FLOAT3, VES_TANGENT).getSize();
        }
        mTempVertex.tangent.x = x;
        mTempVertex.tangent.y = y;
        mTempVertex.tangent.z = z;
    }

    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(float u)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::textureCoord");
        }
        if (mFirstVertex && !mCurrentUpdating)
        {
            // defining declaration
            mDeclSize += mCurrentSection->getRenderOperation()->vertexData->vertexDeclaration
                ->addElement(0, mDeclSize, VET_FLOAT1, VES_TEXTURE_COORDINATES, mTexCoordIndex).getSize();
        }
        mTempVertex.texCoordDims[mTexCoordIndex] = 1;
        mTempVertex.texCoord[mTexCoordIndex].x = u;

        ++mTexCoordIndex;

    }
    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(float u, float v)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::textureCoord");
        }
        if (mFirstVertex && !mCurrentUpdating)
        {
            // defining declaration
            mDeclSize += mCurrentSection->getRenderOperation()->vertexData->vertexDeclaration
                ->addElement(0, mDeclSize, VET_FLOAT2, VES_TEXTURE_COORDINATES, mTexCoordIndex).getSize();
        }
        mTempVertex.texCoordDims[mTexCoordIndex] = 2;
        mTempVertex.texCoord[mTexCoordIndex].x = u;
        mTempVertex.texCoord[mTexCoordIndex].y = v;

        ++mTexCoordIndex;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(float u, float v, float w)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::textureCoord");
        }
        if (mFirstVertex && !mCurrentUpdating)
        {
            // defining declaration
            mDeclSize += mCurrentSection->getRenderOperation()->vertexData->vertexDeclaration
                ->addElement(0, mDeclSize, VET_FLOAT3, VES_TEXTURE_COORDINATES, mTexCoordIndex).getSize();
        }
        mTempVertex.texCoordDims[mTexCoordIndex] = 3;
        mTempVertex.texCoord[mTexCoordIndex].x = u;
        mTempVertex.texCoord[mTexCoordIndex].y = v;
        mTempVertex.texCoord[mTexCoordIndex].z = w;

        ++mTexCoordIndex;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::textureCoord(float x, float y, float z, float w)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::textureCoord");
        }
        if (mFirstVertex && !mCurrentUpdating)
        {
            // defining declaration
            mDeclSize += mCurrentSection->getRenderOperation()->vertexData->vertexDeclaration
                ->addElement(0, mDeclSize, VET_FLOAT4, VES_TEXTURE_COORDINATES, mTexCoordIndex).getSize();
        }
        mTempVertex.texCoordDims[mTexCoordIndex] = 4;
        mTempVertex.texCoord[mTexCoordIndex].x = x;
        mTempVertex.texCoord[mTexCoordIndex].y = y;
        mTempVertex.texCoord[mTexCoordIndex].z = z;
        mTempVertex.texCoord[mTexCoordIndex].w = w;

        ++mTexCoordIndex;
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
    void ManualObject::colour(float r, float g, float b, float a)
    {
        if (!mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must call begin() before this method",
                "ManualObject::colour");
        }
        if (mFirstVertex && !mCurrentUpdating)
        {
            // defining declaration
            mDeclSize += mCurrentSection->getRenderOperation()->vertexData->vertexDeclaration
                ->addElement(0, mDeclSize, VET_COLOUR, VES_DIFFUSE).getSize();
        }
        mTempVertex.colour.r = r;
        mTempVertex.colour.g = g;
        mTempVertex.colour.b = b;
        mTempVertex.colour.a = a;

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
        mAnyIndexed = true;
        if (idx >= 65536)
            mCurrentSection->set32BitIndices(true);

        // make sure we have index data
        RenderOperation* rop = mCurrentSection->getRenderOperation();
        if (!rop->indexData)
        {
            rop->indexData = OGRE_NEW IndexData();
            rop->indexData->indexCount = 0;
        }
        rop->useIndexes = true;
        resizeTempIndexBufferIfNeeded(++rop->indexData->indexCount);

        mTempIndexBuffer[rop->indexData->indexCount - 1] = idx;
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
        if (mCurrentSection->getRenderOperation()->operationType !=
            RenderOperation::OT_TRIANGLE_LIST)
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
        
        RenderOperation* rop = mCurrentSection->getRenderOperation();

        // There's an unfinished vertex being defined, so include it in count
        if (mTempVertexPending)
            return rop->vertexData->vertexCount + 1;
        else
            return rop->vertexData->vertexCount;
        
    }
    //-----------------------------------------------------------------------------
    size_t ManualObject::getCurrentIndexCount() const
    {
        if (!mCurrentSection)
            return 0;

        RenderOperation* rop = mCurrentSection->getRenderOperation();
        if (rop->indexData)
            return rop->indexData->indexCount;
        else
            return 0;

    }
    //-----------------------------------------------------------------------------
    void ManualObject::copyTempVertexToBuffer(void)
    {
        mTempVertexPending = false;
        RenderOperation* rop = mCurrentSection->getRenderOperation();
        if (rop->vertexData->vertexCount == 0 && !mCurrentUpdating)
        {
            // first vertex, autoorganise decl
            VertexDeclaration* oldDcl = rop->vertexData->vertexDeclaration;
            rop->vertexData->vertexDeclaration =
                oldDcl->getAutoOrganisedDeclaration(false, false, false);
            HardwareBufferManager::getSingleton().destroyVertexDeclaration(oldDcl);
        }
        resizeTempVertexBufferIfNeeded(++rop->vertexData->vertexCount);

        // get base pointer
        char* pBase = mTempVertexBuffer + (mDeclSize * (rop->vertexData->vertexCount-1));
        const VertexDeclaration::VertexElementList& elemList =
            rop->vertexData->vertexDeclaration->getElements();
        for (VertexDeclaration::VertexElementList::const_iterator i = elemList.begin();
            i != elemList.end(); ++i)
        {
            float* pFloat = 0;
            RGBA* pRGBA = 0;
            const VertexElement& elem = *i;
            switch(elem.getType())
            {
            case VET_FLOAT1:
            case VET_FLOAT2:
            case VET_FLOAT3:
            case VET_FLOAT4:
                OgreAssert(elem.getSemantic() != VES_DIFFUSE, "must use VET_COLOUR");
                elem.baseVertexPointerToElement(pBase, &pFloat);
                break;
            case VET_COLOUR:
            case VET_COLOUR_ABGR:
            case VET_COLOUR_ARGB:
                OgreAssert(elem.getSemantic() == VES_DIFFUSE, "must use VES_DIFFUSE");
                elem.baseVertexPointerToElement(pBase, &pRGBA);
                break;
            default:
                OgreAssert(false, "invalid element type");
                break;
            };


            RenderSystem* rs;
            unsigned short dims;
            switch(elem.getSemantic())
            {
            case VES_POSITION:
                *pFloat++ = mTempVertex.position.x;
                *pFloat++ = mTempVertex.position.y;
                *pFloat++ = mTempVertex.position.z;
                break;
            case VES_NORMAL:
                *pFloat++ = mTempVertex.normal.x;
                *pFloat++ = mTempVertex.normal.y;
                *pFloat++ = mTempVertex.normal.z;
                break;
            case VES_TANGENT:
                *pFloat++ = mTempVertex.tangent.x;
                *pFloat++ = mTempVertex.tangent.y;
                *pFloat++ = mTempVertex.tangent.z;
                break;
            case VES_TEXTURE_COORDINATES:
                dims = VertexElement::getTypeCount(elem.getType());
                for (ushort t = 0; t < dims; ++t)
                    *pFloat++ = mTempVertex.texCoord[elem.getIndex()][t];
                break;
            case VES_DIFFUSE:
                rs = Root::getSingleton().getRenderSystem();
                if (rs)
                {
                    rs->convertColourValue(mTempVertex.colour, pRGBA++);
                }
                else
                {
                    switch(elem.getType())
                    {
                        case VET_COLOUR_ABGR:
                            *pRGBA++ = mTempVertex.colour.getAsABGR();
                            break;
                        case VET_COLOUR_ARGB:
                            *pRGBA++ = mTempVertex.colour.getAsARGB();
                            break;
                        default:
                            *pRGBA++ = mTempVertex.colour.getAsRGBA();
                    }
                }
                break;
            default:
                OgreAssert(false, "invalid semantic");
                break;
            };

        }

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
        if (mTempVertexPending)
        {
            // bake current vertex
            copyTempVertexToBuffer();
        }

        // pointer that will be returned
        ManualObjectSection* result = NULL;

        RenderOperation* rop = mCurrentSection->getRenderOperation();
        // Check for empty content
        if (rop->vertexData->vertexCount == 0 ||
            (rop->useIndexes && rop->indexData->indexCount == 0))
        {
            // You're wasting my time sonny
            if (mCurrentUpdating)
            {
                // Can't just undo / remove since may be in the middle
                // Just allow counts to be 0, will not be issued to renderer

                // return the finished section (though it has zero vertices)
                result = mCurrentSection;
            }
            else
            {
                // First creation, can really undo
                // Has already been added to section list end, so remove
                mSectionList.pop_back();
                OGRE_DELETE mCurrentSection;

            }
        }
        else // not an empty section
        {

            // Bake the real buffers
            HardwareVertexBufferSharedPtr vbuf;
            // Check buffer sizes
            bool vbufNeedsCreating = true;
            bool ibufNeedsCreating = rop->useIndexes;
            // Work out if we require 16 or 32-bit index buffers
            HardwareIndexBuffer::IndexType indexType = mCurrentSection->get32BitIndices()?  
                HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT;
            if (mCurrentUpdating)
            {
                // May be able to reuse buffers, check sizes
                vbuf = rop->vertexData->vertexBufferBinding->getBuffer(0);
                if (vbuf->getNumVertices() >= rop->vertexData->vertexCount)
                    vbufNeedsCreating = false;

                if (rop->useIndexes)
                {
                    if ((rop->indexData->indexBuffer->getNumIndexes() >= rop->indexData->indexCount) &&
                        (indexType == rop->indexData->indexBuffer->getType()))
                        ibufNeedsCreating = false;
                }

            }
            if (vbufNeedsCreating)
            {
                // Make the vertex buffer larger if estimated vertex count higher
                // to allow for user-configured growth area
                size_t vertexCount = std::max(rop->vertexData->vertexCount, 
                    mEstVertexCount);
                vbuf =
                    HardwareBufferManager::getSingleton().createVertexBuffer(
                        mDeclSize,
                        vertexCount,
                        mDynamic? HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY : 
                            HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                rop->vertexData->vertexBufferBinding->setBinding(0, vbuf);
            }
            if (ibufNeedsCreating)
            {
                // Make the index buffer larger if estimated index count higher
                // to allow for user-configured growth area
                size_t indexCount = std::max(rop->indexData->indexCount, 
                    mEstIndexCount);
                rop->indexData->indexBuffer =
                    HardwareBufferManager::getSingleton().createIndexBuffer(
                        indexType,
                        indexCount,
                        mDynamic? HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY : 
                            HardwareBuffer::HBU_STATIC_WRITE_ONLY);
            }
            // Write vertex data
            vbuf->writeData(
                0, rop->vertexData->vertexCount * vbuf->getVertexSize(), 
                mTempVertexBuffer, true);
            // Write index data
            if(rop->useIndexes)
            {
                if (HardwareIndexBuffer::IT_32BIT == indexType)
                {
                    // direct copy from the mTempIndexBuffer
                    rop->indexData->indexBuffer->writeData(
                        0, 
                        rop->indexData->indexCount 
                            * rop->indexData->indexBuffer->getIndexSize(),
                        mTempIndexBuffer, true);
                }
                else //(HardwareIndexBuffer::IT_16BIT == indexType)
                {
                    HardwareBufferLockGuard indexLock(rop->indexData->indexBuffer, HardwareBuffer::HBL_DISCARD);
                    uint16* pIdx = static_cast<uint16*>(indexLock.pData);
                    uint32* pSrc = mTempIndexBuffer;
                    for (size_t i = 0; i < rop->indexData->indexCount; i++)
                    {
                        *pIdx++ = static_cast<uint16>(*pSrc++);
                    }
                }
            }

            // return the finished section
            result = mCurrentSection;

        } // empty section check

        mCurrentSection = 0;
        resetTempAreas();

        // Tell parent if present
        if (mParentNode)
        {
            mParentNode->needUpdate();
        }

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
    //-----------------------------------------------------------------------------
    void ManualObject::setMaterial(size_t subIndex, const MaterialPtr &mat)
    {
        if (subIndex >= mSectionList.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Index out of bounds!",
                "ManualObject::setMaterial");
        }

        mSectionList[subIndex]->setMaterial(mat);
    }
    //-----------------------------------------------------------------------------
    MeshPtr ManualObject::convertToMesh(const String& meshName, const String& groupName)
    {
        if (mCurrentSection)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You cannot call convertToMesh() whilst you are in the middle of "
                "defining the object; call end() first.",
                "ManualObject::convertToMesh");
        }
        if (mSectionList.empty())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "No data defined to convert to a mesh.",
                "ManualObject::convertToMesh");
        }
        MeshPtr m = MeshManager::getSingleton().createManual(meshName, groupName);

        for (auto sec : mSectionList)
        {
            SubMesh* sm = m->createSubMesh();
            sec->convertToSubMesh(sm);
            sm->setMaterial(sec->getMaterial());
        }
        // update bounds
        m->_setBounds(mAABB);
        m->_setBoundingSphereRadius(mRadius);

        m->load();

        return m;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::setUseIdentityProjection(bool useIdentityProjection)
    {
        // Set existing
        for (SectionList::iterator i = mSectionList.begin(); i != mSectionList.end(); ++i)
        {
            (*i)->setUseIdentityProjection(useIdentityProjection);
        }
        
        // Save setting for future sections
        mUseIdentityProjection = useIdentityProjection;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::setUseIdentityView(bool useIdentityView)
    {
        // Set existing
        for (SectionList::iterator i = mSectionList.begin(); i != mSectionList.end(); ++i)
        {
            (*i)->setUseIdentityView(useIdentityView);
        }

        // Save setting for future sections
        mUseIdentityView = useIdentityView;
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
    const AxisAlignedBox& ManualObject::getBoundingBox(void) const
    {
        return mAABB;
    }
    //-----------------------------------------------------------------------------
    Real ManualObject::getBoundingRadius(void) const
    {
        return mRadius;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::_updateRenderQueue(RenderQueue* queue)
    {
        // To be used when order of creation must be kept while rendering
        unsigned short priority = queue->getDefaultRenderablePriority();

        for (SectionList::iterator i = mSectionList.begin(); i != mSectionList.end(); ++i)
        {
            // Skip empty sections (only happens if non-empty first, then updated)
            RenderOperation* rop = (*i)->getRenderOperation();
            if (rop->vertexData->vertexCount == 0 ||
                (rop->useIndexes && rop->indexData->indexCount == 0))
                continue;
            
            if (mRenderQueuePrioritySet)
            {
                assert(mRenderQueueIDSet == true);
                queue->addRenderable(*i, mRenderQueueID, mRenderQueuePriority);
            }
            else if (mRenderQueueIDSet)
                queue->addRenderable(*i, mRenderQueueID, mKeepDeclarationOrder ? priority++ : queue->getDefaultRenderablePriority());
            else
                queue->addRenderable(*i, queue->getDefaultQueueGroup(), mKeepDeclarationOrder ? priority++ : queue->getDefaultRenderablePriority());
        }
    }
    //-----------------------------------------------------------------------------
    void ManualObject::visitRenderables(Renderable::Visitor* visitor, 
        bool debugRenderables)
    {
        for (SectionList::iterator i = mSectionList.begin(); i != mSectionList.end(); ++i)
        {
            visitor->visit(*i, 0, false);
        }

    }
    //-----------------------------------------------------------------------------
    EdgeData* ManualObject::getEdgeList(void)
    {
        // Build on demand
        if (!mEdgeList && mAnyIndexed)
        {
            EdgeListBuilder eb;
            size_t vertexSet = 0;
            bool anyBuilt = false;
            for (SectionList::iterator i = mSectionList.begin(); i != mSectionList.end(); ++i)
            {
                RenderOperation* rop = (*i)->getRenderOperation();
                // Only indexed triangle geometry supported for stencil shadows
                if (rop->useIndexes && rop->indexData->indexCount != 0 && 
                    (rop->operationType == RenderOperation::OT_TRIANGLE_FAN ||
                     rop->operationType == RenderOperation::OT_TRIANGLE_LIST ||
                     rop->operationType == RenderOperation::OT_TRIANGLE_STRIP))
                {
                    eb.addVertexData(rop->vertexData);
                    eb.addIndexData(rop->indexData, vertexSet++);
                    anyBuilt = true;
                }
            }

            if (anyBuilt)
                mEdgeList = eb.build();

        }
        return mEdgeList;
    }
    //---------------------------------------------------------------------
    bool ManualObject::hasEdgeList()
    {
        return getEdgeList() != 0;
    }
    //-----------------------------------------------------------------------------
    const ShadowCaster::ShadowRenderableList&
    ManualObject::getShadowVolumeRenderableList(
        ShadowTechnique shadowTechnique, const Light* light,
        HardwareIndexBufferSharedPtr* indexBuffer, size_t* indexBufferUsedSize,
        bool extrude, Real extrusionDistance, unsigned long flags)
    {
        assert(indexBuffer && "Only external index buffers are supported right now");       

        EdgeData* edgeList = getEdgeList();
        if (!edgeList)
        {
            return mShadowRenderables;
        }

        // Calculate the object space light details
        Vector4 lightPos = light->getAs4DVector();
        Affine3 world2Obj = mParentNode->_getFullTransform().inverse();
        lightPos = world2Obj * lightPos;
        Matrix3 world2Obj3x3 = world2Obj.linear();
        extrusionDistance *= Math::Sqrt(std::min(std::min(world2Obj3x3.GetColumn(0).squaredLength(), world2Obj3x3.GetColumn(1).squaredLength()), world2Obj3x3.GetColumn(2).squaredLength()));

        // Init shadow renderable list if required (only allow indexed)
        bool init = mShadowRenderables.empty() && mAnyIndexed;

        EdgeData::EdgeGroupList::iterator egi;
        ShadowRenderableList::iterator si, siend;
        ManualObjectSectionShadowRenderable* esr = 0;
        SectionList::iterator seci;
        if (init)
            mShadowRenderables.resize(edgeList->edgeGroups.size());

        siend = mShadowRenderables.end();
        egi = edgeList->edgeGroups.begin();
        seci = mSectionList.begin();
        for (si = mShadowRenderables.begin(); si != siend; ++seci)
        {
            // Skip non-indexed geometry
            if (!(*seci)->getRenderOperation()->useIndexes)
            {
                continue;
            }

            if (init)
            {
                // Create a new renderable, create a separate light cap if
                // we're using a vertex program (either for this model, or
                // for extruding the shadow volume) since otherwise we can
                // get depth-fighting on the light cap
                MaterialPtr mat = (*seci)->getMaterial();
                mat->load();
                bool vertexProgram = false;
                Technique* t = mat->getBestTechnique(0, *seci);
                for (unsigned short p = 0; p < t->getNumPasses(); ++p)
                {
                    Pass* pass = t->getPass(p);
                    if (pass->hasVertexProgram())
                    {
                        vertexProgram = true;
                        break;
                    }
                }
                *si = OGRE_NEW ManualObjectSectionShadowRenderable(this, indexBuffer,
                    egi->vertexData, vertexProgram || !extrude);
            }
            // Get shadow renderable
            esr = static_cast<ManualObjectSectionShadowRenderable*>(*si);
            HardwareVertexBufferSharedPtr esrPositionBuffer = esr->getPositionBuffer();
            // Extrude vertices in software if required
            if (extrude)
            {
                extrudeVertices(esrPositionBuffer,
                    egi->vertexData->vertexCount,
                    lightPos, extrusionDistance);

            }

            ++si;
            ++egi;
        }
        // Calc triangle light facing
        updateEdgeListLightFacing(edgeList, lightPos);

        // Generate indexes and update renderables
        generateShadowVolume(edgeList, *indexBuffer, *indexBufferUsedSize, 
            light, mShadowRenderables, flags);

        return mShadowRenderables;
    }
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    ManualObject::ManualObjectSection::ManualObjectSection(ManualObject* parent,
        const String& materialName, RenderOperation::OperationType opType, const String & groupName)
        : mParent(parent), mMaterialName(materialName), mGroupName(groupName), m32BitIndices(false)
    {
        mRenderOperation.operationType = opType;
        // default to no indexes unless we're told
        mRenderOperation.useIndexes = false;
        mRenderOperation.useGlobalInstancingVertexBufferIsAvailable = false;
        mRenderOperation.vertexData = OGRE_NEW VertexData();
        mRenderOperation.vertexData->vertexCount = 0;
    }
    ManualObject::ManualObjectSection::ManualObjectSection(ManualObject* parent,
        const MaterialPtr& mat, RenderOperation::OperationType opType)
        : mParent(parent), mMaterial(mat), m32BitIndices(false)
    {
        assert(mMaterial);
        mMaterialName = mMaterial->getName();
        mGroupName = mMaterial->getGroup();

        mRenderOperation.operationType = opType;
        mRenderOperation.useIndexes = false;
        mRenderOperation.useGlobalInstancingVertexBufferIsAvailable = false;
        mRenderOperation.vertexData = OGRE_NEW VertexData();
        mRenderOperation.vertexData->vertexCount = 0;
    }
    //-----------------------------------------------------------------------------
    ManualObject::ManualObjectSection::~ManualObjectSection()
    {
        OGRE_DELETE mRenderOperation.vertexData;
        OGRE_DELETE mRenderOperation.indexData; // ok to delete 0
    }
    //-----------------------------------------------------------------------------
    RenderOperation* ManualObject::ManualObjectSection::getRenderOperation(void)
    {
        return &mRenderOperation;
    }
    //-----------------------------------------------------------------------------
    const MaterialPtr& ManualObject::ManualObjectSection::getMaterial(void) const
    {
        if (!mMaterial)
        {
            mMaterial = static_pointer_cast<Material>(MaterialManager::getSingleton().load(mMaterialName, mGroupName));
        }
        return mMaterial;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::setMaterialName(const String& name,
        const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
        if (mMaterialName != name || mGroupName != groupName)
        {
            mMaterialName = name;
            mGroupName = groupName;
            mMaterial.reset();
        }
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::setMaterial(const MaterialPtr& mat)
    {
        assert(mat);
        mMaterial = mat;
        mMaterialName = mat->getName();
        mGroupName = mat->getGroup();
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::getRenderOperation(RenderOperation& op)
    {
        // direct copy
        op = mRenderOperation;
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::getWorldTransforms(Matrix4* xform) const
    {
        xform[0] = mParent->_getParentNodeFullTransform();
    }
    //-----------------------------------------------------------------------------
    Real ManualObject::ManualObjectSection::getSquaredViewDepth(const Ogre::Camera *cam) const
    {
        Node* n = mParent->getParentNode();
        assert(n);
        return n->getSquaredViewDepth(cam);
    }
    //-----------------------------------------------------------------------------
    const LightList& ManualObject::ManualObjectSection::getLights(void) const
    {
        return mParent->queryLights();
    }
    //-----------------------------------------------------------------------------
    void ManualObject::ManualObjectSection::convertToSubMesh(SubMesh* sm) const
    {
        sm->useSharedVertices = false;
        sm->operationType = mRenderOperation.operationType;
        // Copy vertex data; replicate buffers too
        sm->vertexData = mRenderOperation.vertexData->clone(true);

        // Copy index data; replicate buffers too; delete the default, old one to avoid memory leaks

        // check if index data is present
        if (mRenderOperation.indexData)
        {
            // Copy index data; replicate buffers too; delete the default, old one to avoid memory leaks
            OGRE_DELETE sm->indexData;
            sm->indexData = mRenderOperation.indexData->clone(true);
        }
    }
    //--------------------------------------------------------------------------
    ManualObject::ManualObjectSectionShadowRenderable::ManualObjectSectionShadowRenderable(
        ManualObject* parent, HardwareIndexBufferSharedPtr* indexBuffer,
        const VertexData* vertexData, bool createSeparateLightCap,
        bool isLightCap)
        : mParent(parent)
    {
        // Initialise render op
        mRenderOp.indexData = OGRE_NEW IndexData();
        mRenderOp.indexData->indexBuffer = *indexBuffer;
        mRenderOp.indexData->indexStart = 0;
        // index start and count are sorted out later

        // Create vertex data which just references position component (and 2 component)
        mRenderOp.vertexData = OGRE_NEW VertexData();
        // Map in position data
        mRenderOp.vertexData->vertexDeclaration->addElement(0,0,VET_FLOAT3, VES_POSITION);
        ushort origPosBind =
            vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION)->getSource();
        mPositionBuffer = vertexData->vertexBufferBinding->getBuffer(origPosBind);
        mRenderOp.vertexData->vertexBufferBinding->setBinding(0, mPositionBuffer);
        // Map in w-coord buffer (if present)
        if(vertexData->hardwareShadowVolWBuffer)
        {
            mRenderOp.vertexData->vertexDeclaration->addElement(1,0,VET_FLOAT1, VES_TEXTURE_COORDINATES, 0);
            mWBuffer = vertexData->hardwareShadowVolWBuffer;
            mRenderOp.vertexData->vertexBufferBinding->setBinding(1, mWBuffer);
        }
        // Use same vertex start as input
        mRenderOp.vertexData->vertexStart = vertexData->vertexStart;

        if (isLightCap)
        {
            // Use original vertex count, no extrusion
            mRenderOp.vertexData->vertexCount = vertexData->vertexCount;
        }
        else
        {
            // Vertex count must take into account the doubling of the buffer,
            // because second half of the buffer is the extruded copy
            mRenderOp.vertexData->vertexCount =
                vertexData->vertexCount * 2;
            if (createSeparateLightCap)
            {
                // Create child light cap
                mLightCap = OGRE_NEW ManualObjectSectionShadowRenderable(parent,
                    indexBuffer, vertexData, false, true);
            }
        }
    }
    //--------------------------------------------------------------------------
    ManualObject::ManualObjectSectionShadowRenderable::~ManualObjectSectionShadowRenderable()
    {
        OGRE_DELETE mRenderOp.indexData;
        OGRE_DELETE mRenderOp.vertexData;
    }
    //--------------------------------------------------------------------------
    void ManualObject::ManualObjectSectionShadowRenderable::getWorldTransforms(
        Matrix4* xform) const
    {
        // pretransformed
        *xform = mParent->_getParentNodeFullTransform();
    }
    //-----------------------------------------------------------------------
    void ManualObject::ManualObjectSectionShadowRenderable::rebindIndexBuffer(const HardwareIndexBufferSharedPtr& indexBuffer)
    {
        mRenderOp.indexData->indexBuffer = indexBuffer;
        if (mLightCap) mLightCap->rebindIndexBuffer(indexBuffer);
    }
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    String ManualObjectFactory::FACTORY_TYPE_NAME = "ManualObject";
    //-----------------------------------------------------------------------------
    const String& ManualObjectFactory::getType(void) const
    {
        return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------------
    MovableObject* ManualObjectFactory::createInstanceImpl(
        const String& name, const NameValuePairList* params)
    {
        return OGRE_NEW ManualObject(name);
    }
    //-----------------------------------------------------------------------------
    void ManualObjectFactory::destroyInstance( MovableObject* obj)
    {
        OGRE_DELETE obj;
    }



}
