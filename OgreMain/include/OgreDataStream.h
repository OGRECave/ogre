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
#ifndef __DataStream_H__
#define __DataStream_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreSharedPtr.h"
#include <istream>

namespace Ogre {

	/** General purpose class used for encapsulating the reading of data.
	@remarks
		This class performs basically the same tasks as std::basic_istream, 
		except that it does not have any formatting capabilities, and is
		designed to be subclassed to receive data from multiple sources,
		including libraries which have no compatibility with the STL's
		stream interfaces. As such, this is an abstraction of a set of 
		wrapper classes which pretend to be standard stream classes but 
		can actually be implemented quite differently. 
	@par
		Generally, if a plugin or application provides an ArchiveFactory, 
		it should also provide a DataStream subclass which will be used
		to stream data out of that Archive implementation, unless it can 
		use one of the common implementations included.
	@note
		Ogre makes no guarantees about thread safety, for performance reasons.
		If you wish to access stream data asynchronously then you should
		organise your own mutexes to avoid race conditions. 
	*/
	class _OgreExport DataStream : public StreamAlloc
	{
	protected:
		/// The name (e.g. resource name) that can be used to identify the source fot his data (optional)
		String mName;		
        /// Size of the data in the stream (may be 0 if size cannot be determined)
        size_t mSize;
        #define OGRE_STREAM_TEMP_SIZE 128
	public:
		/// Constructor for creating unnamed streams
        DataStream() : mSize(0) {}
		/// Constructor for creating named streams
		DataStream(const String& name) : mName(name), mSize(0) {}
		/// Returns the name of the stream, if it has one.
		const String& getName(void) { return mName; }
        virtual ~DataStream() {}
		// Streaming operators
        template<typename T> DataStream& operator>>(T& val);
		/** Read the requisite number of bytes from the stream, 
			stopping at the end of the file.
		@param buf Reference to a buffer pointer
		@param count Number of bytes to read
		@returns The number of bytes read
		*/
		virtual size_t read(void* buf, size_t count) = 0;
		/** Get a single line from the stream.
		@remarks
			The delimiter character is not included in the data
			returned, and it is skipped over so the next read will occur
			after it. The buffer contents will include a
			terminating character.
        @note
            If you used this function, you <b>must</b> open the stream in <b>binary mode</b>,
            otherwise, it'll produce unexpected results.
		@param buf Reference to a buffer pointer
		@param maxCount The maximum length of data to be read, excluding the terminating character
		@param delim The delimiter to stop at
		@returns The number of bytes read, excluding the terminating character
		*/
		virtual size_t readLine(char* buf, size_t maxCount, const String& delim = "\n");
		
	    /** Returns a String containing the next line of data, optionally 
		    trimmed for whitespace. 
	    @remarks
		    This is a convenience method for text streams only, allowing you to 
		    retrieve a String object containing the next line of data. The data
		    is read up to the next newline character and the result trimmed if
		    required.
        @note
            If you used this function, you <b>must</b> open the stream in <b>binary mode</b>,
            otherwise, it'll produce unexpected results.
	    @param 
		    trimAfter If true, the line is trimmed for whitespace (as in 
		    String.trim(true,true))
	    */
	    virtual String getLine( bool trimAfter = true );

	    /** Returns a String containing the entire stream. 
	    @remarks
		    This is a convenience method for text streams only, allowing you to 
		    retrieve a String object containing all the data in the stream.
	    */
	    virtual String getAsString(void);

		/** Skip a single line from the stream.
        @note
            If you used this function, you <b>must</b> open the stream in <b>binary mode</b>,
            otherwise, it'll produce unexpected results.
		@param delim The delimiter(s) to stop at
		@returns The number of bytes skipped
		*/
		virtual size_t skipLine(const String& delim = "\n");

		/** Skip a defined number of bytes. This can also be a negative value, in which case
		the file pointer rewinds a defined number of bytes. */
		virtual void skip(long count) = 0;
	
		/** Repositions the read point to a specified byte.
	    */
	    virtual void seek( size_t pos ) = 0;
		
		/** Returns the current byte offset from beginning */
	    virtual size_t tell(void) const = 0;

		/** Returns true if the stream has reached the end.
	    */
	    virtual bool eof(void) const = 0;

		/** Returns the total size of the data to be read from the stream, 
			or 0 if this is indeterminate for this stream. 
		*/
        size_t size(void) const { return mSize; }

        /** Close the stream; this makes further operations invalid. */
        virtual void close(void) = 0;
		

	};

	/** Shared pointer to allow data streams to be passed around without
		worrying about deallocation
	*/
	typedef SharedPtr<DataStream> DataStreamPtr;

	/// List of DataStream items
	typedef list<DataStreamPtr>::type DataStreamList;
	/// Shared pointer to list of DataStream items
	typedef SharedPtr<DataStreamList> DataStreamListPtr;

	/** Common subclass of DataStream for handling data from chunks of memory.
	*/
	class _OgreExport MemoryDataStream : public DataStream
	{
	protected:
        /// Pointer to the start of the data area
	    uchar* mData;
        /// Pointer to the current position in the memory
	    uchar* mPos;
        /// Pointer to the end of the memory
	    uchar* mEnd;
        /// Do we delete the memory on close
		bool mFreeOnClose;			
	public:
		
		/** Wrap an existing memory chunk in a stream.
		@param pMem Pointer to the existing memory
		@param size The size of the memory chunk in bytes
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed. Note: it's important that if you set
			this option to true, that you allocated the memory using OGRE_ALLOC_T
			with a category of MEMCATEGORY_GENERAL ensure the freeing of memory 
			matches up.
		*/
		MemoryDataStream(void* pMem, size_t size, bool freeOnClose = false);
		
		/** Wrap an existing memory chunk in a named stream.
		@param name The name to give the stream
		@param pMem Pointer to the existing memory
		@param size The size of the memory chunk in bytes
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed. Note: it's important that if you set
			this option to true, that you allocated the memory using OGRE_ALLOC_T
			with a category of MEMCATEGORY_GENERAL ensure the freeing of memory 
			matches up.
		*/
		MemoryDataStream(const String& name, void* pMem, size_t size, 
				bool freeOnClose = false);

		/** Create a stream which pre-buffers the contents of another stream.
		@remarks
			This constructor can be used to intentionally read in the entire
			contents of another stream, copying them to the internal buffer
			and thus making them available in memory as a single unit.
		@param sourceStream Another DataStream which will provide the source
			of data
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(DataStream& sourceStream, 
				bool freeOnClose = true);
		
		/** Create a stream which pre-buffers the contents of another stream.
		@remarks
			This constructor can be used to intentionally read in the entire
			contents of another stream, copying them to the internal buffer
			and thus making them available in memory as a single unit.
		@param sourceStream Weak reference to another DataStream which will provide the source
			of data
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(DataStreamPtr& sourceStream, 
				bool freeOnClose = true);

		/** Create a named stream which pre-buffers the contents of 
			another stream.
		@remarks
			This constructor can be used to intentionally read in the entire
			contents of another stream, copying them to the internal buffer
			and thus making them available in memory as a single unit.
		@param name The name to give the stream
		@param sourceStream Another DataStream which will provide the source
			of data
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(const String& name, DataStream& sourceStream, 
				bool freeOnClose = true);

        /** Create a named stream which pre-buffers the contents of 
        another stream.
        @remarks
        This constructor can be used to intentionally read in the entire
        contents of another stream, copying them to the internal buffer
        and thus making them available in memory as a single unit.
        @param name The name to give the stream
        @param sourceStream Another DataStream which will provide the source
        of data
        @param freeOnClose If true, the memory associated will be destroyed
        when the stream is destroyed.
        */
        MemoryDataStream(const String& name, const DataStreamPtr& sourceStream, 
            bool freeOnClose = true);

        /** Create a stream with a brand new empty memory chunk.
		@param size The size of the memory chunk to create in bytes
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(size_t size, bool freeOnClose = true);
		/** Create a named stream with a brand new empty memory chunk.
		@param name The name to give the stream
		@param size The size of the memory chunk to create in bytes
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(const String& name, size_t size, 
				bool freeOnClose = true);

		~MemoryDataStream();

		/** Get a pointer to the start of the memory block this stream holds. */
		uchar* getPtr(void) { return mData; }
		
		/** Get a pointer to the current position in the memory block this stream holds. */
		uchar* getCurrentPtr(void) { return mPos; }
		
		/** @copydoc DataStream::read
		*/
		size_t read(void* buf, size_t count);
		/** @copydoc DataStream::readLine
		*/
		size_t readLine(char* buf, size_t maxCount, const String& delim = "\n");
		
		/** @copydoc DataStream::skipLine
		*/
		size_t skipLine(const String& delim = "\n");

		/** @copydoc DataStream::skip
		*/
		void skip(long count);
	
		/** @copydoc DataStream::seek
		*/
	    void seek( size_t pos );
		
		/** @copydoc DataStream::tell
		*/
	    size_t tell(void) const;

		/** @copydoc DataStream::eof
		*/
	    bool eof(void) const;

        /** @copydoc DataStream::close
        */
        void close(void);

		/** Sets whether or not to free the encapsulated memory on close. */
		void setFreeOnClose(bool free) { mFreeOnClose = free; }
	};

    /** Shared pointer to allow memory data streams to be passed around without
    worrying about deallocation
    */
    typedef SharedPtr<MemoryDataStream> MemoryDataStreamPtr;

    /** Common subclass of DataStream for handling data from 
		std::basic_istream.
	*/
	class _OgreExport FileStreamDataStream : public DataStream
	{
	protected:
		/// Reference to source stream
		std::ifstream* mpStream;
        bool mFreeOnClose;			
	public:
		/** Construct stream from an STL stream
        @param s Pointer to source stream
        @param freeOnClose Whether to delete the underlying stream on 
            destruction of this class
        */
		FileStreamDataStream(std::ifstream* s, 
            bool freeOnClose = true);
		/** Construct named stream from an STL stream
        @param name The name to give this stream
        @param s Pointer to source stream
        @param freeOnClose Whether to delete the underlying stream on 
            destruction of this class
        */
		FileStreamDataStream(const String& name, 
            std::ifstream* s, 
            bool freeOnClose = true);

		/** Construct named stream from an STL stream, and tell it the size
        @remarks
            This variant tells the class the size of the stream too, which 
            means this class does not need to seek to the end of the stream 
            to determine the size up-front. This can be beneficial if you have
            metadata about the contents of the stream already.
        @param name The name to give this stream
        @param s Pointer to source stream
        @param size Size of the stream contents in bytes
        @param freeOnClose Whether to delete the underlying stream on 
            destruction of this class. If you specify 'true' for this you
			must ensure that the stream was allocated using OGRE_NEW_T with 
			MEMCATEGRORY_GENERAL.
        */
		FileStreamDataStream(const String& name, 
            std::ifstream* s, 
            size_t size, 
            bool freeOnClose = true);

        ~FileStreamDataStream();

		/** @copydoc DataStream::read
		*/
		size_t read(void* buf, size_t count);
		/** @copydoc DataStream::readLine
		*/
        size_t readLine(char* buf, size_t maxCount, const String& delim = "\n");
		
		/** @copydoc DataStream::skip
		*/
		void skip(long count);
	
		/** @copydoc DataStream::seek
		*/
	    void seek( size_t pos );

		/** @copydoc DataStream::tell
		*/
		size_t tell(void) const;

		/** @copydoc DataStream::eof
		*/
	    bool eof(void) const;

        /** @copydoc DataStream::close
        */
        void close(void);
		
		
	};

	/** Common subclass of DataStream for handling data from C-style file 
		handles.
    @remarks
        Use of this class is generally discouraged; if you want to wrap file
        access in a DataStream, you should definitely be using the C++ friendly
        FileStreamDataStream. However, since there are quite a few applications
        and libraries still wedded to the old FILE handle access, this stream
        wrapper provides some backwards compatibility.
	*/
	class _OgreExport FileHandleDataStream : public DataStream
	{
	protected:
		FILE* mFileHandle;
	public:
		/// Create stream from a C file handle
		FileHandleDataStream(FILE* handle);
		/// Create named stream from a C file handle
		FileHandleDataStream(const String& name, FILE* handle);
        ~FileHandleDataStream();

		/** @copydoc DataStream::read
		*/
		size_t read(void* buf, size_t count);

		/** @copydoc DataStream::skip
		*/
		void skip(long count);
	
		/** @copydoc DataStream::seek
		*/
	    void seek( size_t pos );

		/** @copydoc DataStream::tell
		*/
		size_t tell(void) const;

		/** @copydoc DataStream::eof
		*/
	    bool eof(void) const;

        /** @copydoc DataStream::close
        */
        void close(void);

	};
}
#endif

