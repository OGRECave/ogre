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
#ifndef __HardwareBuffer__
#define __HardwareBuffer__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreException.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */
    /** Abstract class defining common features of hardware buffers.
    @remarks
        A 'hardware buffer' is any area of memory held outside of core system ram,
        and in our case refers mostly to video ram, although in theory this class
        could be used with other memory areas such as sound card memory, custom
        coprocessor memory etc.
    @par
        This reflects the fact that memory held outside of main system RAM must 
        be interacted with in a more formal fashion in order to promote
        cooperative and optimal usage of the buffers between the various 
        processing units which manipulate them.
    @par
        This abstract class defines the core interface which is common to all
        buffers, whether it be vertex buffers, index buffers, texture memory
        or framebuffer memory etc.
    @par
        Buffers have the ability to be 'shadowed' in system memory, this is because
        the kinds of access allowed on hardware buffers is not always as flexible as
        that allowed for areas of system memory - for example it is often either 
        impossible, or extremely undesirable from a performance standpoint to read from
        a hardware buffer; when writing to hardware buffers, you should also write every
        byte and do it sequentially. In situations where this is too restrictive, 
        it is possible to create a hardware, write-only buffer (the most efficient kind) 
        and to back it with a system memory 'shadow' copy which can be read and updated arbitrarily.
        Ogre handles synchronising this buffer with the real hardware buffer (which should still be
        created with the HBU_DYNAMIC flag if you intend to update it very frequently). Whilst this
        approach does have its own costs, such as increased memory overhead, these costs can 
        often be outweighed by the performance benefits of using a more hardware efficient buffer.
        You should look for the 'useShadowBuffer' parameter on the creation methods used to create
        the buffer of the type you require (see HardwareBufferManager) to enable this feature.
    */
    class _OgreExport HardwareBuffer : public BufferAlloc
    {

        public:
            /// Enums describing buffer usage; not mutually exclusive
            enum Usage
            {
                /** Static buffer which the application rarely modifies once created. Modifying
                the contents of this buffer will involve a performance hit.
                */
                HBU_STATIC = 1,
                /** Indicates the application would like to modify this buffer with the CPU
                fairly often.
                Buffers created with this flag will typically end up in AGP memory rather
                than video memory.

                This is the least optimal buffer setting.
                */
                HBU_DYNAMIC = 2,
                /** Indicates the application will never read the contents of the buffer back,
                it will only ever write data. Locking a buffer with this flag will ALWAYS
                return a pointer to new, blank memory rather than the memory associated
                with the contents of the buffer; this avoids DMA stalls because you can
                write to a new memory area while the previous one is being used.

                However, you may read from it’s shadow buffer if you set one up
                */
                HBU_WRITE_ONLY = 4,
                /** Indicates that the application will be refilling the contents
                of the buffer regularly (not just updating, but generating the
                contents from scratch), and therefore does not mind if the contents
                of the buffer are lost somehow and need to be recreated. This
                allows and additional level of optimisation on the buffer.
                This option only really makes sense when combined with
                HBU_DYNAMIC_WRITE_ONLY.
                */
                HBU_DISCARDABLE = 8,
                /** Combination of HBU_STATIC and HBU_WRITE_ONLY
                This is the optimal buffer usage setting.
                */
                HBU_STATIC_WRITE_ONLY = HBU_STATIC | HBU_WRITE_ONLY,
                /** Combination of HBU_DYNAMIC and HBU_WRITE_ONLY. If you use
                this, strongly consider using HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE
                instead if you update the entire contents of the buffer very
                regularly.
                */
                HBU_DYNAMIC_WRITE_ONLY = HBU_DYNAMIC | HBU_WRITE_ONLY,
                /** Combination of HBU_DYNAMIC, HBU_WRITE_ONLY and HBU_DISCARDABLE
                 This means that you expect to replace the entire contents of the buffer on an
                 extremely regular basis, most likely every frame. By selecting this option, you
                 free the system up from having to be concerned about losing the existing contents
                 of the buffer at any time, because if it does lose them, you will be replacing them
                 next frame anyway. On some platforms this can make a significant performance
                 difference, so you should try to use this whenever you have a buffer you need to
                 update regularly. Note that if you create a buffer this way, you should use the
                 HBL_DISCARD flag when locking the contents of it for writing.
                 */
                HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE = HBU_DYNAMIC_WRITE_ONLY | HBU_DISCARDABLE

            };
            /// Locking options
            enum LockOptions
            {
                /** Normal mode, ie allows read/write and contents are preserved.
                 This kind of lock allows reading and writing from the buffer - it’s also the least
                 optimal because basically you’re telling the card you could be doing anything at
                 all. If you’re not using a shadow buffer, it requires the buffer to be transferred
                 from the card and back again. If you’re using a shadow buffer the effect is
                 minimal.
                 */
                HBL_NORMAL,
                /** Discards the <em>entire</em> buffer while locking.
                This means you are happy for the card to discard the entire current contents of the
                buffer. Implicitly this means you are not going to read the data - it also means
                that the card can avoid any stalls if the buffer is currently being rendered from,
                because it will actually give you an entirely different one. Use this wherever
                possible when you are locking a buffer which was not created with a shadow buffer.
                If you are using a shadow buffer it matters less, although with a shadow buffer it’s
                preferable to lock the entire buffer at once, because that allows the shadow buffer
                to use HBL_DISCARD when it uploads the updated contents to the real buffer. Only
                allowed on buffers
                created with the HBU_DYNAMIC flag.
                */
                HBL_DISCARD,
                /** Lock the buffer for reading only. Not allowed in buffers which are created with
                HBU_WRITE_ONLY.
                Mandatory on static buffers, i.e. those created without the HBU_DYNAMIC flag.
                */
                HBL_READ_ONLY,
                /** As HBL_DISCARD, except the application guarantees not to overwrite any
                region of the buffer which has already been used in this frame, can allow
                some optimisation on some APIs.
                This is only useful on buffers with no shadow buffer.*/
                HBL_NO_OVERWRITE,
                /** Lock the buffer for writing only.*/
                HBL_WRITE_ONLY

            };
        protected:
            size_t mSizeInBytes;
            Usage mUsage;
            bool mIsLocked;
            size_t mLockStart;
            size_t mLockSize;
            bool mSystemMemory;
            bool mUseShadowBuffer;
            std::unique_ptr<HardwareBuffer> mShadowBuffer;
            bool mShadowUpdated;
            bool mSuppressHardwareUpdate;
            
            /// Internal implementation of lock()
            virtual void* lockImpl(size_t offset, size_t length, LockOptions options) = 0;
            /// Internal implementation of unlock()
            virtual void unlockImpl(void) = 0;

    public:
            /// Constructor, to be called by HardwareBufferManager only
            HardwareBuffer(Usage usage, bool systemMemory, bool useShadowBuffer) 
                : mSizeInBytes(0), mUsage(usage), mIsLocked(false), mLockStart(0), mLockSize(0), mSystemMemory(systemMemory),
                mUseShadowBuffer(useShadowBuffer), mShadowUpdated(false),
                mSuppressHardwareUpdate(false) 
            {
                // If use shadow buffer, upgrade to WRITE_ONLY on hardware side
                if (useShadowBuffer && usage == HBU_DYNAMIC)
                {
                    mUsage = HBU_DYNAMIC_WRITE_ONLY;
                }
                else if (useShadowBuffer && usage == HBU_STATIC)
                {
                    mUsage = HBU_STATIC_WRITE_ONLY;
                }
            }
            virtual ~HardwareBuffer() {}
            /** Lock the buffer for (potentially) reading / writing.
            @param offset The byte offset from the start of the buffer to lock
            @param length The size of the area to lock, in bytes
            @param options Locking options
            @return Pointer to the locked memory
            */
            virtual void* lock(size_t offset, size_t length, LockOptions options)
            {
                assert(!isLocked() && "Cannot lock this buffer, it is already locked!");

                void* ret = NULL;
                if ((length + offset) > mSizeInBytes)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Lock request out of bounds.",
                        "HardwareBuffer::lock");
                }
                else if (mUseShadowBuffer)
                {
                    if (options != HBL_READ_ONLY)
                    {
                        // we have to assume a read / write lock so we use the shadow buffer
                        // and tag for sync on unlock()
                        mShadowUpdated = true;
                    }

                    ret = mShadowBuffer->lock(offset, length, options);
                }
                else
                {
                    // Lock the real buffer if there is no shadow buffer 
                    ret = lockImpl(offset, length, options);
                    mIsLocked = true;
                }
                mLockStart = offset;
                mLockSize = length;
                return ret;
            }

            /// @overload
            void* lock(LockOptions options)
            {
                return this->lock(0, mSizeInBytes, options);
            }
            /** Releases the lock on this buffer. 
            @remarks 
                Locking and unlocking a buffer can, in some rare circumstances such as 
                switching video modes whilst the buffer is locked, corrupt the 
                contents of a buffer. This is pretty rare, but if it occurs, 
                this method will throw an exception, meaning you
                must re-upload the data.
            @par
                Note that using the 'read' and 'write' forms of updating the buffer does not
                suffer from this problem, so if you want to be 100% sure your
                data will not be lost, use the 'read' and 'write' forms instead.
            */
            virtual void unlock(void)
            {
                assert(isLocked() && "Cannot unlock this buffer, it is not locked!");

                // If we used the shadow buffer this time...
                if (mUseShadowBuffer && mShadowBuffer->isLocked())
                {
                    mShadowBuffer->unlock();
                    // Potentially update the 'real' buffer from the shadow buffer
                    _updateFromShadow();
                }
                else
                {
                    // Otherwise, unlock the real one
                    unlockImpl();
                    mIsLocked = false;
                }

            }

            /** Reads data from the buffer and places it in the memory pointed to by pDest.
            @param offset The byte offset from the start of the buffer to read
            @param length The size of the area to read, in bytes
            @param pDest The area of memory in which to place the data, must be large enough to 
                accommodate the data!
            */
            virtual void readData(size_t offset, size_t length, void* pDest) = 0;
            /** Writes data to the buffer from an area of system memory; note that you must
                ensure that your buffer is big enough.
            @param offset The byte offset from the start of the buffer to start writing
            @param length The size of the data to write to, in bytes
            @param pSource The source of the data to be written
            @param discardWholeBuffer If true, this allows the driver to discard the entire buffer when writing,
                such that DMA stalls can be avoided; use if you can.
            */
            virtual void writeData(size_t offset, size_t length, const void* pSource,
                    bool discardWholeBuffer = false) = 0;

            /** Copy data from another buffer into this one.
            @remarks
                Note that the source buffer must not be created with the
                usage HBU_WRITE_ONLY otherwise this will fail. 
            @param srcBuffer The buffer from which to read the copied data
            @param srcOffset Offset in the source buffer at which to start reading
            @param dstOffset Offset in the destination buffer to start writing
            @param length Length of the data to copy, in bytes.
            @param discardWholeBuffer If true, will discard the entire contents of this buffer before copying
            */
            virtual void copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
                size_t dstOffset, size_t length, bool discardWholeBuffer = false)
            {
                const void *srcData = srcBuffer.lock(
                    srcOffset, length, HBL_READ_ONLY);
                this->writeData(dstOffset, length, srcData, discardWholeBuffer);
                srcBuffer.unlock();
            }

            /** Copy all data from another buffer into this one. 
            @remarks
                Normally these buffers should be of identical size, but if they're
                not, the routine will use the smallest of the two sizes.
            */
            virtual void copyData(HardwareBuffer& srcBuffer)
            {
                size_t sz = std::min(getSizeInBytes(), srcBuffer.getSizeInBytes()); 
                copyData(srcBuffer, 0, 0, sz, true);
            }
            
            /// Updates the real buffer from the shadow buffer, if required
            virtual void _updateFromShadow(void)
            {
                if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
                {
                    // Do this manually to avoid locking problems
                    const void *srcData = mShadowBuffer->lockImpl(
                        mLockStart, mLockSize, HBL_READ_ONLY);
                    // Lock with discard if the whole buffer was locked, otherwise w/o
                    LockOptions lockOpt;
                    if (mLockStart == 0 && mLockSize == mSizeInBytes)
                        lockOpt = HBL_DISCARD;
                    else
                        lockOpt = HBL_WRITE_ONLY;
                    
                    void *destData = this->lockImpl(
                        mLockStart, mLockSize, lockOpt);
                    // Copy shadow to real
                    memcpy(destData, srcData, mLockSize);
                    this->unlockImpl();
                    mShadowBuffer->unlockImpl();
                    mShadowUpdated = false;
                }
            }

            /// Returns the size of this buffer in bytes
            size_t getSizeInBytes(void) const { return mSizeInBytes; }
            /// Returns the Usage flags with which this buffer was created
            Usage getUsage(void) const { return mUsage; }
            /// Returns whether this buffer is held in system memory
            bool isSystemMemory(void) const { return mSystemMemory; }
            /// Returns whether this buffer has a system memory shadow for quicker reading
            bool hasShadowBuffer(void) const { return mUseShadowBuffer; }
            /// Returns whether or not this buffer is currently locked.
            bool isLocked(void) const { 
                return mIsLocked || (mUseShadowBuffer && mShadowBuffer->isLocked()); 
            }
            /// Pass true to suppress hardware upload of shadow buffer changes
            void suppressHardwareUpdate(bool suppress) {
                mSuppressHardwareUpdate = suppress;
                if (!suppress)
                    _updateFromShadow();
            }




            
    };
    /** @} */
    /** @} */

    /** Locking helper. Guaranteed unlocking even in case of exception. */
    template <typename T> struct HardwareBufferLockGuard
    {
        HardwareBufferLockGuard(const T& p, HardwareBuffer::LockOptions options)
            : pBuf(p)
        {
            pData = pBuf->lock(options);
        }
        HardwareBufferLockGuard(const T& p, size_t offset, size_t length, HardwareBuffer::LockOptions options)
            : pBuf(p)
        {
            pData = pBuf->lock(offset, length, options);
        }       
        ~HardwareBufferLockGuard()
        {
            pBuf->unlock();
        }
        
        const T& pBuf;
        void* pData;
    };
}
#endif


