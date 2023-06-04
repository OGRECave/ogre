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
#include "OgreVertexIndexData.h"
#include "OgreHardwareVertexBuffer.h"

#define INT10_MAX ((1 << 9) - 1)

namespace Ogre {
    static void swapPackedRB(uint32* ptr)
    {
        auto cptr = (uint8*)ptr;
        std::swap(cptr[0], cptr[2]);
    }

    struct int_10_10_10_2
    {
        int32_t x : 10;
        int32_t y : 10;
        int32_t z : 10;
        int32_t w :  2;
    };

    template<int INCLUDE_W>
    static void pack_10_10_10_2(uint8* pDst, uint8* pSrc, int elemOffset)
    {
        float* pFloat = (float*)(pSrc + elemOffset);
        int_10_10_10_2 packed = {int(INT10_MAX * pFloat[0]), int(INT10_MAX * pFloat[1]), int(INT10_MAX * pFloat[2]), 1};
        if(INCLUDE_W)
            packed.w = int(pFloat[3]);
        memcpy(pDst + elemOffset, &packed, sizeof(int_10_10_10_2));
    }

    template<int INCLUDE_W>
    static void unpack_10_10_10_2(uint8* pDst, uint8* pSrc, int elemOffset)
    {
        int_10_10_10_2* pPacked = (int_10_10_10_2*)(pSrc + elemOffset);
        float* pFloat = (float*)(pDst + elemOffset);

        pFloat[0] = float(pPacked->x) / INT10_MAX;
        pFloat[1] = float(pPacked->y) / INT10_MAX;
        pFloat[2] = float(pPacked->z) / INT10_MAX;
        if(INCLUDE_W)
            pFloat[3] = pPacked->w;
    }

    static void copy_float3(uint8* pDst, uint8* pSrc, int elemOffset)
    {
        memcpy(pDst, pSrc + elemOffset, sizeof(float) * 3);
    }

    static void spliceElement(const VertexElement* elem, const HardwareVertexBufferPtr& srcBuf, uint8* pDst,
                              uint8* pElemDst, uint32 newElemSize, void (*elemConvert)(uint8*, uint8*, int))
    {
        auto vertexSize = srcBuf->getVertexSize();
        auto numVerts = srcBuf->getNumVertices();

        auto elemSize = elem->getSize();
        auto elemOffset = elem->getOffset();

        auto postVertexOffset = elemOffset + elemSize;
        auto postVertexSize = vertexSize - postVertexOffset;

        auto elemDstSize = pDst == pElemDst ? newElemSize : 0;
        size_t newVertexSize = vertexSize - elemSize + elemDstSize;
        auto elemDstStep = pDst == pElemDst ? newVertexSize : newElemSize;

        HardwareBufferLockGuard srcLock(srcBuf, HardwareBuffer::HBL_READ_ONLY);
        uint8* pSrc = static_cast<uint8*>(srcLock.pData);

        for (uint32 v = 0; v < numVerts; ++v)
        {
            // copy and convert element from vertex
            elemConvert(pElemDst, pSrc, elemOffset);
            pElemDst += elemDstStep;

            // copy over other data
            if (elemOffset)
                memcpy(pDst, pSrc, elemOffset);
            if (postVertexSize)
                memcpy(pDst + elemOffset + elemDstSize, pSrc + postVertexOffset, postVertexSize);

            pSrc += vertexSize;
            pDst += newVertexSize;
        }
    }

    static void updateVertexDeclaration(VertexDeclaration* decl, const VertexElement* elem, VertexElementType newType, uint16 newSource)
    {
        auto elemSize = elem->getSize();
        auto oldElemOffset = elem->getOffset();
        auto newElemOffset = oldElemOffset;

        auto newElemSize = VertexElement::getTypeSize(newType);
        auto oldSource = elem->getSource();

        if(newSource != oldSource)
        {
            newElemOffset = 0;
            newElemSize = 0;
        }

        uint16 idx = 0;
        for (const auto& e : decl->getElements())
        {
            if (&e == elem)
            {
                // Modify element
                decl->modifyElement(idx, newSource, newElemOffset, newType, elem->getSemantic());
            }
            else if (e.getSource() == oldSource && e.getOffset() > oldElemOffset)
            {
                // shift elements after this one
                decl->modifyElement(idx, e.getSource(), e.getOffset() - elemSize + newElemSize, e.getType(),
                                    e.getSemantic(), e.getIndex());
            }
            idx++;
        }
    }

