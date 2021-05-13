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

// Thanks to Vincent Cantin (karmaGfa) for the original implementation of this
// class, although it has now been mostly rewritten

#include "OgreStableHeaders.h"
#include "OgreBillboardChain.h"
#include "OgreViewport.h"

#include <limits>

namespace Ogre {
    const size_t BillboardChain::SEGMENT_EMPTY = std::numeric_limits<size_t>::max();
    //-----------------------------------------------------------------------
    BillboardChain::Element::Element()
    {
    }
    //-----------------------------------------------------------------------
    BillboardChain::Element::Element(const Vector3 &_position,
        Real _width,
        Real _texCoord,
        const ColourValue &_colour,
        const Quaternion &_orientation) :
    position(_position),
        width(_width),
        texCoord(_texCoord),
        colour(_colour),
        orientation(_orientation)
    {
    }
    //-----------------------------------------------------------------------
    BillboardChain::BillboardChain(const String& name, size_t maxElements,
        size_t numberOfChains, bool useTextureCoords, bool useColours, bool dynamic)
        :MovableObject(name),
        mMaxElementsPerChain(maxElements),
        mChainCount(numberOfChains),
        mUseTexCoords(useTextureCoords),
        mUseVertexColour(useColours),
        mDynamic(dynamic),
        mVertexDeclDirty(true),
        mBuffersNeedRecreating(true),
        mBoundsDirty(true),
        mIndexContentDirty(true),
        mVertexContentDirty(true),
        mRadius(0.0f),
        mTexCoordDir(TCD_U),
        mVertexCameraUsed(0),
        mFaceCamera(true),
        mNormalBase(Vector3::UNIT_X)
    {
        mVertexData.reset(new VertexData());
        mIndexData.reset(new IndexData());

        mOtherTexCoordRange[0] = 0.0f;
        mOtherTexCoordRange[1] = 1.0f;

        setupChainContainers();

        mVertexData->vertexStart = 0;
        // index data set up later
        // set basic white material
        mMaterial = MaterialManager::getSingleton().getDefaultMaterial(false);
        mMaterial->load();
    }

    BillboardChain::~BillboardChain() = default; // ensure unique_ptr destructors are in cpp

    //-----------------------------------------------------------------------
    void BillboardChain::setupChainContainers(void)
    {
        // Allocate enough space for everything
        mChainElementList.resize(mChainCount * mMaxElementsPerChain);
        mVertexData->vertexCount = mChainElementList.size() * 2;

        // Configure chains
        mChainSegmentList.resize(mChainCount);
        for (size_t i = 0; i < mChainCount; ++i)
        {
            ChainSegment& seg = mChainSegmentList[i];
            seg.start = i * mMaxElementsPerChain;
            seg.tail = seg.head = SEGMENT_EMPTY;

        }


    }
    //-----------------------------------------------------------------------
    void BillboardChain::setupVertexDeclaration(void)
    {
        if (mVertexDeclDirty)
        {
            VertexDeclaration* decl = mVertexData->vertexDeclaration;
            decl->removeAllElements();

            size_t offset = 0;
            // Add a description for the buffer of the positions of the vertices
            offset += decl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();

            if (mUseVertexColour)
            {
                offset += decl->addElement(0, offset, VET_UBYTE4_NORM, VES_DIFFUSE).getSize();
            }

            if (mUseTexCoords)
            {
                decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
            }

            if (!mUseTexCoords && !mUseVertexColour)
            {
                LogManager::getSingleton().logError(
                    "BillboardChain '" + mName + "' is using neither "
                    "texture coordinates nor vertex colours; it will not be "
                    "visible on some rendering APIs so you should change this "
                    "so you use one or the other.");
            }
            mVertexDeclDirty = false;
        }
    }
    //-----------------------------------------------------------------------
    void BillboardChain::setupBuffers(void)
    {
        setupVertexDeclaration();
        if (mBuffersNeedRecreating)
        {
            // Create the vertex buffer (always dynamic due to the camera adjust)
            HardwareVertexBufferSharedPtr pBuffer =
                HardwareBufferManager::getSingleton().createVertexBuffer(
                mVertexData->vertexDeclaration->getVertexSize(0),
                mVertexData->vertexCount,
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

            // (re)Bind the buffer
            // Any existing buffer will lose its reference count and be destroyed
            mVertexData->vertexBufferBinding->setBinding(0, pBuffer);

            mIndexData->indexBuffer =
                HardwareBufferManager::getSingleton().createIndexBuffer(
                    HardwareIndexBuffer::IT_16BIT,
                    mChainCount * mMaxElementsPerChain * 6, // max we can use
                    mDynamic? HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY : HardwareBuffer::HBU_STATIC_WRITE_ONLY);
            // NB we don't set the indexCount on IndexData here since we will
            // probably use less than the maximum number of indices

            mBuffersNeedRecreating = false;
        }
    }
    //-----------------------------------------------------------------------
    void BillboardChain::setMaxChainElements(size_t maxElements)
    {
        mMaxElementsPerChain = maxElements;
        setupChainContainers();
        mBuffersNeedRecreating = mIndexContentDirty = mVertexContentDirty = true;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::setNumberOfChains(size_t numChains)
    {
        mChainCount = numChains;
        setupChainContainers();
        mBuffersNeedRecreating = mIndexContentDirty = mVertexContentDirty = true;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::setUseTextureCoords(bool use)
    {
        mUseTexCoords = use;
        mVertexDeclDirty = mBuffersNeedRecreating = true;
        mIndexContentDirty = mVertexContentDirty = true;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::setTextureCoordDirection(BillboardChain::TexCoordDirection dir)
    {
        mTexCoordDir = dir;
        mVertexContentDirty = true;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::setOtherTextureCoordRange(Real start, Real end)
    {
        mOtherTexCoordRange[0] = start;
        mOtherTexCoordRange[1] = end;
        mVertexContentDirty = true;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::setUseVertexColours(bool use)
    {
        mUseVertexColour = use;
        mVertexDeclDirty = mBuffersNeedRecreating = true;
        mIndexContentDirty = mVertexContentDirty = true;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::setDynamic(bool dyn)
    {
        mDynamic = dyn;
        mBuffersNeedRecreating = mIndexContentDirty = mVertexContentDirty = true;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::addChainElement(size_t chainIndex,
        const BillboardChain::Element& dtls)
    {
        ChainSegment& seg = mChainSegmentList.at(chainIndex);
        if (seg.head == SEGMENT_EMPTY)
        {
            // Tail starts at end, head grows backwards
            seg.tail = mMaxElementsPerChain - 1;
            seg.head = seg.tail;
        }
        else
        {
            if (seg.head == 0)
            {
                // Wrap backwards
                seg.head = mMaxElementsPerChain - 1;
            }
            else
            {
                // Just step backward
                --seg.head;
            }
            // Run out of elements?
            if (seg.head == seg.tail)
            {
                // Move tail backwards too, losing the end of the segment and re-using
                // it in the head
                if (seg.tail == 0)
                    seg.tail = mMaxElementsPerChain - 1;
                else
                    --seg.tail;
            }
        }

        // Set the details
        mChainElementList[seg.start + seg.head] = dtls;

        mVertexContentDirty = true;
        mIndexContentDirty = true;
        mBoundsDirty = true;
        // tell parent node to update bounds
        if (mParentNode)
            mParentNode->needUpdate();

    }
    //-----------------------------------------------------------------------
    void BillboardChain::removeChainElement(size_t chainIndex)
    {
        ChainSegment& seg = mChainSegmentList.at(chainIndex);
        if (seg.head == SEGMENT_EMPTY)
            return; // do nothing, nothing to remove


        if (seg.tail == seg.head)
        {
            // last item
            seg.head = seg.tail = SEGMENT_EMPTY;
        }
        else if (seg.tail == 0)
        {
            seg.tail = mMaxElementsPerChain - 1;
        }
        else
        {
            --seg.tail;
        }

        // we removed an entry so indexes need updating
        mVertexContentDirty = true;
        mIndexContentDirty = true;
        mBoundsDirty = true;
        // tell parent node to update bounds
        if (mParentNode)
            mParentNode->needUpdate();

    }
    //-----------------------------------------------------------------------
    void BillboardChain::clearChain(size_t chainIndex)
    {
        ChainSegment& seg = mChainSegmentList.at(chainIndex);

        // Just reset head & tail
        seg.tail = seg.head = SEGMENT_EMPTY;

        // we removed an entry so indexes need updating
        mVertexContentDirty = true;
        mIndexContentDirty = true;
        mBoundsDirty = true;
        // tell parent node to update bounds
        if (mParentNode)
            mParentNode->needUpdate();

    }
    //-----------------------------------------------------------------------
    void BillboardChain::clearAllChains(void)
    {
        for (size_t i = 0; i < mChainCount; ++i)
        {
            clearChain(i);
        }

    }
    //-----------------------------------------------------------------------
    void BillboardChain::setFaceCamera( bool faceCamera, const Vector3 &normalVector )
    {
        mFaceCamera = faceCamera;
        mNormalBase = normalVector.normalisedCopy();
        mVertexContentDirty = true;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::updateChainElement(size_t chainIndex, size_t elementIndex,
        const BillboardChain::Element& dtls)
    {
        ChainSegment& seg = mChainSegmentList.at(chainIndex);
        OgreAssert(seg.head != SEGMENT_EMPTY, "Chain segment is empty");

        size_t idx = seg.head + elementIndex;
        // adjust for the edge and start
        idx = (idx % mMaxElementsPerChain) + seg.start;

        mChainElementList[idx] = dtls;

        mVertexContentDirty = true;
        mBoundsDirty = true;
        // tell parent node to update bounds
        if (mParentNode)
            mParentNode->needUpdate();


    }
    //-----------------------------------------------------------------------
    const BillboardChain::Element&
    BillboardChain::getChainElement(size_t chainIndex, size_t elementIndex) const
    {
        const ChainSegment& seg = mChainSegmentList.at(chainIndex);
        OgreAssert(seg.head != SEGMENT_EMPTY, "Chain segment is empty");

        size_t idx = seg.head + elementIndex;
        // adjust for the edge and start
        idx = (idx % mMaxElementsPerChain) + seg.start;

        return mChainElementList[idx];
    }
    //-----------------------------------------------------------------------
    size_t BillboardChain::getNumChainElements(size_t chainIndex) const
    {
        const ChainSegment& seg = mChainSegmentList.at(chainIndex);
        
        if (seg.head == SEGMENT_EMPTY)
        {
            return 0;
        }
        else if (seg.tail < seg.head)
        {
            return seg.tail - seg.head + mMaxElementsPerChain + 1;
        }
        else
        {
            return seg.tail - seg.head + 1;
        }
    }
    //-----------------------------------------------------------------------
    void BillboardChain::updateBoundingBox(void) const
    {
        if (mBoundsDirty)
        {
            mAABB.setNull();
            Vector3 widthVector;
            for (ChainSegmentList::const_iterator segi = mChainSegmentList.begin();
                segi != mChainSegmentList.end(); ++segi)
            {
                const ChainSegment& seg = *segi;

                if (seg.head != SEGMENT_EMPTY)
                {

                    for(size_t e = seg.head; ; ++e) // until break
                    {
                        // Wrap forwards
                        if (e == mMaxElementsPerChain)
                            e = 0;

                        const Element& elem = mChainElementList[seg.start + e];

                        widthVector.x = widthVector.y = widthVector.z = elem.width;
                        mAABB.merge(elem.position - widthVector);
                        mAABB.merge(elem.position + widthVector);

                        if (e == seg.tail)
                            break;

                    }
                }

            }

            // Set the current radius
            if (mAABB.isNull())
            {
                mRadius = 0.0f;
            }
            else
            {
                mRadius = Math::Sqrt(
                    std::max(mAABB.getMinimum().squaredLength(),
                    mAABB.getMaximum().squaredLength()));
            }

            mBoundsDirty = false;
        }
    }
    //-----------------------------------------------------------------------
    void BillboardChain::updateVertexBuffer(Camera* cam)
    {
        setupBuffers();
        
        // The contents of the vertex buffer are correct if they are not dirty
        // and the camera used to build the vertex buffer is still the current 
        // camera.
        if (!mVertexContentDirty && mVertexCameraUsed == cam)
            return;

        HardwareVertexBufferSharedPtr pBuffer =
            mVertexData->vertexBufferBinding->getBuffer(0);
        HardwareBufferLockGuard vertexLock(pBuffer, HardwareBuffer::HBL_DISCARD);

        const Vector3& camPos = cam->getDerivedPosition();
        Vector3 eyePos = mParentNode->convertWorldToLocalPosition(camPos);

        Vector3 chainTangent;
        for (ChainSegmentList::iterator segi = mChainSegmentList.begin();
            segi != mChainSegmentList.end(); ++segi)
        {
            ChainSegment& seg = *segi;

            // Skip 0 or 1 element segment counts
            if (seg.head != SEGMENT_EMPTY && seg.head != seg.tail)
            {
                size_t laste = seg.head;
                for (size_t e = seg.head; ; ++e) // until break
                {
                    // Wrap forwards
                    if (e == mMaxElementsPerChain)
                        e = 0;

                    Element& elem = mChainElementList[e + seg.start];
                    assert (((e + seg.start) * 2) < 65536 && "Too many elements!");
                    uint16 baseIdx = static_cast<uint16>((e + seg.start) * 2);

                    // Determine base pointer to vertex #1
                    float* pFloat = reinterpret_cast<float*>(
                        static_cast<char*>(vertexLock.pData) +
                            pBuffer->getVertexSize() * baseIdx);

                    // Get index of next item
                    size_t nexte = e + 1;
                    if (nexte == mMaxElementsPerChain)
                        nexte = 0;

                    if (e == seg.head)
                    {
                        // No laste, use next item
                        chainTangent = mChainElementList[nexte + seg.start].position - elem.position;
                    }
                    else if (e == seg.tail)
                    {
                        // No nexte, use only last item
                        chainTangent = elem.position - mChainElementList[laste + seg.start].position;
                    }
                    else
                    {
                        // A mid position, use tangent across both prev and next
                        chainTangent = mChainElementList[nexte + seg.start].position - mChainElementList[laste + seg.start].position;

                    }

                    Vector3 vP1ToEye;

                    if( mFaceCamera )
                        vP1ToEye = eyePos - elem.position;
                    else
                        vP1ToEye = elem.orientation * mNormalBase;

                    Vector3 vPerpendicular = chainTangent.crossProduct(vP1ToEye);
                    vPerpendicular.normalise();
                    vPerpendicular *= (elem.width * 0.5f);

                    Vector3 pos0 = elem.position - vPerpendicular;
                    Vector3 pos1 = elem.position + vPerpendicular;

                    // pos1
                    *pFloat++ = pos0.x;
                    *pFloat++ = pos0.y;
                    *pFloat++ = pos0.z;

                    if (mUseVertexColour)
                    {
                        RGBA col = elem.colour.getAsBYTE();
                        memcpy(pFloat++, &col, sizeof(RGBA));
                    }

                    if (mUseTexCoords)
                    {
                        if (mTexCoordDir == TCD_U)
                        {
                            *pFloat++ = elem.texCoord;
                            *pFloat++ = mOtherTexCoordRange[0];
                        }
                        else
                        {
                            *pFloat++ = mOtherTexCoordRange[0];
                            *pFloat++ = elem.texCoord;
                        }
                    }

                    // pos2
                    *pFloat++ = pos1.x;
                    *pFloat++ = pos1.y;
                    *pFloat++ = pos1.z;

                    if (mUseVertexColour)
                    {
                        RGBA col = elem.colour.getAsBYTE();
                        memcpy(pFloat++, &col, sizeof(RGBA));
                    }

                    if (mUseTexCoords)
                    {
                        if (mTexCoordDir == TCD_U)
                        {
                            *pFloat++ = elem.texCoord;
                            *pFloat++ = mOtherTexCoordRange[1];
                        }
                        else
                        {
                            *pFloat++ = mOtherTexCoordRange[1];
                            *pFloat++ = elem.texCoord;
                        }
                    }

                    if (e == seg.tail)
                        break; // last one

                    laste = e;

                } // element
            } // segment valid?

        } // each segment

        mVertexCameraUsed = cam;
        mVertexContentDirty = false;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::updateIndexBuffer(void)
    {

        setupBuffers();
        if (mIndexContentDirty)
        {
            HardwareBufferLockGuard indexLock(mIndexData->indexBuffer, HardwareBuffer::HBL_DISCARD);
            uint16* pShort = static_cast<uint16*>(indexLock.pData);
            mIndexData->indexCount = 0;
            // indexes
            for (ChainSegmentList::iterator segi = mChainSegmentList.begin();
                segi != mChainSegmentList.end(); ++segi)
            {
                ChainSegment& seg = *segi;

                // Skip 0 or 1 element segment counts
                if (seg.head != SEGMENT_EMPTY && seg.head != seg.tail)
                {
                    // Start from head + 1 since it's only useful in pairs
                    size_t laste = seg.head;
                    while(1) // until break
                    {
                        size_t e = laste + 1;
                        // Wrap forwards
                        if (e == mMaxElementsPerChain)
                            e = 0;
                        // indexes of this element are (e * 2) and (e * 2) + 1
                        // indexes of the last element are the same, -2
                        assert (((e + seg.start) * 2) < 65536 && "Too many elements!");
                        uint16 baseIdx = static_cast<uint16>((e + seg.start) * 2);
                        uint16 lastBaseIdx = static_cast<uint16>((laste + seg.start) * 2);
                        *pShort++ = lastBaseIdx;
                        *pShort++ = lastBaseIdx + 1;
                        *pShort++ = baseIdx;
                        *pShort++ = lastBaseIdx + 1;
                        *pShort++ = baseIdx + 1;
                        *pShort++ = baseIdx;

                        mIndexData->indexCount += 6;


                        if (e == seg.tail)
                            break; // last one

                        laste = e;

                    }
                }

            }

            mIndexContentDirty = false;
        }

    }
    //-----------------------------------------------------------------------
    Real BillboardChain::getSquaredViewDepth(const Camera* cam) const
    {
        return (cam->getDerivedPosition() - mAABB.getCenter()).squaredLength();
    }
    //-----------------------------------------------------------------------
    Real BillboardChain::getBoundingRadius(void) const
    {
        return mRadius;
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& BillboardChain::getBoundingBox(void) const
    {
        updateBoundingBox();
        return mAABB;
    }
    //-----------------------------------------------------------------------
    const MaterialPtr& BillboardChain::getMaterial(void) const
    {
        return mMaterial;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::setMaterialName( const String& name, const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
        mMaterial = MaterialManager::getSingleton().getByName(name, groupName);

        if (!mMaterial)
        {
            LogManager::getSingleton().logError("Can't assign material " + name +
                " to BillboardChain " + mName + " because this "
                "Material does not exist in group "+groupName+". Have you forgotten to define it in a "
                ".material script?");
            mMaterial = MaterialManager::getSingleton().getDefaultMaterial(false);
        }
        // Ensure new material loaded (will not load again if already loaded)
        mMaterial->load();
    }
    //-----------------------------------------------------------------------
    const String& BillboardChain::getMovableType(void) const
    {
        return BillboardChainFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::_updateRenderQueue(RenderQueue* queue)
    {
        updateIndexBuffer();

        if (mIndexData->indexCount > 0)
        {
            if (mRenderQueuePrioritySet)
                queue->addRenderable(this, mRenderQueueID, mRenderQueuePriority);
            else if (mRenderQueueIDSet)
                queue->addRenderable(this, mRenderQueueID);
            else
                queue->addRenderable(this);
        }

    }
    //-----------------------------------------------------------------------
    void BillboardChain::getRenderOperation(RenderOperation& op)
    {
        op.indexData = mIndexData.get();
        op.operationType = RenderOperation::OT_TRIANGLE_LIST;
        op.srcRenderable = this;
        op.useIndexes = true;
        op.vertexData = mVertexData.get();
    }
    //-----------------------------------------------------------------------
    bool BillboardChain::preRender(SceneManager* sm, RenderSystem* rsys)
    {
        // Retrieve the current viewport from the scene manager.
        // The viewport is only valid during a viewport update.
        Viewport *currentViewport = sm->getCurrentViewport();
        if( !currentViewport )
            return false;

        updateVertexBuffer(currentViewport->getCamera());
        return true;
    }
    //-----------------------------------------------------------------------
    void BillboardChain::getWorldTransforms(Matrix4* xform) const
    {
        *xform = _getParentNodeFullTransform();
    }
    //-----------------------------------------------------------------------
    const LightList& BillboardChain::getLights(void) const
    {
        return queryLights();
    }
    //---------------------------------------------------------------------
    void BillboardChain::visitRenderables(Renderable::Visitor* visitor, 
        bool debugRenderables)
    {
        // only one renderable
        visitor->visit(this, 0, false);
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String BillboardChainFactory::FACTORY_TYPE_NAME = "BillboardChain";
    //-----------------------------------------------------------------------
    const String& BillboardChainFactory::getType(void) const
    {
        return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    MovableObject* BillboardChainFactory::createInstanceImpl( const String& name,
        const NameValuePairList* params)
    {
        size_t maxElements = 20;
        size_t numberOfChains = 1;
        bool useTex = true;
        bool useCol = true;
        bool dynamic = true;
        // optional params
        if (params != 0)
        {
            NameValuePairList::const_iterator ni = params->find("maxElements");
            if (ni != params->end())
            {
                maxElements = StringConverter::parseSizeT(ni->second);
            }
            ni = params->find("numberOfChains");
            if (ni != params->end())
            {
                numberOfChains = StringConverter::parseSizeT(ni->second);
            }
            ni = params->find("useTextureCoords");
            if (ni != params->end())
            {
                useTex = StringConverter::parseBool(ni->second);
            }
            ni = params->find("useVertexColours");
            if (ni != params->end())
            {
                useCol = StringConverter::parseBool(ni->second);
            }
            ni = params->find("dynamic");
            if (ni != params->end())
            {
                dynamic = StringConverter::parseBool(ni->second);
            }

        }

        return OGRE_NEW BillboardChain(name, maxElements, numberOfChains, useTex, useCol, dynamic);

    }
}


