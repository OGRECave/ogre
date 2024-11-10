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
#ifndef __DataStream_H__
#define __DataStream_H__

#include "OgrePrerequisites.h"
#include <istream>
#include "OgreHeaderPrefix.h"

namespace Ogre {
    
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** General purpose class used for encapsulating the reading and writing of data.

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
    public:
        enum AccessMode
        {
            READ = 1, 
            WRITE = 2
        };
    protected:
        /// The name (e.g. resource name) that can be used to identify the source for this data (optional)
        String mName;       
        /// Size of the data in the stream (may be 0 if size cannot be determined)
        size_t mSize;
        /// What type of access is allowed (AccessMode)
        uint16 mAccess;

        #define OGRE_STREAM_TEMP_SIZE 128
    public:
        /// Constructor for creating unnamed streams
        DataStream(uint16 accessMode = READ) : mSize(0), mAccess(accessMode) {}
        /// Constructor for creating named streams
        DataStream(const String& name, uint16 accessMode = READ) 
            : mName(name), mSize(0), mAccess(accessMode) {}
        /// Returns the name of the stream, if it has one.
        const String& getName(void) { return mName; }
        /// Gets the access mode of the stream
        uint16 getAccessMode() const { return mAccess; }
        /** Reports whether this stream is readable. */
        virtual bool isReadable() const { return (mAccess & READ) != 0; }
        /** Reports whether this stream is writeable. */
        virtual bool isWriteable() const { return (mAccess & WRITE) != 0; }
        virtual ~DataStream() {}
        // Streaming operators
        template<typename T> DataStream& operator>>(T& val);
        /** Read the requisite number of bytes from the stream, 
            stopping at the end of the file.
        @param buf Reference to a buffer pointer
        @param count Number of bytes to read
        @return The number of bytes read
        */
        virtual size_t read(void* buf, size_t count) = 0;
        /** Write the requisite number of bytes from the stream (only applicable to 
            streams that are not read-only)
        @param buf Pointer to a buffer containing the bytes to write
        @param count Number of bytes to write
        @return The number of bytes written
        */
        virtual size_t write(const void* buf, size_t count)
        {
                        (void)buf;
                        (void)count;
            // default to not supported
            return 0;
        }

        /** Get a single line from the stream.

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
        @return The number of bytes read, excluding the terminating character
        */
        virtual size_t readLine(char* buf, size_t maxCount, const String& delim = "\n");
        
        /** Returns a String containing the next line of data, optionally 
            trimmed for whitespace. 

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

            This is a convenience method for text streams only, allowing you to 
            retrieve a String object containing all the data in the stream.
        */
        virtual String getAsString(void);

        /** Skip a single line from the stream.
        @note
            If you used this function, you <b>must</b> open the stream in <b>binary mode</b>,
            otherwise, it'll produce unexpected results.
        @par
            delim The delimiter(s) to stop at
        @return The number of bytes skipped
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

    /// List of DataStream items
    typedef std::list<DataStreamPtr> DataStreamList;

    /** Common subclass of DataStream for handling data from chunks of memory.
    */
    class _OgreExport MemoryDataStream : public DataStream
    {
    private:
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
            when the stream is closed. Note: it's important that if you set
            this option to true, that you allocated the memory using OGRE_ALLOC_T
            with a category of MEMCATEGORY_GENERAL to ensure the freeing of memory 
            matches up.
        @param readOnly Whether to make the stream on this memory read-only once created
        */
        MemoryDataStream(void* pMem, size_t size, bool freeOnClose = false, bool readOnly = false);
        
        /** Wrap an existing memory chunk in a named stream.
        @param name The name to give the stream
        @param pMem Pointer to the existing memory
        @param size The size of the memory chunk in bytes
        @param freeOnClose If true, the memory associated will be destroyed
            when the stream is destroyed. Note: it's important that if you set
            this option to true, that you allocated the memory using OGRE_ALLOC_T
            with a category of MEMCATEGORY_GENERAL ensure the freeing of memory 
            matches up.
        @param readOnly Whether to make the stream on this memory read-only once created
        */
        MemoryDataStream(const String& name, void* pMem, size_t size, 
                bool freeOnClose = false, bool readOnly = false);