    //-----------------------------------------------------------------------
    VertexData::VertexData(HardwareBufferManagerBase* mgr)
    {
        mMgr = mgr ? mgr : HardwareBufferManager::getSingletonPtr();
        vertexBufferBinding = mMgr->createVertexBufferBinding();
        vertexDeclaration = mMgr->createVertexDeclaration();
        mDeleteDclBinding = true;
        vertexCount = 0;
        vertexStart = 0;
        hwAnimDataItemsUsed = 0;

    }
    //---------------------------------------------------------------------
    VertexData::VertexData(VertexDeclaration* dcl, VertexBufferBinding* bind)
    {
        // this is a fallback rather than actively used
        mMgr = HardwareBufferManager::getSingletonPtr();
        vertexDeclaration = dcl;
        vertexBufferBinding = bind;
        mDeleteDclBinding = false;
        vertexCount = 0;
        vertexStart = 0;
        hwAnimDataItemsUsed = 0;
    }
    //-----------------------------------------------------------------------
    VertexData::~VertexData()
    {
        if (mDeleteDclBinding)
        {
            mMgr->destroyVertexBufferBinding(vertexBufferBinding);
            mMgr->destroyVertexDeclaration(vertexDeclaration);
        }
    }
    //-----------------------------------------------------------------------
    VertexData* VertexData::clone(bool copyData, HardwareBufferManagerBase* mgr) const
    {
        HardwareBufferManagerBase* pManager = mgr ? mgr : mMgr;

        VertexData* dest = OGRE_NEW VertexData(mgr);

        // Copy vertex buffers in turn
        const VertexBufferBinding::VertexBufferBindingMap& bindings = 
            this->vertexBufferBinding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator vbi, vbend;
        vbend = bindings.end();
        for (vbi = bindings.begin(); vbi != vbend; ++vbi)
        {
            HardwareVertexBufferSharedPtr srcbuf = vbi->second;
            HardwareVertexBufferSharedPtr dstBuf;
            if (copyData)
            {
                // create new buffer with the same settings
                dstBuf = pManager->createVertexBuffer(
                        srcbuf->getVertexSize(), srcbuf->getNumVertices(), srcbuf->getUsage(),
                        srcbuf->hasShadowBuffer());

                // copy data
                dstBuf->copyData(*srcbuf, 0, 0, srcbuf->getSizeInBytes(), true);
            }
            else
            {
                // don't copy, point at existing buffer
                dstBuf = srcbuf;
            }

            // Copy binding
            dest->vertexBufferBinding->setBinding(vbi->first, dstBuf);
        }

        // Basic vertex info
        dest->vertexStart = this->vertexStart;
        dest->vertexCount = this->vertexCount;
        // Copy elements
        const VertexDeclaration::VertexElementList elems = 
            this->vertexDeclaration->getElements();
        VertexDeclaration::VertexElementList::const_iterator ei, eiend;
        eiend = elems.end();
        for (ei = elems.begin(); ei != eiend; ++ei)
        {
            dest->vertexDeclaration->addElement(
                ei->getSource(),
                ei->getOffset(),
                ei->getType(),
                ei->getSemantic(),
                ei->getIndex() );
        }

        // Copy reference to hardware shadow buffer, no matter whether copy data or not
        dest->hardwareShadowVolWBuffer = hardwareShadowVolWBuffer;

        // copy anim data
        dest->hwAnimationDataList = hwAnimationDataList;
        dest->hwAnimDataItemsUsed = hwAnimDataItemsUsed;

        
        return dest;
    }

    void VertexData::convertVertexElement(VertexElementSemantic semantic, VertexElementType dstType)
    {
        auto elem = vertexDeclaration->findElementBySemantic(semantic);

        if (!elem || VertexElement::getBaseType(elem->getType()) == VertexElement::getBaseType(dstType))
            return; // nothing to do

        auto srcType = elem->getType();
        auto vbuf = vertexBufferBinding->getBuffer(elem->getSource());

        size_t newElemSize = VertexElement::getTypeSize(dstType);
        size_t newVertexSize = vbuf->getVertexSize() - elem->getSize() + newElemSize;
        auto newVBuf = vbuf->getManager()->createVertexBuffer(newVertexSize, vbuf->getNumVertices(), vbuf->getUsage(),
                                                              vbuf->hasShadowBuffer());

        {
            HardwareBufferLockGuard dst(newVBuf, HardwareBuffer::HBL_DISCARD);
            auto pDst = static_cast<uint8*>(dst.pData);

            if(dstType == VET_INT_10_10_10_2_NORM)
            {
                if(srcType == VET_FLOAT3)
                    spliceElement(elem, vbuf, pDst, pDst, newElemSize, pack_10_10_10_2<false>);
                else
                {
                    OgreAssert(srcType == VET_FLOAT4, "unsupported conversion");
                    spliceElement(elem, vbuf, pDst, pDst, newElemSize, pack_10_10_10_2<true>);
                }
            }
            else if(dstType == VET_FLOAT3)
            {
                OgreAssert(srcType == VET_INT_10_10_10_2_NORM, "unsupported conversion");
                spliceElement(elem, vbuf, pDst, pDst, newElemSize, unpack_10_10_10_2<false>);
            }
            else if(dstType == VET_FLOAT4)
            {
                OgreAssert(srcType == VET_INT_10_10_10_2_NORM, "unsupported conversion");
                spliceElement(elem, vbuf, pDst, pDst, newElemSize, unpack_10_10_10_2<true>);
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "unsupported dstType");
            }
        }

