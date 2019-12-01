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
#ifndef __HardwareVertexBuffer__
#define __HardwareVertexBuffer__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreHardwareBuffer.h"
#include "OgreSharedPtr.h"
#include "OgreColourValue.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    class HardwareBufferManagerBase;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */
    /** Specialisation of HardwareBuffer for a vertex buffer. */
    class _OgreExport HardwareVertexBuffer : public HardwareBuffer
    {
        protected:

            bool mIsInstanceData;
            HardwareBufferManagerBase* mMgr;
            size_t mNumVertices;
            size_t mVertexSize;
            size_t mInstanceDataStepRate;           
            /// Checks if vertex instance data is supported by the render system
            virtual bool checkIfVertexInstanceDataIsSupported();

        public:
            /// Should be called by HardwareBufferManager
            HardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize, size_t numVertices,
                HardwareBuffer::Usage usage, bool useSystemMemory, bool useShadowBuffer);
            ~HardwareVertexBuffer();
            /// Return the manager of this buffer, if any
            HardwareBufferManagerBase* getManager() const { return mMgr; }
            /// Gets the size in bytes of a single vertex in this buffer
            size_t getVertexSize(void) const { return mVertexSize; }
            /// Get the number of vertices in this buffer
            size_t getNumVertices(void) const { return mNumVertices; }
            /// Get if this vertex buffer is an "instance data" buffer (per instance)
            bool isInstanceData() const { return mIsInstanceData; }
            /// Set if this vertex buffer is an "instance data" buffer (per instance)
            void setIsInstanceData(const bool val);
            /// Get the number of instances to draw using the same per-instance data before advancing in the buffer by one element.
            size_t getInstanceDataStepRate() const;
            /// Set the number of instances to draw using the same per-instance data before advancing in the buffer by one element.
            void setInstanceDataStepRate(const size_t val);


            // NB subclasses should override lock, unlock, readData, writeData

    };

    /// @deprecated use HardwareBufferLockGuard directly
    OGRE_DEPRECATED typedef HardwareBufferLockGuard HardwareVertexBufferLockGuard;

    /// Vertex element semantics, used to identify the meaning of vertex buffer contents
    enum VertexElementSemantic {
        /// Position, 3 reals per vertex
        VES_POSITION = 1,
        /// Blending weights
        VES_BLEND_WEIGHTS = 2,
        /// Blending indices
        VES_BLEND_INDICES = 3,
        /// Normal, 3 reals per vertex
        VES_NORMAL = 4,
        /// Diffuse colours
        VES_DIFFUSE = 5,
        /// Specular colours
        VES_SPECULAR = 6,
        /// Texture coordinates
        VES_TEXTURE_COORDINATES = 7,
        /// Binormal (Y axis if normal is Z)
        VES_BINORMAL = 8,
        /// Tangent (X axis if normal is Z)
        VES_TANGENT = 9,
        /// The  number of VertexElementSemantic elements (note - the first value VES_POSITION is 1) 
        VES_COUNT = 9
    };

    /**
     * Vertex element type, used to identify the base types of the vertex contents
     *
     * @note VET_SHORT1, VET_SHORT3, VET_USHORT1 and VET_USHORT3 should never be used
     * because they aren't supported on any known hardware - they are unaligned as their size
     * is not a multiple of 4 bytes. Therefore drivers usually must add padding on upload.
     */
    enum VertexElementType
    {
        VET_FLOAT1 = 0,
        VET_FLOAT2 = 1,
        VET_FLOAT3 = 2,
        VET_FLOAT4 = 3,
        /// alias to more specific colour type - use the current rendersystem's colour packing
        VET_COLOUR = 4,
        VET_SHORT1 = 5,  ///< @deprecated (see #VertexElementType note)
        VET_SHORT2 = 6,
        VET_SHORT3 = 7,  ///< @deprecated (see #VertexElementType note)
        VET_SHORT4 = 8,
        VET_UBYTE4 = 9,
        /// D3D style compact colour
        VET_COLOUR_ARGB = 10,
        /// GL style compact colour
        VET_COLOUR_ABGR = 11,

        // the following are not universally supported on all hardware:
        VET_DOUBLE1 = 12,
        VET_DOUBLE2 = 13,
        VET_DOUBLE3 = 14,
        VET_DOUBLE4 = 15,
        VET_USHORT1 = 16,  ///< @deprecated (see #VertexElementType note)
        VET_USHORT2 = 17,
        VET_USHORT3 = 18,  ///< @deprecated (see #VertexElementType note)
        VET_USHORT4 = 19,
        VET_INT1 = 20,
        VET_INT2 = 21,
        VET_INT3 = 22,
        VET_INT4 = 23,
        VET_UINT1 = 24,
        VET_UINT2 = 25,
        VET_UINT3 = 26,
        VET_UINT4 = 27,
        VET_BYTE4 = 28,  /// signed bytes
        VET_BYTE4_NORM = 29,   /// signed bytes (normalized to -1..1)
        VET_UBYTE4_NORM = 30,  /// unsigned bytes (normalized to 0..1)
        VET_SHORT2_NORM = 31,  /// signed shorts (normalized to -1..1)
        VET_SHORT4_NORM = 32,
        VET_USHORT2_NORM = 33, /// unsigned shorts (normalized to 0..1)
        VET_USHORT4_NORM = 34
    };

    /** This class declares the usage of a single vertex buffer as a component
        of a complete VertexDeclaration.
        @remarks
        Several vertex buffers can be used to supply the input geometry for a
        rendering operation, and in each case a vertex buffer can be used in
        different ways for different operations; the buffer itself does not
        define the semantics (position, normal etc), the VertexElement
        class does.
    */
    class _OgreExport VertexElement : public VertexDataAlloc
    {
    protected:
        /// The source vertex buffer, as bound to an index using VertexBufferBinding
        unsigned short mSource;
        /// Index of the item, only applicable for some elements like texture coords
        unsigned short mIndex;
        /// The offset in the buffer that this element starts at
        size_t mOffset;
        /// The type of element
        VertexElementType mType;
        /// The meaning of the element
        VertexElementSemantic mSemantic;
    public:
        /// Constructor, should not be called directly, only needed because of list
        VertexElement() {}
        /// Constructor, should not be called directly, call VertexDeclaration::addElement
        VertexElement(unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);
        /// Gets the vertex buffer index from where this element draws it's values
        unsigned short getSource(void) const { return mSource; }
        /// Gets the offset into the buffer where this element starts
        size_t getOffset(void) const { return mOffset; }
        /// Gets the data format of this element
        VertexElementType getType(void) const { return mType; }
        /// Gets the meaning of this element
        VertexElementSemantic getSemantic(void) const { return mSemantic; }
        /// Gets the index of this element, only applicable for repeating elements
        unsigned short getIndex(void) const { return mIndex; }
        /// Gets the size of this element in bytes
        size_t getSize(void) const;
        /// Utility method for helping to calculate offsets
        static size_t getTypeSize(VertexElementType etype);
        /// Utility method which returns the count of values in a given type (result for colors may be counter-intuitive)
        static unsigned short getTypeCount(VertexElementType etype);
        /** Simple converter function which will return a type large enough to hold 'count' values
            of the same type as the values in 'baseType'.  The 'baseType' parameter should have the
            smallest count available.  The return type may have the count rounded up to the next multiple
            of 4 bytes.  Byte types will always return a 4-count type, while short types will return either
            a 2-count or 4-count type.
        */
        static VertexElementType multiplyTypeCount(VertexElementType baseType, unsigned short count);
        /** Simple converter function which will turn a type into it's single-value (or lowest multiple-value)
            equivalent - makes switches on type easier.  May give counter-intuitive results with bytes or shorts.
        */
        static VertexElementType getBaseType(VertexElementType multiType);

        /** Utility method for converting colour from
            one packed 32-bit colour type to another.
        @param srcType The source type
        @param dstType The destination type
        @param ptr Read / write value to change
        */
        static void convertColourValue(VertexElementType srcType,
            VertexElementType dstType, uint32* ptr);

        /** Utility method for converting colour to
            a packed 32-bit colour type.
        @param src source colour
        @param dst The destination type
        */
        static uint32 convertColourValue(const ColourValue& src,
            VertexElementType dst);

        /** Utility method to get the most appropriate packed colour vertex element format. */
        static VertexElementType getBestColourVertexElementType(void);

        inline bool operator== (const VertexElement& rhs) const
        {
            if (mType != rhs.mType ||
                mIndex != rhs.mIndex ||
                mOffset != rhs.mOffset ||
                mSemantic != rhs.mSemantic ||
                mSource != rhs.mSource)
                return false;
            else
                return true;

        }
        /** Adjusts a pointer to the base of a vertex to point at this element.
        @remarks
            This variant is for void pointers, passed as a parameter because we can't
            rely on covariant return types.
        @param pBase Pointer to the start of a vertex in this buffer.
        @param pElem Pointer to a pointer which will be set to the start of this element.
        */
        inline void baseVertexPointerToElement(void* pBase, void** pElem) const
        {
            // The only way we can do this is to cast to char* in order to use byte offset
            // then cast back to void*.
            *pElem = static_cast<void*>(
                static_cast<unsigned char*>(pBase) + mOffset);
        }
        /** Adjusts a pointer to the base of a vertex to point at this element.
        @remarks
            This variant is for float pointers, passed as a parameter because we can't
            rely on covariant return types.
        @param pBase Pointer to the start of a vertex in this buffer.
        @param pElem Pointer to a pointer which will be set to the start of this element.
        */
        inline void baseVertexPointerToElement(void* pBase, float** pElem) const
        {
            // The only way we can do this is to cast to char* in order to use byte offset
            // then cast back to float*. However we have to go via void* because casting
            // directly is not allowed
            *pElem = static_cast<float*>(
                static_cast<void*>(
                    static_cast<unsigned char*>(pBase) + mOffset));
        }

        /** Adjusts a pointer to the base of a vertex to point at this element.
        @remarks
            This variant is for RGBA pointers, passed as a parameter because we can't
            rely on covariant return types.
        @param pBase Pointer to the start of a vertex in this buffer.
        @param pElem Pointer to a pointer which will be set to the start of this element.
        */
        inline void baseVertexPointerToElement(void* pBase, RGBA** pElem) const
        {
            *pElem = static_cast<RGBA*>(
                static_cast<void*>(
                    static_cast<unsigned char*>(pBase) + mOffset));
        }
        /** Adjusts a pointer to the base of a vertex to point at this element.
        @remarks
            This variant is for char pointers, passed as a parameter because we can't
            rely on covariant return types.
        @param pBase Pointer to the start of a vertex in this buffer.
        @param pElem Pointer to a pointer which will be set to the start of this element.
        */
        inline void baseVertexPointerToElement(void* pBase, unsigned char** pElem) const
        {
            *pElem = static_cast<unsigned char*>(pBase) + mOffset;
        }

        /** Adjusts a pointer to the base of a vertex to point at this element.
        @remarks
        This variant is for ushort pointers, passed as a parameter because we can't
        rely on covariant return types.
        @param pBase Pointer to the start of a vertex in this buffer.
        @param pElem Pointer to a pointer which will be set to the start of this element.
        */
        inline void baseVertexPointerToElement(void* pBase, unsigned short** pElem) const
        {
            *pElem = static_cast<unsigned short*>(
                static_cast<void*>(
                    static_cast<unsigned char*>(pBase) + mOffset));
        }


    };
    /** This class declares the format of a set of vertex inputs, which
        can be issued to the rendering API through a RenderOperation.

    The ordering is important on Direct3D9 with Direct3D 7 grade cards.
    Calling closeGapsInSource() will format this VertexDeclaration accordingly.

    Whilst GL and more modern graphics cards in D3D will allow you to defy these rules,
    sticking to them will reduce state changes and improve performance on modern APIs as well.

    Like the other classes in this functional area, these declarations should be created and
    destroyed using the HardwareBufferManager.
    */
    class _OgreExport VertexDeclaration : public VertexDataAlloc
    {
    public:
        /// Defines the list of vertex elements that makes up this declaration
        typedef std::list<VertexElement> VertexElementList;
        /// Sort routine for vertex elements
        static bool vertexElementLess(const VertexElement& e1, const VertexElement& e2);
    protected:
        VertexElementList mElementList;

        /** Notify derived class that it is time to invalidate cached state, such as VAO or ID3D11InputLayout */
        virtual void notifyChanged() {}
    public:
        /// Standard constructor, not you should use HardwareBufferManager::createVertexDeclaration
        VertexDeclaration();
        virtual ~VertexDeclaration();

        /** Get the number of elements in the declaration. */
        size_t getElementCount(void) const { return mElementList.size(); }
        /** Gets read-only access to the list of vertex elements. */
        const VertexElementList& getElements(void) const;
        /** Get a single element. */
        const VertexElement* getElement(unsigned short index) const;

        /** Sorts the elements in this list to be compatible with D3D7 graphics cards

           the order is as follows: position, blending weights, normals, diffuse colours, specular colours,
           texture coordinates
        */
        void sort(void);

        /** Remove any gaps in the source buffer list used by this declaration.
        @remarks
            This is useful if you've modified a declaration and want to remove
            any gaps in the list of buffers being used. Note, however, that if this
            declaration is already being used with a VertexBufferBinding, you will
            need to alter that too. This method is mainly useful when reorganising
            buffers based on an altered declaration.
        @note
            This will also call sort()
        */
        void closeGapsInSource(void);

        /** Generates a new VertexDeclaration for optimal usage based on the current
            vertex declaration, which can be used with VertexData::reorganiseBuffers later
            if you wish, or simply used as a template.
        @remarks
            Different buffer organisations and buffer usages will be returned
            depending on the parameters passed to this method.
        @param skeletalAnimation Whether this vertex data is going to be
            skeletally animated
        @param vertexAnimation Whether this vertex data is going to be vertex animated
        @param vertexAnimationNormals Whether vertex data animation is going to include normals animation
        */
        VertexDeclaration* getAutoOrganisedDeclaration(bool skeletalAnimation,
            bool vertexAnimation, bool vertexAnimationNormals) const;

        /** Gets the index of the highest source value referenced by this declaration. */
        unsigned short getMaxSource(void) const;



        /** Adds a new VertexElement to this declaration.
        @remarks
            This method adds a single element (positions, normals etc) to the end of the
            vertex declaration. <b>Please read the information in VertexDeclaration about
        the importance of ordering and structure for compatibility with older D3D drivers</b>.
        @param source The binding index of HardwareVertexBuffer which will provide the source for this element.
            See VertexBufferBinding for full information.
        @param offset The offset in bytes where this element is located in the buffer
        @param theType The data format of the element (3 floats, a colour etc)
        @param semantic The meaning of the data (position, normal, diffuse colour etc)
        @param index Optional index for multi-input elements like texture coordinates
        @return A reference to the VertexElement added.
        */
        const VertexElement& addElement(unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);
        /** Inserts a new VertexElement at a given position in this declaration.
        @remarks
        This method adds a single element (positions, normals etc) at a given position in this
        vertex declaration. <b>Please read the information in VertexDeclaration about
        the importance of ordering and structure for compatibility with older D3D drivers</b>.
        @param source The binding index of HardwareVertexBuffer which will provide the source for this element.
        See VertexBufferBinding for full information.
        @param offset The offset in bytes where this element is located in the buffer
        @param theType The data format of the element (3 floats, a colour etc)
        @param semantic The meaning of the data (position, normal, diffuse colour etc)
        @param index Optional index for multi-input elements like texture coordinates
        @return A reference to the VertexElement added.
        */
        const VertexElement& insertElement(unsigned short atPosition,
            unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);

        /** Remove the element at the given index from this declaration. */
        void removeElement(unsigned short elem_index);

        /** Remove the element with the given semantic and usage index.
        @remarks
            In this case 'index' means the usage index for repeating elements such
            as texture coordinates. For other elements this will always be 0 and does
            not refer to the index in the vector.
        */
        void removeElement(VertexElementSemantic semantic, unsigned short index = 0);

        /** Remove all elements. */
        void removeAllElements(void);

        /** Modify an element in-place, params as addElement.
       @remarks
       <b>Please read the information in VertexDeclaration about
        the importance of ordering and structure for compatibility with older D3D drivers</b>.
     */
        void modifyElement(unsigned short elem_index, unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);

        /** Finds a VertexElement with the given semantic, and index if there is more than
            one element with the same semantic.
        @remarks
            If the element is not found, this method returns null.
        */
        const VertexElement* findElementBySemantic(VertexElementSemantic sem, unsigned short index = 0) const;
        /** Based on the current elements, gets the size of the vertex for a given buffer source.
        @param source The buffer binding index for which to get the vertex size.
        */

        /** Gets a list of elements which use a given source.
        @remarks
            Note that the list of elements is returned by value therefore is separate from
            the declaration as soon as this method returns.
        */
        VertexElementList findElementsBySource(unsigned short source) const;

        /** Gets the vertex size defined by this declaration for a given source. */
        size_t getVertexSize(unsigned short source) const;
        
        /** Return the index of the next free texture coordinate set which may be added
            to this declaration.
        */
        unsigned short getNextFreeTextureCoordinate() const;

        /** Clones this declaration. 
        @param mgr Optional HardwareBufferManager to use for creating the clone
            (if null, use the current default).
        */
        VertexDeclaration* clone(HardwareBufferManagerBase* mgr = 0) const OGRE_NODISCARD;

        inline bool operator== (const VertexDeclaration& rhs) const
        {
            if (mElementList.size() != rhs.mElementList.size())
                return false;

            VertexElementList::const_iterator i, iend, rhsi, rhsiend;
            iend = mElementList.end();
            rhsiend = rhs.mElementList.end();
            rhsi = rhs.mElementList.begin();
            for (i = mElementList.begin(); i != iend && rhsi != rhsiend; ++i, ++rhsi)
            {
                if ( !(*i == *rhsi) )
                    return false;
            }

            return true;
        }
        inline bool operator!= (const VertexDeclaration& rhs) const
        {
            return !(*this == rhs);
        }

    };

    /** Records the state of all the vertex buffer bindings required to provide a vertex declaration
        with the input data it needs for the vertex elements.
    @remarks
        Why do we have this binding list rather than just have VertexElement referring to the
        vertex buffers direct? Well, in the underlying APIs, binding the vertex buffers to an
        index (or 'stream') is the way that vertex data is linked, so this structure better
        reflects the realities of that. In addition, by separating the vertex declaration from
        the list of vertex buffer bindings, it becomes possible to reuse bindings between declarations
        and vice versa, giving opportunities to reduce the state changes required to perform rendering.
    @par
        Like the other classes in this functional area, these binding maps should be created and
        destroyed using the HardwareBufferManager.
    */
    class _OgreExport VertexBufferBinding : public VertexDataAlloc
    {
    public:
        /// Defines the vertex buffer bindings used as source for vertex declarations
        typedef std::map<unsigned short, HardwareVertexBufferSharedPtr> VertexBufferBindingMap;
    protected:
        VertexBufferBindingMap mBindingMap;
        mutable unsigned short mHighIndex;
    public:
        /// Constructor, should not be called direct, use HardwareBufferManager::createVertexBufferBinding
        VertexBufferBinding();
        ~VertexBufferBinding();
        /** Set a binding, associating a vertex buffer with a given index.
        @remarks
            If the index is already associated with a vertex buffer,
            the association will be replaced. This may cause the old buffer
            to be destroyed if nothing else is referring to it.
            You should assign bindings from 0 and not leave gaps, although you can
            bind them in any order.
        */
        void setBinding(unsigned short index, const HardwareVertexBufferSharedPtr& buffer);
        /** Removes an existing binding. */
        void unsetBinding(unsigned short index);

        /** Removes all the bindings. */
        void unsetAllBindings(void);

        /// Gets a read-only version of the buffer bindings
        const VertexBufferBindingMap& getBindings(void) const;

        /// Gets the buffer bound to the given source index
        const HardwareVertexBufferSharedPtr& getBuffer(unsigned short index) const;
        /// Gets whether a buffer is bound to the given source index
        bool isBufferBound(unsigned short index) const;

        size_t getBufferCount(void) const { return mBindingMap.size(); }

        /** Gets the highest index which has already been set, plus 1.
        @remarks
            This is to assist in binding the vertex buffers such that there are
            not gaps in the list.
        */
        unsigned short getNextIndex(void) const { return mHighIndex++; }

        /** Gets the last bound index.
        */
        unsigned short getLastBoundIndex(void) const;

        typedef std::map<ushort, ushort> BindingIndexMap;

        /** Check whether any gaps in the bindings.
        */
        bool hasGaps(void) const;

        /** Remove any gaps in the bindings.
        @remarks
            This is useful if you've removed vertex buffer from this vertex buffer
            bindings and want to remove any gaps in the bindings. Note, however,
            that if this bindings is already being used with a VertexDeclaration,
            you will need to alter that too. This method is mainly useful when
            reorganising buffers manually.
        @param
            bindingIndexMap To be retrieve the binding index map that used to
            translation old index to new index; will be cleared by this method
            before fill-in.
        */
        void closeGaps(BindingIndexMap& bindingIndexMap);

        /// Returns true if this binding has an element that contains instance data
        bool hasInstanceData() const;


    };
    /** @} */
    /** @} */



}

#include "OgreHeaderSuffix.h"

#endif