        /** Create a stream which pre-buffers the contents of another stream.

            This constructor can be used to intentionally read in the entire
            contents of another stream, copying them to the internal buffer
            and thus making them available in memory as a single unit.
        @param sourceStream Another DataStream which will provide the source
            of data
        @param freeOnClose If true, the memory associated will be destroyed
            when the stream is destroyed.
        @param readOnly Whether to make the stream on this memory read-only once created
        */
        MemoryDataStream(DataStream& sourceStream, 
                bool freeOnClose = true, bool readOnly = false);
        
        /** Create a stream which pre-buffers the contents of another stream.

            This constructor can be used to intentionally read in the entire
            contents of another stream, copying them to the internal buffer
            and thus making them available in memory as a single unit.
        @param sourceStream Another DataStream which will provide the source
            of data
        @param freeOnClose If true, the memory associated will be destroyed
            when the stream is destroyed.
        @param readOnly Whether to make the stream on this memory read-only once created
        */
        MemoryDataStream(const DataStreamPtr& sourceStream,
                bool freeOnClose = true, bool readOnly = false);

        /** Create a named stream which pre-buffers the contents of 
            another stream.

            This constructor can be used to intentionally read in the entire
            contents of another stream, copying them to the internal buffer
            and thus making them available in memory as a single unit.
        @param name The name to give the stream
        @param sourceStream Another DataStream which will provide the source
            of data
        @param freeOnClose If true, the memory associated will be destroyed
            when the stream is destroyed.
        @param readOnly Whether to make the stream on this memory read-only once created
        */
        MemoryDataStream(const String& name, DataStream& sourceStream, 
                bool freeOnClose = true, bool readOnly = false);

        /** Create a named stream which pre-buffers the contents of 
        another stream.

        This constructor can be used to intentionally read in the entire
        contents of another stream, copying them to the internal buffer
        and thus making them available in memory as a single unit.
        @param name The name to give the stream
        @param sourceStream Another DataStream which will provide the source
        of data
        @param freeOnClose If true, the memory associated will be destroyed
        when the stream is destroyed.
        @param readOnly Whether to make the stream on this memory read-only once created
        */
        MemoryDataStream(const String& name, const DataStreamPtr& sourceStream, 
            bool freeOnClose = true, bool readOnly = false);

        /** Create a stream with a brand new empty memory chunk.
        @param size The size of the memory chunk to create in bytes
        @param freeOnClose If true, the memory associated will be destroyed
            when the stream is destroyed.
        @param readOnly Whether to make the stream on this memory read-only once created
        */
        MemoryDataStream(size_t size, bool freeOnClose = true, bool readOnly = false);
        /** Create a named stream with a brand new empty memory chunk.
        @param name The name to give the stream
        @param size The size of the memory chunk to create in bytes
        @param freeOnClose If true, the memory associated will be destroyed
            when the stream is destroyed.
        @param readOnly Whether to make the stream on this memory read-only once created
        */
        MemoryDataStream(const String& name, size_t size, 
                bool freeOnClose = true, bool readOnly = false);

        ~MemoryDataStream();

        /** Get a pointer to the start of the memory block this stream holds. */
        uchar* getPtr(void) { return mData; }
        
        /** Get a pointer to the current position in the memory block this stream holds. */
        uchar* getCurrentPtr(void) { return mPos; }
        
        /** @copydoc DataStream::read
        */
        size_t read(void* buf, size_t count) override;

        /** @copydoc DataStream::write
        */
        size_t write(const void* buf, size_t count) override;

        /** @copydoc DataStream::readLine
        */
        size_t readLine(char* buf, size_t maxCount, const String& delim = "\n") override;
        
        /** @copydoc DataStream::skipLine
        */
        size_t skipLine(const String& delim = "\n") override;

        /** @copydoc DataStream::skip
        */
        void skip(long count) override;
    
        /** @copydoc DataStream::seek
        */
        void seek( size_t pos ) override;
        
        /** @copydoc DataStream::tell
        */
        size_t tell(void) const override;

        /** @copydoc DataStream::eof
        */
        bool eof(void) const override;

