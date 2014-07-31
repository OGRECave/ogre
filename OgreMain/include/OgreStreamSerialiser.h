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
#ifndef __StreamSerialiser_H__
#define __StreamSerialiser_H__

#include "OgrePrerequisites.h"
#include "OgreDataStream.h"
#include "OgreHeaderPrefix.h"

namespace Ogre 
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/

	/** Utility class providing helper methods for reading / writing 
		structured data held in a DataStream.
	@remarks
		The structure of a file read / written by this class is a series of 
		'chunks'. A chunk-based format has the advantage of being extensible later, 
		and it's robust, in that a reader can skip chunks that they are not 
		able (or willing) to process.
	@par
		Chunks are contained serially in the file, but they can also be 
		nested in order both to provide context, and to group chunks together for 
		potential skipping. 
	@par
		The data format of a chunk is as follows:
		-# Chunk ID (32-bit uint). This can be any number unique in a context, except the numbers 0x0000, 0x0001 and 0x1000, which are reserved for Ogre's use
		-# Chunk version (16-bit uint). Chunks can change over time so this version number reflects that
		-# Length (32-bit uint). The length of the chunk data section, including nested chunks. Note that
			this length excludes this header, but includes the header of any nested chunks. 
		-# Checksum (32-bit uint). Checksum value generated from the above - basically lets us check this is a valid chunk.
		-# Chunk data
		The 'Chunk data' section will contain chunk-specific data, which may include
		other nested chunks.
	*/
	class _OgreExport StreamSerialiser : public StreamAlloc
	{
	public:
		/// The endianness of files
		enum Endian
		{
			/// Automatically determine endianness
			ENDIAN_AUTO,
			/// Use big endian (0x1000 is serialised as 0x10 0x00)
			ENDIAN_BIG,
			/// Use little endian (0x1000 is serialised as 0x00 0x10)
			ENDIAN_LITTLE
		};

		/// The storage format of Real values
		enum RealStorageFormat
		{
			/// Real is stored as float, reducing precision if you're using OGRE_DOUBLE_PRECISION
			REAL_FLOAT,
			/// Real as stored as double, not useful unless you're using OGRE_DOUBLE_PRECISION
			REAL_DOUBLE
		};


		/// Definition of a chunk of data in a file
		struct Chunk : public StreamAlloc
		{
			/// Identifier of the chunk (for example from makeIdentifier)  (stored)
			uint32 id;
			/// Version of the chunk (stored)
			uint16 version;
			/// Length of the chunk data in bytes, excluding the header of this chunk (stored)
			uint32 length;
			/// Location of the chunk (header) in bytes from the start of a stream (derived)
			uint32 offset;

			Chunk() : id(0), version(1), length(0), offset(0) {}
		};

		/** Constructor.
		@param stream The stream on which you will read / write data.
		@param endianMode The endian mode in which to read / writedata. If left at
			the default, when writing the endian mode will be the native platform mode, 
			and when reading it's expected that the first chunk encountered will be 
			the header chunk, which will determine the endian mode.
		@param autoHeader If true, the first write or read to this stream will 
			automatically read / write the header too. This is required if you
			set endianMode to ENDIAN_AUTO, but if you manually set the endian mode, 
			then you can skip writing / reading the header if you wish, if for example
			this stream is midway through a file which has already included header
			information.
		@param realFormat Set the format you want to write reals in. Only useful for files that 
			you're writing (since when reading this is picked up from the file), 
			and can only be changed if autoHeader is true, since real format is stored in the header. 
			Defaults to float unless you're using OGRE_DOUBLE_PRECISION.
		*/
		StreamSerialiser(const DataStreamPtr& stream, Endian endianMode = ENDIAN_AUTO, 
			bool autoHeader = true, 
#if OGRE_DOUBLE_PRECISION
			RealStorageFormat realFormat = REAL_DOUBLE
#else
			RealStorageFormat realFormat = REAL_FLOAT
#endif
			);
		virtual ~StreamSerialiser();

		/** Get the endian mode.
		@remarks
			If the result is ENDIAN_AUTO, this mode will change when the first piece of
			data is read / written. 
		*/
		virtual Endian getEndian() const { return mEndian; }

		/** Pack a 4-character code into a 32-bit identifier.
		@remarks
			You can use this to generate id's for your chunks based on friendlier
			4-character codes rather than assigning numerical IDs, if you like.
		@param code String to pack - must be 4 characters.
		*/
		static uint32 makeIdentifier(const String& code);

		/** Report the current depth of the chunk nesting, whether reading or writing. 
		@remarks
			Returns how many levels of nested chunks are currently being processed, 
			either writing or reading. In order to tidily finish, you must call
			read/writeChunkEnd this many times.
		*/
		size_t getCurrentChunkDepth() const { return mChunkStack.size(); }

		/** Get the ID of the chunk that's currently being read/written, if any.
		@return The id of the current chunk being read / written (at the tightest
			level of nesting), or zero if no chunk is being processed.
		*/
		uint32 getCurrentChunkID() const;

		/** Get the current byte position relative to the start of the data section
			of the last chunk that was read or written. 
		@return the offset. Note that a return value of 0 means that either the
			position is at the start of the chunk data section (ie right after the
			header), or that no chunk is currently active. Use getCurrentChunkID
			or getCurrentChunkDepth to determine if a chunk is active.
		*/
		size_t getOffsetFromChunkStart() const;

		/** Reads the start of the next chunk in the file.
		@remarks
			Files are serialised in a chunk-based manner, meaning that each section
			of data is prepended by a chunk header. After reading this chunk header, 
			the next set of data is available directly afterwards. 
		@note
			When you have finished with this chunk, you should call readChunkEnd. 
			This will perform a bit of validation and clear the chunk from 
			the stack. 
		@return The Chunk that comes next
		*/
		virtual const Chunk* readChunkBegin();

		/** Reads the start of the next chunk so long as it's of a given ID and version.
		@remarks
			This method operates like readChunkBegin, except it checks the ID and
			version.
		@param id The ID you're expecting. If the next chunk isn't of this ID, then
			the chunk read is undone and the method returns null.
		@param maxVersion The maximum version you're able to process. If the ID is correct
			but the version	exceeds what is passed in here, the chunk is skipped over,
			the problem logged and null is returned. 
		@param msg Descriptive text added to the log if versions are not compatible
		@return The chunk if it passes the validation.
		*/
		virtual const Chunk* readChunkBegin(uint32 id, uint16 maxVersion, const String& msg = StringUtil::BLANK);

		/** Call this to 'rewind' the stream to just before the start of the current
			chunk. 
		@remarks
			The most common case of wanting to use this is if you'd calledReadChunkBegin(), 
			but the chunk you read wasn't one you wanted to process, and rather than
			skipping over it (which readChunkEnd() would do), you want to backtrack
			and give something else an opportunity to read it. 
		@param id The id of the chunk that you were reading (for validation purposes)
		*/
		virtual void undoReadChunk(uint32 id);

		/** Call this to 'peek' at the next chunk ID without permanently moving the stream pointer. */
		virtual uint32 peekNextChunkID(); 

		/** Finish the reading of a chunk.
		@remarks
			You can call this method at any point after calling readChunkBegin, even
			if you didn't read all the rest of the data in the chunk. If you did 
			not read to the end of a chunk, this method will automatically skip 
			over the remainder of the chunk and position the stream just after it. 
		@param id The id of the chunk that you were reading (for validation purposes)
		*/
		virtual void readChunkEnd(uint32 id);

		/** Return whether the current data pointer is at the end of the current chunk.
		@param id The id of the chunk that you were reading (for validation purposes)
		*/
		virtual bool isEndOfChunk(uint32 id);

		/// Reports whether the stream is at the end of file
		virtual bool eof() const;

		/** Get the definition of the current chunk being read (if any). */
		virtual const Chunk* getCurrentChunk() const;

		/** Begin writing a new chunk.
		@remarks
			This starts the process of writing a new chunk to the stream. This will 
			write the chunk header for you, and store a pointer so that the
			class can automatically go back and fill in the size for you later
			should you need it to. If you have already begun a chunk without ending
			it, then this method will start a nested chunk within it. Once written, 
			you can then start writing chunk-specific data into your stream.
		@note If this is the first chunk in the file
		@param id The identifier of the new chunk. Any value that's unique in the
			file context is valid, except for the numbers 0x0001 and 0x1000 which are reserved
			for internal header identification use. 
		@param version The version of the chunk you're writing
		*/
		virtual void writeChunkBegin(uint32 id, uint16 version = 1);
		/** End writing a chunk. 
		@param id The identifier of the chunk - this is really just a safety check, 
			since you can only end the chunk you most recently started.
		*/
		virtual void writeChunkEnd(uint32 id);

		/** Write arbitrary data to a stream. 
		@param buf Pointer to bytes
		@param size The size of each element to write; each will be endian-flipped if
			necessary
		@param count The number of elements to write
		*/
		virtual void writeData(const void* buf, size_t size, size_t count);

		/** Catch-all method to write primitive types. */
		template <typename T>
		void write(const T* pT, size_t count = 1)
		{
			writeData(pT, sizeof(T), count);
		}

		// Special-case Real since we need to deal with single/double precision
		virtual void write(const Real* val, size_t count = 1);

		virtual void write(const Vector2* vec, size_t count = 1);
		virtual void write(const Vector3* vec, size_t count = 1);
		virtual void write(const Vector4* vec, size_t count = 1);
		virtual void write(const Quaternion* q, size_t count = 1);
		virtual void write(const Matrix3* m, size_t count = 1);
		virtual void write(const Matrix4* m, size_t count = 1);
		virtual void write(const String* string);
		virtual void write(const AxisAlignedBox* aabb, size_t count = 1);
		virtual void write(const Sphere* sphere, size_t count = 1);
		virtual void write(const Plane* plane, size_t count = 1);
		virtual void write(const Ray* ray, size_t count = 1);
		virtual void write(const Radian* angle, size_t count = 1);
		virtual void write(const Node* node, size_t count = 1);
		virtual void write(const bool* boolean, size_t count = 1);


		/** Read arbitrary data from a stream. 
		@param buf Pointer to bytes
		@param size The size of each element to read; each will be endian-flipped if
		necessary
		@param count The number of elements to read
		*/
		virtual void readData(void* buf, size_t size, size_t count);

		/** Catch-all method to read primitive types. */
		template <typename T>
		void read(T* pT, size_t count = 1)
		{
			readData(pT, sizeof(T), count);
		}

		// Special case Real, single/double-precision issues
		virtual void read(Real* val, size_t count = 1);

		/// read a Vector3
		virtual void read(Vector2* vec, size_t count = 1);
		virtual void read(Vector3* vec, size_t count = 1);
		virtual void read(Vector4* vec, size_t count = 1);
		virtual void read(Quaternion* q, size_t count = 1);
		virtual void read(Matrix3* m, size_t count = 1);
		virtual void read(Matrix4* m, size_t count = 1);
		virtual void read(String* string);
		virtual void read(AxisAlignedBox* aabb, size_t count = 1);
		virtual void read(Sphere* sphere, size_t count = 1);
		virtual void read(Plane* plane, size_t count = 1);
		virtual void read(Ray* ray, size_t count = 1);
		virtual void read(Radian* angle, size_t count = 1);
		virtual void read(Node* node, size_t count = 1);
		virtual void read(bool* val, size_t count = 1);

		/** Start (un)compressing data
		@param avail_in Available bytes for uncompressing
		*/
		virtual void startDeflate(size_t avail_in = 0);
		/** Stop (un)compressing data
		*/
		virtual void stopDeflate();
	protected:
		DataStreamPtr mStream;
		DataStreamPtr mOriginalStream;
		Endian mEndian;
		bool mFlipEndian;
		bool mReadWriteHeader;
		RealStorageFormat mRealFormat;
		typedef deque<Chunk*>::type ChunkStack;
		/// Current list of open chunks
		ChunkStack mChunkStack;

		static uint32 HEADER_ID;
		static uint32 REVERSE_HEADER_ID;
		static uint32 CHUNK_HEADER_SIZE;

		virtual Chunk* readChunkImpl();
		virtual void writeChunkImpl(uint32 id, uint16 version);
		virtual void readHeader();
		virtual void writeHeader();
		virtual uint32 calculateChecksum(Chunk* c);
		virtual void checkStream(bool failOnEof = false, 
			bool validateReadable = false, bool validateWriteable = false) const;

		virtual void determineEndianness();
		virtual Chunk* popChunk(uint id);

		virtual void writeFloatsAsDoubles(const float* val, size_t count);
		virtual void writeDoublesAsFloats(const double* val, size_t count);
		virtual void readFloatsAsDoubles(double* val, size_t count);
		virtual void readDoublesAsFloats(float* val, size_t count);
		template <typename T, typename U>
		void writeConverted(const T* src, U typeToWrite, size_t count)
		{
			U* tmp = OGRE_ALLOC_T(U, count, MEMCATEGORY_GENERAL);
			U* pDst = tmp;
			const T* pSrc = src;
			for (size_t i = 0; i < count; ++i)
				*pDst++ = static_cast<U>(*pSrc++);
			
			writeData(tmp, sizeof(U), count);

			OGRE_FREE(tmp, MEMCATEGORY_GENERAL);
		}
		template <typename T, typename U>
		void readConverted(T* dst, U typeToRead, size_t count)
		{
			U* tmp = OGRE_ALLOC_T(U, count, MEMCATEGORY_GENERAL);
			readData(tmp, sizeof(U), count);

			T* pDst = dst;
			const U* pSrc = tmp;
			for (size_t i = 0; i < count; ++i)
				*pDst++ = static_cast<T>(*pSrc++);


			OGRE_FREE(tmp, MEMCATEGORY_GENERAL);
		}

	};
	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif

