/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __VertexIndexData_H__
#define __VertexIndexData_H__

#include "OgrePrerequisites.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre {
	
	/// Define a list of usage flags
	typedef std::vector<HardwareBuffer::Usage> BufferUsageList;


	/** Summary class collecting together vertex source information. */
	class _OgreExport VertexData : public VertexDataAlloc
	{
    private:
        /// Protected copy constructor, to prevent misuse
        VertexData(const VertexData& rhs); /* do nothing, should not use */
        /// Protected operator=, to prevent misuse
        VertexData& operator=(const VertexData& rhs); /* do not use */
    public:
        VertexData();
        ~VertexData();

		/** Declaration of the vertex to be used in this operation. 
		@remarks Note that this is created for you on construction.
		*/
		VertexDeclaration* vertexDeclaration;
		/** The vertex buffer bindings to be used. 
		@remarks Note that this is created for you on construction.
		*/
		VertexBufferBinding* vertexBufferBinding;
		/// The base vertex index to start from
		size_t vertexStart;
		/// The number of vertices used in this operation
		size_t vertexCount;


		/// Struct used to hold hardware morph / pose vertex data information
		struct HardwareAnimationData
		{
			const VertexElement* targetVertexElement;
			Real parametric;
		};
		typedef std::vector<HardwareAnimationData> HardwareAnimationDataList;
		/// VertexElements used for hardware morph / pose animation
		HardwareAnimationDataList hwAnimationDataList;
		/// Number of hardware animation data items used
		size_t hwAnimDataItemsUsed;
		
		/** Clones this vertex data, potentially including replicating any vertex buffers.
		@remarks The caller is expected to delete the returned pointer when ready
		*/
		VertexData* clone(bool copyData = true) const;

        /** Modifies the vertex data to be suitable for use for rendering shadow geometry.
        @remarks
            Preparing vertex data to generate a shadow volume involves firstly ensuring that the 
            vertex buffer containing the positions is a standalone vertex buffer,
            with no other components in it. This method will therefore break apart any existing
            vertex buffers if position is sharing a vertex buffer. 
            Secondly, it will double the size of this vertex buffer so that there are 2 copies of 
            the position data for the mesh. The first half is used for the original, and the second 
            half is used for the 'extruded' version. The vertex count used to render will remain 
            the same though, so as not to add any overhead to regular rendering of the object.
            Both copies of the position are required in one buffer because shadow volumes stretch 
            from the original mesh to the extruded version. 
        @par
            It's important to appreciate that this method can fundamentally change the structure of your
            vertex buffers, although in reality they will be new buffers. As it happens, if other 
            objects are using the original buffers then they will be unaffected because the reference
            counting will keep them intact. However, if you have made any assumptions about the 
            structure of the vertex data in the buffers of this object, you may have to rethink them.
        */
        void prepareForShadowVolume(void);

        /** Additional shadow volume vertex buffer storage. 
        @remarks
            This additional buffer is only used where we have prepared this VertexData for
            use in shadow volume construction, and where the current render system supports
            vertex programs. This buffer contains the 'w' vertex position component which will
            be used by that program to differentiate between extruded and non-extruded vertices.
            This 'w' component cannot be included in the original position buffer because
            DirectX does not allow 4-component positions in the fixed-function pipeline, and the original
            position buffer must still be usable for fixed-function rendering.
        @par    
            Note that we don't store any vertex declaration or vertex buffer binding here because this
            can be reused in the shadow algorithm.
        */
        HardwareVertexBufferSharedPtr hardwareShadowVolWBuffer;


		/** Reorganises the data in the vertex buffers according to the 
			new vertex declaration passed in. Note that new vertex buffers
			are created and written to, so if the buffers being referenced 
			by this vertex data object are also used by others, then the 
			original buffers will not be damaged by this operation.
			Once this operation has completed, the new declaration 
			passed in will overwrite the current one.
		@param newDeclaration The vertex declaration which will be used
			for the reorganised buffer state. Note that the new declaration
			must not include any elements which do not already exist in the 
			current declaration; you can drop elements by 
			excluding them from the declaration if you wish, however.
		@param bufferUsages Vector of usage flags which indicate the usage options
			for each new vertex buffer created. The indexes of the entries must correspond
			to the buffer binding values referenced in the declaration.
		*/
		void reorganiseBuffers(VertexDeclaration* newDeclaration, const BufferUsageList& bufferUsage);

		/** Reorganises the data in the vertex buffers according to the 
			new vertex declaration passed in. Note that new vertex buffers
			are created and written to, so if the buffers being referenced 
			by this vertex data object are also used by others, then the 
			original buffers will not be damaged by this operation.
			Once this operation has completed, the new declaration 
			passed in will overwrite the current one.
            This version of the method derives the buffer usages from the existing
            buffers, by using the 'most flexible' usage from the equivalent sources.
		@param newDeclaration The vertex declaration which will be used
			for the reorganised buffer state. Note that the new delcaration
			must not include any elements which do not already exist in the 
			current declaration; you can drop elements by 
			excluding them from the declaration if you wish, however.
		*/
		void reorganiseBuffers(VertexDeclaration* newDeclaration);

        /** Remove any gaps in the vertex buffer bindings.
        @remarks
            This is useful if you've removed elements and buffers from this vertex
            data and want to remove any gaps in the vertex buffer bindings. This
            method is mainly useful when reorganising vertex data manually.
        @note
            This will cause binding index of the elements in the vertex declaration
            to be altered to new binding index.
        */
        void closeGapsInBindings(void);

        /** Remove all vertex buffers that never used by the vertex declaration.
        @remarks
            This is useful if you've removed elements from the vertex declaration
            and want to unreference buffers that never used any more. This method
            is mainly useful when reorganising vertex data manually.
        @note
            This also remove any gaps in the vertex buffer bindings.
        */
        void removeUnusedBuffers(void);

		/** Convert all packed colour values (VET_COLOUR_*) in buffers used to
			another type.
		@param srcType The source colour type to assume if the ambiguous VET_COLOUR
			is encountered.
		@param destType The destination colour type, must be VET_COLOUR_ABGR or
			VET_COLOUR_ARGB.
		*/
		void convertPackedColour(VertexElementType srcType, VertexElementType destType);


		/** Allocate elements to serve a holder of morph / pose target data 
			for hardware morphing / pose blending.
		@remarks
			This method will allocate the given number of 3D texture coordinate 
			sets for use as a morph target or target pose offset (3D position).
			These elements will be saved in hwAnimationDataList.
			It will also assume that the source of these new elements will be new
			buffers which are not bound at this time, so will start the sources to 
			1 higher than the current highest binding source. The caller is
			expected to bind these new buffers when appropriate. For morph animation
			the original position buffer will be the 'from' keyframe data, whilst
			for pose animation it will be the original vertex data.
		*/
		void allocateHardwareAnimationElements(ushort count);



	};

	/** Summary class collecting together index data source information. */
	class _OgreExport IndexData : public IndexDataAlloc
	{
    protected:
        /// Protected copy constructor, to prevent misuse
        IndexData(const IndexData& rhs); /* do nothing, should not use */
        /// Protected operator=, to prevent misuse
        IndexData& operator=(const IndexData& rhs); /* do not use */
    public:
        IndexData();
        ~IndexData();
		/// pointer to the HardwareIndexBuffer to use, must be specified if useIndexes = true
		HardwareIndexBufferSharedPtr indexBuffer;

		/// index in the buffer to start from for this operation
		size_t indexStart;

		/// The number of indexes to use from the buffer
		size_t indexCount;

		/** Clones this index data, potentially including replicating the index buffer.
		@remarks The caller is expected to delete the returned pointer when finished
		*/
		IndexData* clone(bool copyData = true) const;

		/** Re-order the indexes in this index data structure to be more
			vertex cache friendly; that is to re-use the same vertices as close
			together as possible. 
		@remarks
			Can only be used for index data which consists of triangle lists.
			It would in fact be pointless to use it on triangle strips or fans
			in any case.
		*/
		void optimiseVertexCacheTriList(void);
	
	};

	/** Vertex cache profiler.
	@remarks
		Utility class for evaluating the effectiveness of the use of the vertex
		cache by a given index buffer.
	*/
	class _OgreExport VertexCacheProfiler
    {
		public:
			enum CacheType {
				FIFO, LRU
			};

			VertexCacheProfiler(unsigned int cachesize = 16, CacheType cachetype = FIFO )
				: size ( cachesize ), type ( cachetype ), tail (0), buffersize (0), hit (0), miss (0)
			{
				cache = new uint32[size];
			};

			~VertexCacheProfiler()
			{
				delete[] cache;
			}

			void profile(const HardwareIndexBufferSharedPtr& indexBuffer);
			void reset() { hit = 0; miss = 0; tail = 0; buffersize = 0; };
			void flush() { tail = 0; buffersize = 0; };

			unsigned int getHits() { return hit; };
			unsigned int getMisses() { return miss; };
			unsigned int getSize() { return size; };
		private:
			unsigned int size;
			uint32 *cache;
			CacheType type;

			unsigned int tail, buffersize;
			unsigned int hit, miss;

			bool inCache(unsigned int index);
	};
}
#endif

