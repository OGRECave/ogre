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
#ifndef __HardwareBufferManager__
#define __HardwareBufferManager__

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreSingleton.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreHardwareVertexBuffer.h"
#include "Threading/OgreThreadHeaders.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */

    /** Abstract interface representing a 'licensee' of a hardware buffer copy.

        Often it's useful to have temporary buffers which are used for working
        but are not necessarily needed permanently. However, creating and 
        destroying buffers is expensive, so we need a way to share these 
        working areas, especially those based on existing fixed buffers. 
        This class represents a licensee of one of those temporary buffers, 
        and must be implemented by any user of a temporary buffer if they 
        wish to be notified when the license is expired. 
    */
    class _OgreExport HardwareBufferLicensee
    {
    public:
        virtual ~HardwareBufferLicensee() { }
        /** This method is called when the buffer license is expired and is about
            to be returned to the shared pool.
        */
        virtual void licenseExpired(HardwareBuffer* buffer) = 0;
    };

    /** Base definition of a hardware buffer manager.

        This class is deliberately not a Singleton, so that multiple types can 
        exist at once (notably DefaultHardwareBufferManagerBase).
        The Singleton is add via the inheritance in HardwareBufferManager below.
    */
    class _OgreExport HardwareBufferManagerBase : public BufferAlloc
    {
    protected:
        /** WARNING: The following member should place before all other members.
            Members destruct order is very important here, because destructing other
            members will cause notify back to this class, and then will access to this
            two members.
        */
        typedef std::set<HardwareVertexBuffer*> VertexBufferList;
        typedef std::set<HardwareIndexBuffer*> IndexBufferList;
        VertexBufferList mVertexBuffers;


        typedef std::set<VertexDeclaration*> VertexDeclarationList;
        typedef std::set<VertexBufferBinding*> VertexBufferBindingList;
        VertexDeclarationList mVertexDeclarations;
        VertexBufferBindingList mVertexBufferBindings;

        // Mutexes
        OGRE_MUTEX(mVertexBuffersMutex);
        OGRE_MUTEX(mVertexDeclarationsMutex);
        OGRE_MUTEX(mVertexBufferBindingsMutex);

        /// Internal method for destroys all vertex declarations.
        void destroyAllDeclarations(void);
        /// Internal method for destroys all vertex buffer bindings.
        void destroyAllBindings(void);

    private:
        /// Internal method for creates a new vertex declaration, may be overridden by certain rendering APIs.
        virtual VertexDeclaration* createVertexDeclarationImpl(void);
        /// Internal method for destroys a vertex declaration, may be overridden by certain rendering APIs.
        void destroyVertexDeclarationImpl(VertexDeclaration* decl);

        /// Internal method for creates a new VertexBufferBinding, may be overridden by certain rendering APIs.
        virtual VertexBufferBinding* createVertexBufferBindingImpl(void);
        /// Internal method for destroys a VertexBufferBinding, may be overridden by certain rendering APIs.
        void destroyVertexBufferBindingImpl(VertexBufferBinding* binding);

        /** Struct holding details of a license to use a temporary shared buffer. */
        class _OgrePrivate VertexBufferLicense
        {
        public:
            HardwareVertexBuffer* originalBufferPtr;
            size_t expiredDelay;
            HardwareVertexBufferSharedPtr buffer;
            HardwareBufferLicensee* licensee;
            VertexBufferLicense(
                HardwareVertexBuffer* orig,
                size_t delay,
                const HardwareVertexBufferSharedPtr& buf,
                HardwareBufferLicensee* lic) 
                : originalBufferPtr(orig)
                , expiredDelay(delay)
                , buffer(buf)
                , licensee(lic)
            {}

        };

        /// Map from original buffer to temporary buffers.
        typedef std::multimap<HardwareVertexBuffer*, HardwareVertexBufferSharedPtr> FreeTemporaryVertexBufferMap;
        /// Map of current available temp buffers.
        FreeTemporaryVertexBufferMap mFreeTempVertexBufferMap;
        /// Map from temporary buffer to details of a license.
        typedef std::map<HardwareVertexBuffer*, VertexBufferLicense> TemporaryVertexBufferLicenseMap;
        /// Map of currently licensed temporary buffers.
        TemporaryVertexBufferLicenseMap mTempVertexBufferLicenses;
        /// Number of frames elapsed since temporary buffers utilization was above half the available.
        size_t mUnderUsedFrameCount;
        /// Number of frames to wait before free unused temporary buffers.
        static const size_t UNDER_USED_FRAME_THRESHOLD;
        /// Frame delay for temporary buffers.
        static const size_t EXPIRED_DELAY_FRAME_THRESHOLD;
        // Mutexes
        OGRE_MUTEX(mTempBuffersMutex);

        void _forceReleaseBufferCopies(HardwareVertexBuffer* sourceBuffer);
    public:
        HardwareBufferManagerBase();
        virtual ~HardwareBufferManagerBase();
        /** Create a hardware vertex buffer.

            This method creates a new vertex buffer; this will act as a source of geometry
            data for rendering objects. Note that because the meaning of the contents of
            the vertex buffer depends on the usage, this method does not specify a
            vertex format; the user of this buffer can actually insert whatever data 
            they wish, in any format. However, in order to use this with a RenderOperation,
            the data in this vertex buffer will have to be associated with a semantic element
            of the rendering pipeline, e.g. a position, or texture coordinates. This is done 
            using the VertexDeclaration class, which itself contains VertexElement structures
            referring to the source data.
            Note that because vertex buffers can be shared, they are reference
            counted so you do not need to worry about destroying them this will be done
            automatically.
        @param vertexSize
            The size in bytes of each vertex in this buffer; you must calculate
            this based on the kind of data you expect to populate this buffer with.
        @param numVerts
            The number of vertices in this buffer.
        @param usage
            One or more members of the #HardwareBufferUsage enumeration; you are
            strongly advised to use #HBU_GPU_ONLY wherever possible, if you need to
            update regularly, consider #HBU_CPU_TO_GPU or useShadowBuffer=true.
        @param useShadowBuffer
            If set to @c true, this buffer will be 'shadowed' by one stored in 
            system memory rather than GPU memory. See @ref Shadow-Buffers.
        */
        virtual HardwareVertexBufferSharedPtr 
            createVertexBuffer(size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage, 
            bool useShadowBuffer = false) = 0;
        /** Create a hardware index buffer.
        @remarks Note that because buffers can be shared, they are reference
            counted so you do not need to worry about destroying them this will be done
            automatically.
        @param itype
            The type in index, either 16- or 32-bit, depending on how many vertices
            you need to be able to address
        @param numIndexes
            The number of indexes in the buffer
        @param usage
            One or more members of the #HardwareBufferUsage enumeration.
        @param useShadowBuffer
            If set to @c true, this buffer will be 'shadowed' by one stored in 
            system memory rather than GPU memory. See @ref Shadow-Buffers.
        */
        virtual HardwareIndexBufferSharedPtr 
            createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
            HardwareBuffer::Usage usage, bool useShadowBuffer = false) = 0;

        /** Create a render to vertex buffer.
        @remarks The parameters (such as vertex size etc) are determined later
            and are allocated when needed.
        */
        virtual RenderToVertexBufferSharedPtr createRenderToVertexBuffer();

        /**
         * Create uniform buffer. This type of buffer allows the upload of shader constants once,
         * and sharing between shader stages or even shaders from another materials. 
         * The update shall be triggered by GpuProgramParameters, if is dirty
         */
        virtual HardwareBufferPtr createUniformBuffer(size_t sizeBytes,
                                                      HardwareBufferUsage usage = HBU_CPU_TO_GPU,
                                                      bool useShadowBuffer = false);

        /** Creates a new vertex declaration. */
        VertexDeclaration* createVertexDeclaration(void);
        /** Destroys a vertex declaration. */
        void destroyVertexDeclaration(VertexDeclaration* decl);

        /** Creates a new VertexBufferBinding. */
        VertexBufferBinding* createVertexBufferBinding(void);
        /** Destroys a VertexBufferBinding. */
        void destroyVertexBufferBinding(VertexBufferBinding* binding);

        /** Allocates a copy of a given vertex buffer.

            This method allocates a temporary copy of an existing vertex buffer.
            This buffer is subsequently stored and can be made available for 
            other purposes later without incurring the cost of construction / 
            destruction.
        @param sourceBuffer
            The source buffer to use as a copy.
        @param licensee
            Pointer back to the class requesting the copy, which must
            implement HardwareBufferLicense in order to be notified when the license
            expires.
        @param copyData
            If @c true, the current data is copied as well as the 
            structure of the buffer/
        */
        HardwareVertexBufferSharedPtr allocateVertexBufferCopy(
            const HardwareVertexBufferSharedPtr& sourceBuffer, 
            HardwareBufferLicensee* licensee,
            bool copyData = false);

        /** Manually release a vertex buffer copy for others to subsequently use.

        @param bufferCopy
            The buffer copy. The caller is expected to delete
            or at least no longer use this reference, since another user may
            well begin to modify the contents of the buffer.
        */
        void releaseVertexBufferCopy(const HardwareVertexBufferSharedPtr& bufferCopy);

        /** Tell engine that the vertex buffer copy intent to reuse.

            Ogre internal keep an expired delay counter.
            When the counter count down to zero, it'll release for other
            purposes later. But you can use this function to reset the counter to
            the internal configured value, keep the buffer not get released for
            some frames.
        @param bufferCopy
            The buffer copy. The caller is expected to keep this
            buffer copy for use.
        */
        void touchVertexBufferCopy(const HardwareVertexBufferSharedPtr& bufferCopy);

        /** Free all unused vertex buffer copies.

            This method free all temporary vertex buffers that not in used.
            In normally, temporary vertex buffers are subsequently stored and can
            be made available for other purposes later without incurring the cost
            of construction / destruction. But in some cases you want to free them
            to save hardware memory (e.g. application was runs in a long time, you
            might free temporary buffers periodically to avoid memory overload).
        */
        void _freeUnusedBufferCopies(void);

        /** Internal method for releasing all temporary buffers; is called by OGRE.
        @param forceFreeUnused
            If @c true, free all unused temporary buffers.
            If @c false, auto detect and free all unused temporary buffers based on
            temporary buffers utilization.
        */
        void _releaseBufferCopies(bool forceFreeUnused = false);

        /** Internal method that forces the release of copies of a given buffer.

            This usually means that the buffer which the copies are based on has
            been changed in some fundamental way, and the owner of the original 
            wishes to make that known so that new copies will reflect the
            changes.
        @param sourceBuffer
            The source buffer as a shared pointer.  Any buffer copies created
            from the source buffer are deleted.
        */
        void _forceReleaseBufferCopies(const HardwareVertexBufferSharedPtr& sourceBuffer);

        /// Notification that a hardware vertex buffer has been destroyed.
        void _notifyVertexBufferDestroyed(HardwareVertexBuffer* buf);
    };

    /** Singleton wrapper for hardware buffer manager. */
    class _OgreExport HardwareBufferManager : public HardwareBufferManagerBase, public Singleton<HardwareBufferManager>
    {
    public:
        HardwareBufferManager();
        ~HardwareBufferManager();

        /// @copydoc Singleton::getSingleton()
        static HardwareBufferManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static HardwareBufferManager* getSingletonPtr(void);

    };

    /** @} */
    /** @} */
} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __HardwareBufferManager__

