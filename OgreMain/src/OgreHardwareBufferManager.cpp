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

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> HardwareBufferManager* Singleton<HardwareBufferManager>::msSingleton = 0;
    HardwareBufferManager* HardwareBufferManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    HardwareBufferManager& HardwareBufferManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //---------------------------------------------------------------------
    HardwareBufferManager::HardwareBufferManager()
    {
    }
    //---------------------------------------------------------------------
    HardwareBufferManager::~HardwareBufferManager()
    {
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // Free temporary vertex buffers every 5 minutes on 100fps
    const size_t HardwareBufferManagerBase::UNDER_USED_FRAME_THRESHOLD = 30000;
    const size_t HardwareBufferManagerBase::EXPIRED_DELAY_FRAME_THRESHOLD = 5;
    //-----------------------------------------------------------------------
    HardwareBufferManagerBase::HardwareBufferManagerBase()
        : mUnderUsedFrameCount(0)
    {
    }
    //-----------------------------------------------------------------------
    HardwareBufferManagerBase::~HardwareBufferManagerBase()
    {
        // Destroy everything
        destroyAllDeclarations();
        destroyAllBindings();
        // No need to destroy main buffers - they will be destroyed by removal of bindings

        // No need to destroy temp buffers - they will be destroyed automatically.
    }
    //-----------------------------------------------------------------------
    VertexDeclaration* HardwareBufferManagerBase::createVertexDeclaration(void)
    {
        VertexDeclaration* decl = createVertexDeclarationImpl();
        OGRE_LOCK_MUTEX(mVertexDeclarationsMutex);
        mVertexDeclarations.insert(decl);
        return decl;
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::destroyVertexDeclaration(VertexDeclaration* decl)
    {
        OGRE_LOCK_MUTEX(mVertexDeclarationsMutex);
        OgreAssertDbg(mVertexDeclarations.find(decl) != mVertexDeclarations.end(), "unknown decl");
        mVertexDeclarations.erase(decl);
        destroyVertexDeclarationImpl(decl);
    }
    //-----------------------------------------------------------------------
    VertexBufferBinding* HardwareBufferManagerBase::createVertexBufferBinding(void)
    {
        VertexBufferBinding* ret = createVertexBufferBindingImpl();
        OGRE_LOCK_MUTEX(mVertexBufferBindingsMutex);
        mVertexBufferBindings.insert(ret);
        return ret;
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::destroyVertexBufferBinding(VertexBufferBinding* binding)
    {
        OGRE_LOCK_MUTEX(mVertexBufferBindingsMutex);
        OgreAssertDbg(mVertexBufferBindings.find(binding) != mVertexBufferBindings.end(),
                      "unknown binding");
        mVertexBufferBindings.erase(binding);
        destroyVertexBufferBindingImpl(binding);
    }
    //-----------------------------------------------------------------------
    VertexDeclaration* HardwareBufferManagerBase::createVertexDeclarationImpl(void)
    {
        return OGRE_NEW VertexDeclaration();
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::destroyVertexDeclarationImpl(VertexDeclaration* decl)
    {
        OGRE_DELETE decl;
    }
    //-----------------------------------------------------------------------
    VertexBufferBinding* HardwareBufferManagerBase::createVertexBufferBindingImpl(void)
    {
        return OGRE_NEW VertexBufferBinding();
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::destroyVertexBufferBindingImpl(VertexBufferBinding* binding)
    {
        OGRE_DELETE binding;
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::destroyAllDeclarations(void)
    {
        OGRE_LOCK_MUTEX(mVertexDeclarationsMutex);
        for (auto *decl : mVertexDeclarations)
        {
            destroyVertexDeclarationImpl(decl);
        }
        mVertexDeclarations.clear();
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::destroyAllBindings(void)
    {
        OGRE_LOCK_MUTEX(mVertexBufferBindingsMutex);
        for (auto *bind : mVertexBufferBindings)
        {
            destroyVertexBufferBindingImpl(bind);
        }
        mVertexBufferBindings.clear();
    }
    //-----------------------------------------------------------------------
    HardwareVertexBufferSharedPtr 
    HardwareBufferManagerBase::allocateVertexBufferCopy(
        const HardwareVertexBufferSharedPtr& sourceBuffer, 
        HardwareBufferLicensee* licensee,
        bool copyData)
    {
        // pre-lock the mVertexBuffers mutex, which would usually get locked in
        // createVertexBuffer
        // this prevents a deadlock in _notifyVertexBufferDestroyed
        // which locks the same mutexes (via other methods) but in reverse order
        OGRE_LOCK_MUTEX(mVertexBuffersMutex);
        {
            OGRE_LOCK_MUTEX(mTempBuffersMutex);
            HardwareVertexBufferSharedPtr vbuf;

            // Locate existing buffer copy in temporary vertex buffers
            FreeTemporaryVertexBufferMap::iterator i = 
                mFreeTempVertexBufferMap.find(sourceBuffer.get());
            if (i == mFreeTempVertexBufferMap.end())
            {
                // create new copy buffer, use shadow buffer and make dynamic
                vbuf = createVertexBuffer(sourceBuffer->getVertexSize(), sourceBuffer->getNumVertices(), HBU_CPU_TO_GPU,
                                          true);
            }
            else
            {
                // Allocate existing copy
                vbuf = i->second;
                mFreeTempVertexBufferMap.erase(i);
            }

            // Copy data?
            if (copyData)
            {
                vbuf->copyData(*(sourceBuffer.get()), 0, 0, sourceBuffer->getSizeInBytes(), true);
            }

            // Insert copy into licensee list
            mTempVertexBufferLicenses.emplace(
                    vbuf.get(),
                    VertexBufferLicense(sourceBuffer.get(), EXPIRED_DELAY_FRAME_THRESHOLD, vbuf, licensee));
            return vbuf;
        }

    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::releaseVertexBufferCopy(
        const HardwareVertexBufferSharedPtr& bufferCopy)
    {
        OGRE_LOCK_MUTEX(mTempBuffersMutex);
        TemporaryVertexBufferLicenseMap::iterator i =
            mTempVertexBufferLicenses.find(bufferCopy.get());
        if (i != mTempVertexBufferLicenses.end())
        {
            const VertexBufferLicense& vbl = i->second;
            vbl.licensee->licenseExpired(vbl.buffer.get());
            mFreeTempVertexBufferMap.emplace(vbl.originalBufferPtr, vbl.buffer);
            mTempVertexBufferLicenses.erase(i);
        }
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::touchVertexBufferCopy(
            const HardwareVertexBufferSharedPtr& bufferCopy)
    {
        OGRE_LOCK_MUTEX(mTempBuffersMutex);
        TemporaryVertexBufferLicenseMap::iterator i =
            mTempVertexBufferLicenses.find(bufferCopy.get());
        if (i != mTempVertexBufferLicenses.end())
        {
            VertexBufferLicense& vbl = i->second;
            vbl.expiredDelay = EXPIRED_DELAY_FRAME_THRESHOLD;
        }
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::_freeUnusedBufferCopies(void)
    {
        OGRE_LOCK_MUTEX(mTempBuffersMutex);
        size_t numFreed = 0;

        // Free unused temporary buffers
        FreeTemporaryVertexBufferMap::iterator i;
        i = mFreeTempVertexBufferMap.begin();
        while (i != mFreeTempVertexBufferMap.end())
        {
            FreeTemporaryVertexBufferMap::iterator icur = i++;
            // Free the temporary buffer that referenced by ourself only.
            // TODO: Some temporary buffers are bound to vertex buffer bindings
            // but not checked out, need to sort out method to unbind them.
            if (icur->second.use_count() <= 1)
            {
                ++numFreed;
                mFreeTempVertexBufferMap.erase(icur);
            }
        }

        StringStream str;
        if (numFreed)
        {
            str << "HardwareBufferManager: Freed " << numFreed << " unused temporary vertex buffers.";
        }
        else
        {
            str << "HardwareBufferManager: No unused temporary vertex buffers found.";
        }
        LogManager::getSingleton().logMessage(str.str(), LML_TRIVIAL);
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::_releaseBufferCopies(bool forceFreeUnused)
    {
        OGRE_LOCK_MUTEX(mTempBuffersMutex);
        size_t numUnused = mFreeTempVertexBufferMap.size();
        size_t numUsed = mTempVertexBufferLicenses.size();

        // Erase the copies which are automatic licensed out
        TemporaryVertexBufferLicenseMap::iterator i;
        i = mTempVertexBufferLicenses.begin(); 
        while (i != mTempVertexBufferLicenses.end()) 
        {
            TemporaryVertexBufferLicenseMap::iterator icur = i++;
            VertexBufferLicense& vbl = icur->second;
            if (forceFreeUnused || --vbl.expiredDelay <= 0)
            {
                vbl.licensee->licenseExpired(vbl.buffer.get());

                mFreeTempVertexBufferMap.emplace(vbl.originalBufferPtr, vbl.buffer);
                mTempVertexBufferLicenses.erase(icur);
            }
        }

        // Check whether or not free unused temporary vertex buffers.
        if (forceFreeUnused)
        {
            _freeUnusedBufferCopies();
            mUnderUsedFrameCount = 0;
        }
        else
        {
            if (numUsed < numUnused)
            {
                // Free temporary vertex buffers if too many unused for a long time.
                // Do overall temporary vertex buffers instead of per source buffer
                // to avoid overhead.
                ++mUnderUsedFrameCount;
                if (mUnderUsedFrameCount >= UNDER_USED_FRAME_THRESHOLD)
                {
                    _freeUnusedBufferCopies();
                    mUnderUsedFrameCount = 0;
                }
            }
            else
            {
                mUnderUsedFrameCount = 0;
            }
        }
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::_forceReleaseBufferCopies(
        const HardwareVertexBufferSharedPtr& sourceBuffer)
    {
        _forceReleaseBufferCopies(sourceBuffer.get());
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::_forceReleaseBufferCopies(
        HardwareVertexBuffer* sourceBuffer)
    {
        OGRE_LOCK_MUTEX(mTempBuffersMutex);
        // Erase the copies which are licensed out
        TemporaryVertexBufferLicenseMap::iterator i;
        i = mTempVertexBufferLicenses.begin();
        while (i != mTempVertexBufferLicenses.end()) 
        {
            TemporaryVertexBufferLicenseMap::iterator icur = i++;
            const VertexBufferLicense& vbl = icur->second;
            if (vbl.originalBufferPtr == sourceBuffer)
            {
                // Just tell the owner that this is being released
                vbl.licensee->licenseExpired(vbl.buffer.get());
                mTempVertexBufferLicenses.erase(icur);
            }
        }

        // Erase the free copies
        //
        // Why we need this unusual code? It's for resolve reenter problem.
        //
        // Using mFreeTempVertexBufferMap.erase(sourceBuffer) directly will
        // cause reenter into here because vertex buffer destroyed notify.
        // In most time there are no problem. But when sourceBuffer is the
        // last item of the mFreeTempVertexBufferMap, some STL multimap
        // implementation (VC and STLport) will call to clear(), which will
        // causing intermediate state of mFreeTempVertexBufferMap, in that
        // time destroyed notify back to here cause illegal accessing in
        // the end.
        //
        // For safely reason, use following code to resolve reenter problem.
        //
        typedef FreeTemporaryVertexBufferMap::iterator _Iter;
        std::pair<_Iter, _Iter> range = mFreeTempVertexBufferMap.equal_range(sourceBuffer);
        if (range.first != range.second)
        {
            std::list<HardwareVertexBufferSharedPtr> holdForDelayDestroy;
            for (_Iter it = range.first; it != range.second; ++it)
            {
                if (it->second.use_count() <= 1)
                {
                    holdForDelayDestroy.push_back(it->second);
                }
            }

            mFreeTempVertexBufferMap.erase(range.first, range.second);

            // holdForDelayDestroy will destroy auto.
        }
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManagerBase::_notifyVertexBufferDestroyed(HardwareVertexBuffer* buf)
    {
        OGRE_LOCK_MUTEX(mVertexBuffersMutex);
        VertexBufferList::iterator i = mVertexBuffers.find(buf);
        if (i != mVertexBuffers.end())
        {
            // release vertex buffer copies
            mVertexBuffers.erase(i);
            _forceReleaseBufferCopies(buf);
        }
    }
    RenderToVertexBufferSharedPtr HardwareBufferManagerBase::createRenderToVertexBuffer()
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "not supported by RenderSystem");
    }
    HardwareBufferPtr HardwareBufferManagerBase::createUniformBuffer(size_t sizeBytes,
                                                                     HardwareBufferUsage usage,
                                                                     bool useShadowBuffer)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "not supported by RenderSystem");
    }
}
