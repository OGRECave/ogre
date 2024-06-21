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
#include "OgreHardwareVertexBuffer.h"

#include <memory>
#include "OgreDefaultHardwareBufferManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------------
    HardwareVertexBuffer::HardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize,
        size_t numVertices, HardwareBuffer::Usage usage, bool useShadowBuffer)
        : HardwareBuffer(usage, useShadowBuffer),
          mIsInstanceData(false),
          mMgr(mgr),
          mNumVertices(numVertices),
          mVertexSize(vertexSize),
          mInstanceDataStepRate(1)
    {
        // Calculate the size of the vertices
        mSizeInBytes = mVertexSize * numVertices;

        // Create a shadow buffer if required
        if (useShadowBuffer)
        {
            mShadowBuffer = std::make_unique<DefaultHardwareBuffer>(mSizeInBytes);
        }

    }
    HardwareVertexBuffer::HardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize,
                                               size_t numVertices, HardwareBuffer* delegate)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, delegate->getUsage(), false)
    {
        mDelegate.reset(delegate);
    }
    //-----------------------------------------------------------------------------
    HardwareVertexBuffer::~HardwareVertexBuffer()
    {
        if (mMgr)
        {
            mMgr->_notifyVertexBufferDestroyed(this);
        }
    }
    //-----------------------------------------------------------------------------
    void HardwareVertexBuffer::setIsInstanceData( const bool val )
    {
        RenderSystem* rs = Root::getSingleton().getRenderSystem();

        OgreAssert(!val || rs->getCapabilities()->hasCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA),
                   "unsupported by rendersystem");

        mIsInstanceData = val;
    }
    //-----------------------------------------------------------------------------
    uint32 HardwareVertexBuffer::getInstanceDataStepRate() const
    {
        return mInstanceDataStepRate;
    }
    //-----------------------------------------------------------------------------
    void HardwareVertexBuffer::setInstanceDataStepRate( const size_t val )
    {
        if (val > 0)
        {
            mInstanceDataStepRate = val;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Instance data step rate must be bigger then 0.", 
                "HardwareVertexBuffer::setInstanceDataStepRate");
        }
    }
    //-----------------------------------------------------------------------------
    // VertexElement
    //-----------------------------------------------------------------------------
    VertexElement::VertexElement(unsigned short source, size_t offset, VertexElementType theType,
                                 VertexElementSemantic semantic, unsigned short index)
        : mOffset(offset), mSource(source), mIndex(index), mType(theType), mSemantic(semantic)
    {
    }
    //-----------------------------------------------------------------------------
    size_t VertexElement::getSize(void) const
    {
        return getTypeSize(mType);
    }
    //-----------------------------------------------------------------------------
    size_t VertexElement::getTypeSize(VertexElementType etype)
    {
        switch(etype)
        {
        case VET_FLOAT1:
            return sizeof(float);
        case VET_FLOAT2:
            return sizeof(float)*2;
        case VET_FLOAT3:
            return sizeof(float)*3;
        case VET_FLOAT4:
            return sizeof(float)*4;
        case VET_DOUBLE1:
            return sizeof(double);
        case VET_DOUBLE2:
            return sizeof(double)*2;
        case VET_DOUBLE3:
            return sizeof(double)*3;
        case VET_DOUBLE4:
            return sizeof(double)*4;
        case VET_SHORT1:
        case VET_USHORT1:
        case VET_HALF1:
            return sizeof( short );
        case VET_SHORT2:
        case VET_SHORT2_NORM:
        case VET_USHORT2:
        case VET_USHORT2_NORM:
        case VET_HALF2:
            return sizeof( short ) * 2;
        case VET_SHORT3:
        case VET_USHORT3:
        case VET_HALF3:
            return sizeof( short ) * 3;
        case VET_SHORT4:
        case VET_SHORT4_NORM:
        case VET_USHORT4:
        case VET_USHORT4_NORM:
        case VET_HALF4:
            return sizeof( short ) * 4;
        case VET_INT1:
        case VET_UINT1:
            return sizeof( int );
        case VET_INT2:
        case VET_UINT2:
            return sizeof( int ) * 2;
        case VET_INT3:
        case VET_UINT3:
            return sizeof( int ) * 3;
        case VET_INT4:
        case VET_UINT4:
            return sizeof( int ) * 4;
        case VET_BYTE4:
        case VET_BYTE4_NORM:
        case VET_UBYTE4:
        case VET_UBYTE4_NORM:
        case _DETAIL_SWAP_RB:
            return sizeof(char)*4;
        case VET_INT_10_10_10_2_NORM:
            return 4;
        }
        return 0;
    }
    //-----------------------------------------------------------------------------
    unsigned short VertexElement::getTypeCount(VertexElementType etype)
    {
        switch (etype)
        {
        case VET_FLOAT1:
        case VET_SHORT1:
        case VET_USHORT1:
        case VET_UINT1:
        case VET_INT1:
        case VET_DOUBLE1:
        case VET_HALF1:
            return 1;
        case VET_FLOAT2:
        case VET_SHORT2:
        case VET_SHORT2_NORM:
        case VET_USHORT2:
        case VET_USHORT2_NORM:
        case VET_UINT2:
        case VET_INT2:
        case VET_DOUBLE2:
        case VET_HALF2:
            return 2;
        case VET_FLOAT3:
        case VET_SHORT3:
        case VET_USHORT3:
        case VET_UINT3:
        case VET_INT3:
        case VET_DOUBLE3:
        case VET_HALF3:
            return 3;
        case VET_FLOAT4:
        case VET_SHORT4:
        case VET_SHORT4_NORM:
        case VET_USHORT4:
        case VET_USHORT4_NORM:
        case VET_UINT4:
        case VET_INT4:
        case VET_DOUBLE4:
        case VET_BYTE4:
        case VET_UBYTE4:
        case VET_BYTE4_NORM:
        case VET_UBYTE4_NORM:
        case _DETAIL_SWAP_RB:
        case VET_INT_10_10_10_2_NORM:
        case VET_HALF4:
            return 4;
        }
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid type", 
            "VertexElement::getTypeCount");
    }
    //-----------------------------------------------------------------------------
    VertexElementType VertexElement::multiplyTypeCount(VertexElementType baseType, 
        unsigned short count)
    {
        OgreAssert(count > 0 && count < 5, "Count out of range");

        switch (baseType)
        {
        case VET_FLOAT1:
        case VET_DOUBLE1:
        case VET_INT1:
        case VET_UINT1:
        case VET_HALF1:
            // evil enumeration arithmetic
            return static_cast<VertexElementType>( baseType + count - 1 );

        case VET_SHORT1:
        case VET_SHORT2:
            if ( count <= 2 )
            {
                return VET_SHORT2;
            }
            return VET_SHORT4;

        case VET_USHORT1:
        case VET_USHORT2:
            if ( count <= 2 )
            {
                return VET_USHORT2;
            }
            return VET_USHORT4;

        case VET_SHORT2_NORM:
            if ( count <= 2 )
            {
                return VET_SHORT2_NORM;
            }
            return VET_SHORT4_NORM;

        case VET_USHORT2_NORM:
            if ( count <= 2 )
            {
                return VET_USHORT2_NORM;
            }
            return VET_USHORT4_NORM;

        case VET_BYTE4:
        case VET_BYTE4_NORM:
        case VET_UBYTE4:
        case VET_UBYTE4_NORM:
            return baseType;

        default:
            break;
        }
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid base type", 
            "VertexElement::multiplyTypeCount");
    }
    //--------------------------------------------------------------------------
    void VertexElement::convertColourValue(VertexElementType srcType, VertexElementType dstType, uint32* ptr)
    {
        if (srcType == dstType)
            return;

        // Conversion between ARGB and ABGR is always a case of flipping R/B
        *ptr = ((*ptr & 0x00FF0000) >> 16) | ((*ptr & 0x000000FF) << 16) | (*ptr & 0xFF00FF00);
    }
    //-----------------------------------------------------------------------------
    VertexElementType VertexElement::getBaseType(VertexElementType multiType)
    {
        switch (multiType)
        {
            case VET_FLOAT1:
            case VET_FLOAT2:
            case VET_FLOAT3:
            case VET_FLOAT4:
                return VET_FLOAT1;
            case VET_HALF1:
            case VET_HALF2:
            case VET_HALF3:
            case VET_HALF4:
                return VET_HALF1;
            case VET_DOUBLE1:
            case VET_DOUBLE2:
            case VET_DOUBLE3:
            case VET_DOUBLE4:
                return VET_DOUBLE1;
            case VET_INT1:
            case VET_INT2:
            case VET_INT3:
            case VET_INT4:
                return VET_INT1;
            case VET_UINT1:
            case VET_UINT2:
            case VET_UINT3:
            case VET_UINT4:
                return VET_UINT1;
            case VET_SHORT1:
            case VET_SHORT2:
            case VET_SHORT3:
            case VET_SHORT4:
                return VET_SHORT1;
            case VET_USHORT1:
            case VET_USHORT2:
            case VET_USHORT3:
            case VET_USHORT4:
                return VET_USHORT1;
            case VET_SHORT2_NORM:
            case VET_SHORT4_NORM:
                return VET_SHORT2_NORM;
            case VET_USHORT2_NORM:
            case VET_USHORT4_NORM:
                return VET_USHORT2_NORM;
            case VET_BYTE4:
                return VET_BYTE4;
            case VET_BYTE4_NORM:
                return VET_BYTE4_NORM;
            case VET_UBYTE4:
                return VET_UBYTE4;
            case VET_UBYTE4_NORM:
            case _DETAIL_SWAP_RB:
                return VET_UBYTE4_NORM;
            case VET_INT_10_10_10_2_NORM:
                return VET_INT_10_10_10_2_NORM;
        };
        // To keep compiler happy
        return VET_FLOAT1;
    }
    //-----------------------------------------------------------------------------
    VertexDeclaration::VertexDeclaration()
    {
    }
    //-----------------------------------------------------------------------------
    VertexDeclaration::~VertexDeclaration()
    {
    }
    //-----------------------------------------------------------------------------
    const VertexDeclaration::VertexElementList& VertexDeclaration::getElements(void) const
    {
        return mElementList;
    }
    //-----------------------------------------------------------------------------
    const VertexElement& VertexDeclaration::addElement(unsigned short source, 
        size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        mElementList.push_back(VertexElement(source, offset, theType, semantic, index));
        notifyChanged();
        return mElementList.back();
    }
    //-----------------------------------------------------------------------------
    const VertexElement& VertexDeclaration::insertElement(unsigned short atPosition,
        unsigned short source, size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        if (atPosition >= mElementList.size())
        {
            return addElement(source, offset, theType, semantic, index);
        }

        VertexElementList::iterator i = mElementList.begin();
        for (unsigned short n = 0; n < atPosition; ++n)
            ++i;

        i = mElementList.insert(i, 
            VertexElement(source, offset, theType, semantic, index));

        notifyChanged();
        return *i;
    }
    //-----------------------------------------------------------------------------
    const VertexElement* VertexDeclaration::getElement(unsigned short index) const
    {
        assert(index < mElementList.size() && "Index out of bounds");

        VertexElementList::const_iterator i = mElementList.begin();
        for (unsigned short n = 0; n < index; ++n)
            ++i;

        return &(*i);

    }
    //-----------------------------------------------------------------------------
    void VertexDeclaration::removeElement(unsigned short elem_index)
    {
        assert(elem_index < mElementList.size() && "Index out of bounds");
        VertexElementList::iterator i = mElementList.begin();
        for (unsigned short n = 0; n < elem_index; ++n)
            ++i;
        mElementList.erase(i);
        notifyChanged();
    }
    //-----------------------------------------------------------------------------
    void VertexDeclaration::removeElement(VertexElementSemantic semantic, unsigned short index)
    {
        VertexElementList::iterator ei, eiend;
        eiend = mElementList.end();
        for (ei = mElementList.begin(); ei != eiend; ++ei)
        {
            if (ei->getSemantic() == semantic && ei->getIndex() == index)
            {
                mElementList.erase(ei);
                notifyChanged();
                break;
            }
        }
    }
    //-----------------------------------------------------------------------------
    void VertexDeclaration::removeAllElements(void)
    {
        mElementList.clear();
        notifyChanged();
    }
    //-----------------------------------------------------------------------------
    void VertexDeclaration::modifyElement(unsigned short elem_index, 
        unsigned short source, size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        assert(elem_index < mElementList.size() && "Index out of bounds");
        VertexElementList::iterator i = mElementList.begin();
        std::advance(i, elem_index);
        (*i) = VertexElement(source, offset, theType, semantic, index);
        notifyChanged();
    }
    //-----------------------------------------------------------------------------
    const VertexElement* VertexDeclaration::findElementBySemantic(
        VertexElementSemantic sem, unsigned short index) const
    {
        for (auto& e : mElementList)
        {
            if (e.getSemantic() == sem && e.getIndex() == index)
            {
                return &e;
            }
        }

        return NULL;
    }
    //-----------------------------------------------------------------------------
    VertexDeclaration::VertexElementList VertexDeclaration::findElementsBySource(
        unsigned short source) const
    {
        VertexElementList retList;
        for (auto& e : mElementList)
        {
            if (e.getSource() == source)
            {
                retList.push_back(e);
            }
        }
        return retList;
    }

    //-----------------------------------------------------------------------------
    size_t VertexDeclaration::getVertexSize(unsigned short source) const
    {
        size_t sz = 0;
        for (auto& e : mElementList)
        {
            if (e.getSource() == source)
            {
                sz += e.getSize();

            }
        }
        return sz;
    }
    //-----------------------------------------------------------------------------
    VertexDeclaration* VertexDeclaration::clone(HardwareBufferManagerBase* mgr) const
    {
        HardwareBufferManagerBase* pManager = mgr ? mgr : HardwareBufferManager::getSingletonPtr(); 
        VertexDeclaration* ret = pManager->createVertexDeclaration();

        for (auto& e : mElementList)
        {
            ret->addElement(e.getSource(), e.getOffset(), e.getType(), e.getSemantic(), e.getIndex());
        }
        return ret;
    }
    //-----------------------------------------------------------------------------
    // Sort routine for VertexElement
    static bool vertexElementLess(const VertexElement& e1, const VertexElement& e2)
    {
        // Sort by source first
        if (e1.getSource() < e2.getSource())
        {
            return true;
        }
        else if (e1.getSource() == e2.getSource())
        {
            // Use ordering of semantics to sort
            if (e1.getSemantic() < e2.getSemantic())
            {
                return true;
            }
            else if (e1.getSemantic() == e2.getSemantic())
            {
                // Use index to sort
                if (e1.getIndex() < e2.getIndex())
                {
                    return true;
                }
            }
        }
        return false;
    }
    void VertexDeclaration::sort(void)
    {
        mElementList.sort(vertexElementLess);
    }
    //-----------------------------------------------------------------------------
    void VertexDeclaration::closeGapsInSource(void)
    {
        if (mElementList.empty())
            return;

        // Sort first
        sort();

        unsigned short targetIdx = 0;
        unsigned short lastIdx = getElement(0)->getSource();
        unsigned short c = 0;
        for (auto& elem : mElementList)
        {
            if (lastIdx != elem.getSource())
            {
                targetIdx++;
                lastIdx = elem.getSource();
            }
            if (targetIdx != elem.getSource())
            {
                modifyElement(c, targetIdx, elem.getOffset(), elem.getType(), 
                    elem.getSemantic(), elem.getIndex());
            }

            ++c;
        }
    }
    //-----------------------------------------------------------------------
    VertexDeclaration* VertexDeclaration::getAutoOrganisedDeclaration(
        bool skeletalAnimation, bool vertexAnimation, bool vertexAnimationNormals) const
    {
        VertexDeclaration* newDecl = this->clone();
        // Set all sources to the same buffer (for now)
        const VertexDeclaration::VertexElementList& elems = newDecl->getElements();
        unsigned short c = 0;
        for (auto& elem : elems)
        {
            // Set source & offset to 0 for now, before sort
            newDecl->modifyElement(c, 0, 0, elem.getType(), elem.getSemantic(), elem.getIndex());
            ++c;
        }
        newDecl->sort();
        // Now sort out proper buffer assignments and offsets
        size_t offset = 0;
        c = 0;
        unsigned short buffer = 0;
        VertexElementSemantic prevSemantic = VES_POSITION;
        for (auto& elem : elems)
        {
            bool splitWithPrev = false;
            bool splitWithNext = false;
            switch (elem.getSemantic())
            {
            case VES_POSITION:
                // Split positions if vertex animated with only positions
                // group with normals otherwise
                splitWithPrev = false;
                splitWithNext = vertexAnimation && !vertexAnimationNormals;
                break;
            case VES_NORMAL:
                // Normals can't share with blend weights/indices
                splitWithPrev = (prevSemantic == VES_BLEND_WEIGHTS || prevSemantic == VES_BLEND_INDICES);
                // All animated meshes have to split after normal
                splitWithNext = (skeletalAnimation || (vertexAnimation && vertexAnimationNormals));
                break;
            case VES_BLEND_WEIGHTS:
                // Blend weights/indices can be sharing with their own buffer only
                splitWithPrev = true;
                break;
            case VES_BLEND_INDICES:
                // Blend weights/indices can be sharing with their own buffer only
                splitWithNext = true;
                break;
            default:
            case VES_DIFFUSE:
            case VES_SPECULAR:
            case VES_TEXTURE_COORDINATES:
            case VES_BINORMAL:
            case VES_TANGENT:
                // Make sure position is separate if animated & there were no normals
                splitWithPrev = prevSemantic == VES_POSITION && 
                    (skeletalAnimation || vertexAnimation);
                break;
            }

            if (splitWithPrev && offset)
            {
                ++buffer;
                offset = 0;
            }

            prevSemantic = elem.getSemantic();
            newDecl->modifyElement(c, buffer, offset,
                elem.getType(), elem.getSemantic(), elem.getIndex());

            if (splitWithNext)
            {
                ++buffer;
                offset = 0;
            }
            else
            {
                offset += elem.getSize();
            }
            ++c;
        }

        return newDecl;


    }
    //-----------------------------------------------------------------------------
    unsigned short VertexDeclaration::getMaxSource(void) const
    {
        unsigned short ret = 0;
        for (auto& e : mElementList)
        {
            if (e.getSource() > ret)
            {
                ret = e.getSource();
            }
        }
        return ret;
    }
    //-----------------------------------------------------------------------------
    unsigned short VertexDeclaration::getNextFreeTextureCoordinate() const
    {
        unsigned short texCoord = 0;
        for (const auto & el : mElementList)
        {
            if (el.getSemantic() == VES_TEXTURE_COORDINATES)
            {
                ++texCoord;
            }
        }
        return texCoord;
    }
    //-----------------------------------------------------------------------------
    VertexBufferBinding::VertexBufferBinding() : mHighIndex(0)
    {
    }
    //-----------------------------------------------------------------------------
    VertexBufferBinding::~VertexBufferBinding()
    {
        unsetAllBindings();
    }
    //-----------------------------------------------------------------------------
    void VertexBufferBinding::setBinding(unsigned short index, const HardwareVertexBufferSharedPtr& buffer)
    {
        // NB will replace any existing buffer ptr at this index, and will thus cause
        // reference count to decrement on that buffer (possibly destroying it)
        mBindingMap[index] = buffer;
        mHighIndex = std::max(mHighIndex, (unsigned short)(index+1));
    }
    //-----------------------------------------------------------------------------
    void VertexBufferBinding::unsetBinding(unsigned short index)
    {
        VertexBufferBindingMap::iterator i = mBindingMap.find(index);
        if (i == mBindingMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                "Cannot find buffer binding for index " + StringConverter::toString(index),
                "VertexBufferBinding::unsetBinding");
        }
        mBindingMap.erase(i);
    }
    //-----------------------------------------------------------------------------
    void VertexBufferBinding::unsetAllBindings(void)
    {
        mBindingMap.clear();
        mHighIndex = 0;
    }
    //-----------------------------------------------------------------------------
    const VertexBufferBinding::VertexBufferBindingMap& 
    VertexBufferBinding::getBindings(void) const
    {
        return mBindingMap;
    }
    //-----------------------------------------------------------------------------
    const HardwareVertexBufferSharedPtr& VertexBufferBinding::getBuffer(unsigned short index) const
    {
        VertexBufferBindingMap::const_iterator i = mBindingMap.find(index);
        if (i == mBindingMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No buffer is bound to that index.",
                "VertexBufferBinding::getBuffer");
        }
        return i->second;
    }
    //-----------------------------------------------------------------------------
    bool VertexBufferBinding::isBufferBound(unsigned short index) const
    {
        return mBindingMap.find(index) != mBindingMap.end();
    }
    //-----------------------------------------------------------------------------
    unsigned short VertexBufferBinding::getLastBoundIndex(void) const
    {
        return mBindingMap.empty() ? 0 : mBindingMap.rbegin()->first + 1;
    }
    //-----------------------------------------------------------------------------
    bool VertexBufferBinding::hasGaps(void) const
    {
        if (mBindingMap.empty())
            return false;
        if (mBindingMap.rbegin()->first + 1 == (int) mBindingMap.size())
            return false;
        return true;
    }
    //-----------------------------------------------------------------------------
    void VertexBufferBinding::closeGaps(BindingIndexMap& bindingIndexMap)
    {
        bindingIndexMap.clear();

        VertexBufferBindingMap newBindingMap;

        VertexBufferBindingMap::const_iterator it;
        ushort targetIndex = 0;
        for (it = mBindingMap.begin(); it != mBindingMap.end(); ++it, ++targetIndex)
        {
            bindingIndexMap[it->first] = targetIndex;
            newBindingMap[targetIndex] = it->second;
        }

        mBindingMap.swap(newBindingMap);
        mHighIndex = targetIndex;
    }
}