        // Bind the new buffer
        vertexBufferBinding->setBinding(elem->getSource(), newVBuf);
        updateVertexDeclaration(vertexDeclaration, elem, dstType, elem->getSource());
    }
    //-----------------------------------------------------------------------
    void VertexData::prepareForShadowVolume(void)
    {
        /* NOTE
        I would dearly, dearly love to just use a 4D position buffer in order to 
        store the extra 'w' value I need to differentiate between extruded and 
        non-extruded sections of the buffer, so that vertex programs could use that.
        Hey, it works fine for GL. However, D3D9 in it's infinite stupidity, does not
        support 4d position vertices in the fixed-function pipeline. If you use them, 
        you just see nothing. Since we can't know whether the application is going to use
        fixed function or vertex programs, we have to stick to 3d position vertices and
        store the 'w' in a separate 1D texture coordinate buffer, which is only used
        when rendering the shadow.
        */

        // Look for a position element
        const VertexElement* posElem = vertexDeclaration->findElementBySemantic(VES_POSITION);
        if (!posElem)
            return;

        // Upfront, lets check whether we have vertex program capability
        bool useVertexPrograms = Root::getSingleton().getRenderSystem() != 0;

        auto vbuf = vertexBufferBinding->getBuffer(posElem->getSource());

        // Are there other elements in the buffer except for the position?
        // We need to create another buffer to contain the remaining elements
        // Most drivers don't like gaps in the declaration, and in any case it's waste
        HardwareVertexBufferPtr newRemainderBuffer;
        if (vbuf->getVertexSize() > posElem->getSize())
        {
            newRemainderBuffer = vbuf->getManager()->createVertexBuffer(
                vbuf->getVertexSize() - posElem->getSize(), vbuf->getNumVertices(), vbuf->getUsage(),
                vbuf->hasShadowBuffer());
        }
        // Allocate new position buffer, will be FLOAT3 and 2x the size
        size_t oldVertexCount = vbuf->getNumVertices();
        size_t newVertexCount = oldVertexCount * 2;
        auto newPosBuffer = vbuf->getManager()->createVertexBuffer(
            VertexElement::getTypeSize(VET_FLOAT3), newVertexCount, vbuf->getUsage(), vbuf->hasShadowBuffer());

        // Point first destination pointer at the start of the new position buffer,
        // the other one half way along
        auto pDest = static_cast<float*>(newPosBuffer->lock(HardwareBuffer::HBL_DISCARD));
        auto pDest2 = pDest + oldVertexCount * 3;

        if (newRemainderBuffer)
        {
            // Basically we just memcpy the vertex excluding the position
            HardwareBufferLockGuard destRemLock(newRemainderBuffer, HardwareBuffer::HBL_DISCARD);
            spliceElement(posElem, vbuf, (uint8*)destRemLock.pData, (uint8*)pDest, posElem->getSize(), copy_float3);
        }
        else
        {
            // Unshared buffer, can block copy the whole thing
            vbuf->readData(0, vbuf->getSizeInBytes(), pDest);
        }

        memcpy(pDest2, pDest, oldVertexCount * 3 * sizeof(float));

        newPosBuffer->unlock();

        // At this stage, he original vertex buffer is going to be destroyed
        // So we should force the deallocation of any temporary copies
        vbuf->getManager()->_forceReleaseBufferCopies(vbuf);

        if (useVertexPrograms)
        {
            // Now it's time to set up the w buffer
            hardwareShadowVolWBuffer =
                vbuf->getManager()->createVertexBuffer(sizeof(float), newVertexCount, HBU_GPU_ONLY, false);
            // Fill the first half with 1.0, second half with 0.0
            pDest = static_cast<float*>(hardwareShadowVolWBuffer->lock(HardwareBuffer::HBL_DISCARD));
            for (size_t v = 0; v < oldVertexCount; ++v)
            {
                *pDest++ = 1.0f;
            }
            // Fill the second half with 0.0
            memset(pDest, 0, sizeof(float) * oldVertexCount);
            hardwareShadowVolWBuffer->unlock();
        }

        auto newPosBufferSource = posElem->getSource();
        if (newRemainderBuffer)
        {
            // Get the a new buffer binding index
            newPosBufferSource= vertexBufferBinding->getNextIndex();
            // Re-bind the old index to the remainder buffer
            vertexBufferBinding->setBinding(posElem->getSource(), newRemainderBuffer);
        }

        // Bind the new position buffer
        vertexBufferBinding->setBinding(newPosBufferSource, newPosBuffer);

        // Now, alter the vertex declaration to change the position source
        // and the offsets of elements using the same buffer
        // Note that we don't change vertexCount, because the other buffer(s) are still the same
        // size after all
        updateVertexDeclaration(vertexDeclaration, posElem, VET_FLOAT3, newPosBufferSource);
    }
    //-----------------------------------------------------------------------
    void VertexData::reorganiseBuffers(VertexDeclaration* newDeclaration, const BufferUsageList& bufferUsages,
                                       HardwareBufferManagerBase* mgr)
    {
        HardwareBufferManagerBase* pManager = mgr ? mgr : mMgr;
        // Firstly, close up any gaps in the buffer sources which might have arisen
        newDeclaration->closeGapsInSource();

        // Build up a list of both old and new elements in each buffer
        std::vector<uint8*> oldBufferLocks;
        std::vector<size_t> oldBufferVertexSizes;
        std::vector<uint8*> newBufferLocks;
        std::vector<size_t> newBufferVertexSizes;
        VertexBufferBinding* newBinding = pManager->createVertexBufferBinding();
        const auto& oldBindingMap = vertexBufferBinding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator itBinding;

        // Pre-allocate old buffer locks
        if (!oldBindingMap.empty())
        {
            size_t count = oldBindingMap.rbegin()->first + 1;
            oldBufferLocks.resize(count);
            oldBufferVertexSizes.resize(count);
        }

        bool useShadowBuffer = false;

        // Lock all the old buffers for reading
        for (const auto& it : oldBindingMap)
        {
            assert(it.second->getNumVertices() >= vertexCount);

            oldBufferVertexSizes[it.first] = it.second->getVertexSize();
            oldBufferLocks[it.first] = (uint8*)it.second->lock(HardwareBuffer::HBL_READ_ONLY);

            useShadowBuffer |= it.second->hasShadowBuffer();
        }
        
        // Create new buffers and lock all for writing
        uint16 buf = 0;
        while (!newDeclaration->findElementsBySource(buf).empty())
        {
            size_t vertexSize = newDeclaration->getVertexSize(buf);

            auto vbuf = pManager->createVertexBuffer(vertexSize, vertexCount, bufferUsages[buf], useShadowBuffer);
            newBinding->setBinding(buf, vbuf);

            newBufferVertexSizes.push_back(vertexSize);
            newBufferLocks.push_back((uint8*)vbuf->lock(HardwareBuffer::HBL_DISCARD));
            buf++;
        }

        // Map from new to old elements
        std::map<const VertexElement*, const VertexElement*>  newToOldElementMap;
        const auto& newElemList = newDeclaration->getElements();
        for (const auto& newElem : newElemList)
        {
            // Find corresponding old element
            auto oldElem = vertexDeclaration->findElementBySemantic(newElem.getSemantic(), newElem.getIndex());
            if (!oldElem)
            {
                // Error, cannot create new elements with this method
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Element not found in old vertex declaration");
            }
            newToOldElementMap[&newElem] = oldElem;
        }
        // Now iterate over the new buffers, pulling data out of the old ones
        // For each vertex
        for (size_t v = 0; v < vertexCount; ++v)
        {
            // For each (new) element
            for (const auto& newElem : newElemList)
            {
                const VertexElement* oldElem = newToOldElementMap[&newElem];
                auto oldBufferNo = oldElem->getSource();
                auto newBufferNo = newElem.getSource();
                auto pSrc = oldBufferLocks[oldBufferNo] + v * oldBufferVertexSizes[oldBufferNo];
                auto pDst = newBufferLocks[newBufferNo] + v * newBufferVertexSizes[newBufferNo];
                memcpy(pDst + newElem.getOffset(), pSrc + oldElem->getOffset(), newElem.getSize());
            }
        }

        // Unlock all buffers
        for (const auto& it : oldBindingMap)
        {
            it.second->unlock();
        }
        for (buf = 0; buf < newBinding->getBufferCount(); ++buf)
        {
            newBinding->getBuffer(buf)->unlock();
        }

        // Delete old binding & declaration
        if (mDeleteDclBinding)
        {
            pManager->destroyVertexBufferBinding(vertexBufferBinding);
            pManager->destroyVertexDeclaration(vertexDeclaration);
        }

        // Assign new binding and declaration
        vertexDeclaration = newDeclaration;
        vertexBufferBinding = newBinding;       
        // after this is complete, new manager should be used
        mMgr = pManager;
        mDeleteDclBinding = true; // because we created these through a manager

    }
    //-----------------------------------------------------------------------
    void VertexData::reorganiseBuffers(VertexDeclaration* newDeclaration, HardwareBufferManagerBase* mgr)
    {
        // Derive the buffer usages from looking at where the source has come
        // from
        BufferUsageList usages;
        for (unsigned short b = 0; b <= newDeclaration->getMaxSource(); ++b)
        {
            VertexDeclaration::VertexElementList destElems = newDeclaration->findElementsBySource(b);
            // Initialise with most restrictive version
            uint8 final = HBU_GPU_ONLY;
            for (VertexElement& destelem : destElems)
            {
                // get source
                const VertexElement* srcelem =
                    vertexDeclaration->findElementBySemantic(
                        destelem.getSemantic(), destelem.getIndex());
                // get buffer
                HardwareVertexBufferSharedPtr srcbuf = 
                    vertexBufferBinding->getBuffer(srcelem->getSource());
                // improve flexibility only
                if (srcbuf->getUsage() & HBU_CPU_ONLY)
                {
                    // remove static
                    final &= ~HBU_GPU_TO_CPU;
                    // add dynamic
                    final |= HBU_CPU_ONLY;
                }
                if (!(srcbuf->getUsage() & HBU_DETAIL_WRITE_ONLY))
                {
                    // remove write only
                    final &= ~HBU_DETAIL_WRITE_ONLY;
                }
            }
            usages.push_back(static_cast<HardwareBufferUsage>(final));
        }
        // Call specific method
        reorganiseBuffers(newDeclaration, usages, mgr);

    }
    //-----------------------------------------------------------------------
    void VertexData::closeGapsInBindings(void)
    {
        if (!vertexBufferBinding->hasGaps())
            return;

        // Check for error first
        const VertexDeclaration::VertexElementList& allelems = 
            vertexDeclaration->getElements();
        for (auto& e : allelems)
        {
            if (!vertexBufferBinding->isBufferBound(e.getSource()))
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                    "No buffer is bound to that element source.",
                    "VertexData::closeGapsInBindings");
            }
        }

        // Close gaps in the vertex buffer bindings
        VertexBufferBinding::BindingIndexMap bindingIndexMap;
        vertexBufferBinding->closeGaps(bindingIndexMap);

        // Modify vertex elements to reference to new buffer index
        unsigned short elemIndex = 0;
        for (auto ai = allelems.begin(); ai != allelems.end(); ++ai, ++elemIndex)
        {
            const VertexElement& elem = *ai;
            VertexBufferBinding::BindingIndexMap::const_iterator it =
                bindingIndexMap.find(elem.getSource());
            assert(it != bindingIndexMap.end());
            ushort targetSource = it->second;
            if (elem.getSource() != targetSource)
            {
                vertexDeclaration->modifyElement(elemIndex, 
                    targetSource, elem.getOffset(), elem.getType(), 
                    elem.getSemantic(), elem.getIndex());
            }
        }
    }
    //-----------------------------------------------------------------------
    void VertexData::removeUnusedBuffers(void)
    {
        std::set<ushort> usedBuffers;

        // Collect used buffers
        const VertexDeclaration::VertexElementList& allelems = 
            vertexDeclaration->getElements();
        for (auto& e : allelems)
        {
            usedBuffers.insert(e.getSource());
        }

        // Unset unused buffer bindings
        ushort count = vertexBufferBinding->getLastBoundIndex();
        for (ushort index = 0; index < count; ++index)
        {
            if (usedBuffers.find(index) == usedBuffers.end() &&
                vertexBufferBinding->isBufferBound(index))
            {
                vertexBufferBinding->unsetBinding(index);
            }
        }

        // Close gaps
        closeGapsInBindings();
    }
    //-----------------------------------------------------------------------
    void VertexData::convertPackedColour(VertexElementType, VertexElementType destType)
    {
        OgreAssert(destType == VET_UBYTE4_NORM, "Not supported");

        const VertexBufferBinding::VertexBufferBindingMap& bindMap = 
            vertexBufferBinding->getBindings();
        for (auto& m : bindMap)
        {
            const auto& elems =
                vertexDeclaration->findElementsBySource(m.first);
            bool conversionNeeded = false;
            for (auto& e : elems)
            {
                if (e.getType() == _DETAIL_SWAP_RB)
                {
                    conversionNeeded = true;
                }
            }

            if (conversionNeeded)
            {
                void* pBase = m.second->lock(HardwareBuffer::HBL_NORMAL);

                for (size_t v = 0; v < m.second->getNumVertices(); ++v)
                {

                    for (auto& e : elems)
                    {
                        if (e.getType() == _DETAIL_SWAP_RB)
                        {
                            uint32* pRGBA;
                            e.baseVertexPointerToElement(pBase, &pRGBA);
                            swapPackedRB(pRGBA);
                        }
                    }
                    pBase = static_cast<void*>(
                        static_cast<char*>(pBase) + m.second->getVertexSize());
                }
                m.second->unlock();

                // Modify the elements to reflect the changed type
                const VertexDeclaration::VertexElementList& allelems = 
                    vertexDeclaration->getElements();
                unsigned short elemIndex = 0;
                for (auto& e : allelems)
                {
                    if (e.getType() == _DETAIL_SWAP_RB)
                    {
                        vertexDeclaration->modifyElement(elemIndex,
                            e.getSource(), e.getOffset(), destType,
                            e.getSemantic(), e.getIndex());
                    }
                    ++elemIndex;
                }
            }
        } // each buffer
    }
    //-----------------------------------------------------------------------
    ushort VertexData::allocateHardwareAnimationElements(ushort count, bool animateNormals)
    {
        // Find first free texture coord set
        unsigned short texCoord = vertexDeclaration->getNextFreeTextureCoordinate();
        unsigned short freeCount = (ushort)(OGRE_MAX_TEXTURE_COORD_SETS - texCoord);
        if (animateNormals)
            // we need 2x the texture coords, round down
            freeCount /= 2;
        
        unsigned short supportedCount = std::min(freeCount, count);
        
        // Increase to correct size
        for (size_t c = hwAnimationDataList.size(); c < supportedCount; ++c)
        {
            // Create a new 3D texture coordinate set
            HardwareAnimationData data;
            data.targetBufferIndex = vertexBufferBinding->getNextIndex();
            vertexDeclaration->addElement(data.targetBufferIndex, 0, VET_FLOAT3, VES_TEXTURE_COORDINATES, texCoord++);
            if (animateNormals)
                    vertexDeclaration->addElement(data.targetBufferIndex, sizeof(float)*3, VET_FLOAT3, VES_TEXTURE_COORDINATES, texCoord++);

            hwAnimationDataList.push_back(data);
            // Vertex buffer will not be bound yet, we expect this to be done by the
            // caller when it becomes appropriate (e.g. through a VertexAnimationTrack)
        }
        
        return supportedCount;
    }
    VertexData* VertexData::_cloneRemovingBlendData() const
    {
        // Clone without copying data
        VertexData* ret = clone(false);
        bool removeIndices = Root::getSingleton().isBlendIndicesGpuRedundant();
        bool removeWeights = Root::getSingleton().isBlendWeightsGpuRedundant();

        unsigned short safeSource = 0xFFFF;
        auto blendIndexElem = vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
        if (blendIndexElem && removeIndices)
        {
            //save the source in order to prevent the next stage from unbinding it.
            safeSource = blendIndexElem->getSource();
            // Remove buffer reference
            ret->vertexBufferBinding->unsetBinding(blendIndexElem->getSource());
        }

        // Remove blend weights
        const VertexElement* blendWeightElem = vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS);
        if (removeWeights && blendWeightElem && blendWeightElem->getSource() != safeSource)
        {
            // Remove buffer reference
            ret->vertexBufferBinding->unsetBinding(blendWeightElem->getSource());
        }

        // remove elements from declaration
        if (removeIndices)
            ret->vertexDeclaration->removeElement(VES_BLEND_INDICES);
        if (removeWeights)
            ret->vertexDeclaration->removeElement(VES_BLEND_WEIGHTS);

        // Close gaps in bindings for effective and safely
        if (removeWeights || removeIndices)
            ret->closeGapsInBindings();

        return ret;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    IndexData::IndexData()
    {
        indexCount = 0;
        indexStart = 0;
        
    }
    //-----------------------------------------------------------------------
    IndexData::~IndexData()
    {
    }
    //-----------------------------------------------------------------------
    IndexData* IndexData::clone(bool copyData, HardwareBufferManagerBase* mgr) const
    {
        HardwareBufferManagerBase* pManager = mgr ? mgr : HardwareBufferManager::getSingletonPtr();
        IndexData* dest = OGRE_NEW IndexData();
        if (indexBuffer.get())
        {
            if (copyData)
            {
                dest->indexBuffer = pManager->createIndexBuffer(indexBuffer->getType(), indexBuffer->getNumIndexes(),
                    indexBuffer->getUsage(), indexBuffer->hasShadowBuffer());
                dest->indexBuffer->copyData(*indexBuffer, 0, 0, indexBuffer->getSizeInBytes(), true);
            }
            else
            {
                dest->indexBuffer = indexBuffer;
            }
        }
        dest->indexCount = indexCount;
        dest->indexStart = indexStart;
        return dest;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Local Utility class for vertex cache optimizer
    class Triangle
    {
    public:
        enum EdgeMatchType {
            AB, BC, CA, ANY, NONE
        };

        uint32 a, b, c;     

        inline Triangle()
        {
        }

        inline Triangle( uint32 ta, uint32 tb, uint32 tc ) 
            : a( ta ), b( tb ), c( tc )
        {
        }

        inline Triangle( uint32 t[3] )
            : a( t[0] ), b( t[1] ), c( t[2] )
        {
        }

        inline Triangle( const Triangle& t )
            : a( t.a ), b( t.b ), c( t.c )
        {
        }

        inline Triangle& operator=(const Triangle& rhs) {
            a = rhs.a;
            b = rhs.b;
            c = rhs.c;
            return *this;
        }

        inline bool sharesEdge(const Triangle& t) const
        {
            return( (a == t.a && b == t.c) ||
                    (a == t.b && b == t.a) ||
                    (a == t.c && b == t.b) ||
                    (b == t.a && c == t.c) ||
                    (b == t.b && c == t.a) ||
                    (b == t.c && c == t.b) ||
                    (c == t.a && a == t.c) ||
                    (c == t.b && a == t.a) ||
                    (c == t.c && a == t.b) );
        }

        inline bool sharesEdge(const uint32 ea, const uint32 eb, const Triangle& t) const
        {
            return( (ea == t.a && eb == t.c) ||
                    (ea == t.b && eb == t.a) ||
                    (ea == t.c && eb == t.b) ); 
        }

        inline bool sharesEdge(const EdgeMatchType edge, const Triangle& t) const
        {
            if (edge == AB)
                return sharesEdge(a, b, t);
            else if (edge == BC)
                return sharesEdge(b, c, t);
            else if (edge == CA)
                return sharesEdge(c, a, t);
            else
                return (edge == ANY) == sharesEdge(t);
        }

        inline EdgeMatchType endoSharedEdge(const Triangle& t) const
        {
            if (sharesEdge(a, b, t)) return AB;
            if (sharesEdge(b, c, t)) return BC;
            if (sharesEdge(c, a, t)) return CA;
            return NONE;
        }

        inline EdgeMatchType exoSharedEdge(const Triangle& t) const
        {
            return t.endoSharedEdge(*this);
        }

        inline void shiftClockwise()
        {
            uint32 t = a;
            a = c;
            c = b;
            b = t;
        }

        inline void shiftCounterClockwise()
        {
            uint32 t = a;
            a = b;
            b = c;
            c = t;
        }
    };
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    void IndexData::optimiseVertexCacheTriList(void)
    {
        if (indexBuffer->isLocked()) return;

        void *buffer = indexBuffer->lock(HardwareBuffer::HBL_NORMAL);

        Triangle* triangles;

        size_t nIndexes = indexCount;
        size_t nTriangles = nIndexes / 3;
        size_t i, j;
        uint16 *source = 0;

        if (indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT)
        {
            triangles = OGRE_ALLOC_T(Triangle, nTriangles, MEMCATEGORY_GEOMETRY);
            source = (uint16 *)buffer;
            uint32 *dest = (uint32 *)triangles;
            for (i = 0; i < nIndexes; ++i) dest[i] = source[i];
        }
        else
            triangles = static_cast<Triangle*>(buffer);

        // sort triangles based on shared edges
        uint32 *destlist = OGRE_ALLOC_T(uint32, nTriangles, MEMCATEGORY_GEOMETRY);
        unsigned char *visited = OGRE_ALLOC_T(unsigned char, nTriangles, MEMCATEGORY_GEOMETRY);

        for (i = 0; i < nTriangles; ++i) visited[i] = 0;

        uint32 start = 0, ti = 0, destcount = 0;

        bool found = false;
        for (i = 0; i < nTriangles; ++i)
        {
            if (found)
                found = false;
            else
            {
                while (visited[start++]);
                ti = start - 1;
            }

            destlist[destcount++] = ti;
            visited[ti] = 1;

            for (j = start; j < nTriangles; ++j)
            {
                if (visited[j]) continue;
                
                if (triangles[ti].sharesEdge(triangles[j]))
                {
                    found = true;
                    ti = static_cast<uint32>(j);
                    break;
                }
            }
        }

        if (indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT)
        {
            // reorder the indexbuffer
            j = 0;
            for (i = 0; i < nTriangles; ++i)
            {
                Triangle *t = &triangles[destlist[i]];
                if(source)
                {
                    source[j++] = (uint16)t->a;
                    source[j++] = (uint16)t->b;
                    source[j++] = (uint16)t->c;
                }
            }
            OGRE_FREE(triangles, MEMCATEGORY_GEOMETRY);
        }
        else
        {
            uint32 *reflist = OGRE_ALLOC_T(uint32, nTriangles, MEMCATEGORY_GEOMETRY);

            // fill the referencebuffer
            for (i = 0; i < nTriangles; ++i)
                reflist[destlist[i]] = static_cast<uint32>(i);
            
            // reorder the indexbuffer
            for (i = 0; i < nTriangles; ++i)
            {
                j = destlist[i];
                if (i == j) continue; // do not move triangle

                // swap triangles

                Triangle t = triangles[i];
                triangles[i] = triangles[j];
                triangles[j] = t;

                // change reference
                destlist[reflist[i]] = static_cast<uint32>(j);
                // destlist[i] = i; // not needed, it will not be used
            }

            OGRE_FREE(reflist, MEMCATEGORY_GEOMETRY);
        }

        OGRE_FREE(destlist, MEMCATEGORY_GEOMETRY);
        OGRE_FREE(visited, MEMCATEGORY_GEOMETRY);
                    
        indexBuffer->unlock();
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    void VertexCacheProfiler::profile(const HardwareIndexBufferSharedPtr& indexBuffer)
    {
        if (indexBuffer->isLocked()) return;

        uint16 *shortbuffer = (uint16 *)indexBuffer->lock(HardwareBuffer::HBL_READ_ONLY);

        if (indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT)
            for (unsigned int i = 0; i < indexBuffer->getNumIndexes(); ++i)
                inCache(shortbuffer[i]);
        else
        {
            uint32 *buffer = (uint32 *)shortbuffer;
            for (unsigned int i = 0; i < indexBuffer->getNumIndexes(); ++i)
                inCache(buffer[i]);
        }

        indexBuffer->unlock();
    }

    //-----------------------------------------------------------------------
    bool VertexCacheProfiler::inCache(unsigned int index)
    {
        for (unsigned int i = 0; i < buffersize; ++i)
        {
            if (index == cache[i])
            {
                hit++;
                return true;
            }
        }

        miss++;
        cache[tail++] = index;
        tail %= size;

        if (buffersize < size) buffersize++;

        return false;
    }
    

}