        /** @copydoc DataStream::close
        */
        void close(void) override;

        /** Sets whether or not to free the encapsulated memory on close. */
        void setFreeOnClose(bool free) { mFreeOnClose = free; }
    };

    /** Common subclass of DataStream for handling data from 
        std::basic_istream.
    */
    class _OgreExport FileStreamDataStream : public DataStream
    {
    private:
        /// Reference to source stream (read)
        std::istream* mInStream;
        /// Reference to source file stream (read-only)
        std::ifstream* mFStreamRO;
        /// Reference to source file stream (read-write)
        std::fstream* mFStream;
        bool mFreeOnClose;  

        void determineAccess();
    public:
        /** Construct a read-only stream from an STL stream
        @param s Pointer to source stream
        @param freeOnClose Whether to delete the underlying stream on 
            destruction of this class
        */
        FileStreamDataStream(std::ifstream* s, 
            bool freeOnClose = true);
        /** Construct a read-write stream from an STL stream
        @param s Pointer to source stream
        @param freeOnClose Whether to delete the underlying stream on 
        destruction of this class
        */
        FileStreamDataStream(std::fstream* s, 
            bool freeOnClose = true);

        /** Construct named read-only stream from an STL stream
        @param name The name to give this stream
        @param s Pointer to source stream
        @param freeOnClose Whether to delete the underlying stream on 
            destruction of this class
        */
        FileStreamDataStream(const String& name, 
            std::ifstream* s, 
            bool freeOnClose = true);

        /** Construct named read-write stream from an STL stream
        @param name The name to give this stream
        @param s Pointer to source stream
        @param freeOnClose Whether to delete the underlying stream on 
        destruction of this class
        */
        FileStreamDataStream(const String& name, 
            std::fstream* s, 
            bool freeOnClose = true);

        /** Construct named read-only stream from an STL stream, and tell it the size

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

        /** Construct named read-write stream from an STL stream, and tell it the size

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
            std::fstream* s, 
            size_t size, 
            bool freeOnClose = true);

        ~FileStreamDataStream();

        /** @copydoc DataStream::read
        */
        size_t read(void* buf, size_t count) override;

        /** @copydoc DataStream::write
        */
        size_t write(const void* buf, size_t count) override;

        /** @copydoc DataStream::readLine
        */
        size_t readLine(char* buf, size_t maxCount, const String& delim = "\n") override;
        
        /** @copydoc DataStream::skip
        */
        void skip(long count) override;
    
        /** @copydoc DataStream::seek
        */
        void seek( size_t pos ) override;

        /** @copydoc DataStream::tell
        */
        size_t tell(void) const override;

        /** @copydoc DataStream::eof
        */
        bool eof(void) const override;

        /** @copydoc DataStream::close
        */
        void close(void) override;
        
        
    };

    /** Common subclass of DataStream for handling data from C-style file 
        handles.

        Use of this class is generally discouraged; if you want to wrap file
        access in a DataStream, you should definitely be using the C++ friendly
        FileStreamDataStream. However, since there are quite a few applications
        and libraries still wedded to the old FILE handle access, this stream
        wrapper provides some backwards compatibility.
    */
    class _OgreExport FileHandleDataStream : public DataStream
    {
    private:
        FILE* mFileHandle;
    public:
        /// Create stream from a C file handle
        FileHandleDataStream(FILE* handle, uint16 accessMode = READ);
        /// Create named stream from a C file handle
        FileHandleDataStream(const String& name, FILE* handle, uint16 accessMode = READ);
        ~FileHandleDataStream();

        /** @copydoc DataStream::read
        */
        size_t read(void* buf, size_t count) override;

        /** @copydoc DataStream::write
        */
        size_t write(const void* buf, size_t count) override;

        /** @copydoc DataStream::skip
        */
        void skip(long count) override;
    
        /** @copydoc DataStream::seek
        */
        void seek( size_t pos ) override;

        /** @copydoc DataStream::tell
        */
        size_t tell(void) const override;

        /** @copydoc DataStream::eof
        */
        bool eof(void) const override;

        /** @copydoc DataStream::close
        */
        void close(void) override;

    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif

