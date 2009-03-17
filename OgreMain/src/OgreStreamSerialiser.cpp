/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreStableHeaders.h"

#include "OgreCommon.h"
#include "OgreStreamSerialiser.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreQuaternion.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	uint32 StreamSerialiser::HEADER_ID = 0x0001;
	uint32 StreamSerialiser::REVERSE_HEADER_ID = 0x1000;
	uint32 StreamSerialiser::CHUNK_HEADER_SIZE = 
		sizeof(uint32) + // id
		sizeof(uint16) + // version
		sizeof(uint32) + // length
		sizeof(uint32); // checksum
	//---------------------------------------------------------------------
	StreamSerialiser::StreamSerialiser(const DataStreamPtr& stream, Endian endianMode, 
		bool autoHeader)
		: mStream(stream)
		, mEndian(endianMode)
		, mFlipEndian(false)
		, mReadWriteHeader(autoHeader)
	{
		if (mEndian != ENDIAN_AUTO)
		{
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
			if (mEndian == ENDIAN_LITTLE)
				mFlipEndian = true;
#else
			if (mEndian == ENDIAN_BIG)
				mFlipEndian = true;
#endif

		}

		checkStream();

	}
	//---------------------------------------------------------------------
	StreamSerialiser::~StreamSerialiser()
	{
		// really this should be empty if read/write was complete, but be tidy
		if (!mChunkStack.empty())
		{
			LogManager::getSingleton().stream() <<
				"Warning: stream " << mStream->getName() << " was not fully read / written; " <<
				mChunkStack.size() << " chunks remain unterminated.";
		}
		for (ChunkStack::iterator i = mChunkStack.begin(); i != mChunkStack.end(); ++i)
			delete *i;
		mChunkStack.clear();

	}
	//---------------------------------------------------------------------
	uint32 StreamSerialiser::makeIdentifier(const String& code)
	{
		assert(code.length() <= 4 && "Characters after the 4th are being ignored");
		uint32 ret = 0;
		size_t c = std::min((size_t)4, code.length());
		for (size_t i = 0; i < c; ++i)
		{
			ret += (code.at(i) << (i * 8));
		}
		return ret;

	}
	//---------------------------------------------------------------------
	uint32 StreamSerialiser::getCurrentChunkID() const
	{
		if (mChunkStack.empty())
			return 0;
		else
			return mChunkStack.back()->id;
	}
	//---------------------------------------------------------------------
	const StreamSerialiser::Chunk* StreamSerialiser::readChunkBegin()
	{
		// Have we figured out the endian mode yet?
		if (mReadWriteHeader)
			readHeader();

		if (mEndian == ENDIAN_AUTO)
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
			"Endian mode has not been determined, did you disable header without setting?", 
			"StreamSerialiser::readChunkBegin");
		
		Chunk* chunk = readChunkImpl();
		mChunkStack.push_back(chunk);

		return chunk;

	}
	//---------------------------------------------------------------------
	void StreamSerialiser::readChunkEnd(uint32 id)
	{
		Chunk* c = popChunk(id);

		checkStream();

		// skip to the end of the chunk if we were not there already
		// this lets us quite reading a chunk anywhere and have the read marker
		// automatically skip to the next one
		if (mStream->tell() < (c->offset + c->length))
			mStream->seek(c->offset + c->length);

		OGRE_DELETE c;
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::readHeader()
	{
		uint32 headerid;
		size_t actually_read = mStream->read(&headerid, sizeof(uint32));
		// skip back
		mStream->skip(0 - (long)actually_read);
		// validate that this is a header chunk
		if (headerid == REVERSE_HEADER_ID)
		{
			mFlipEndian = true;
		}
		else if (headerid == HEADER_ID)
		{
			mFlipEndian = false;
		}
		else
		{
			// no good
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
				"Cannot determine endian mode because header is missing", 
				"StreamSerialiser::readHeader");
		}
		determineEndianness();

		mReadWriteHeader = false;

		const Chunk* c = readChunkBegin();
		// endian should be flipped now
		assert(c->id == HEADER_ID);
		readChunkEnd(HEADER_ID);

	}
	//---------------------------------------------------------------------
	void StreamSerialiser::determineEndianness()
	{
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		if (mFlipEndian)
			mEndian = ENDIAN_LITTLE;
		else
			mEndian = ENDIAN_BIG;
#else
		if (mFlipEndian)
			mEndian = ENDIAN_BIG;
		else
			mEndian = ENDIAN_LITTLE;
#endif
	}
	//---------------------------------------------------------------------
	const StreamSerialiser::Chunk* StreamSerialiser::getCurrentChunk() const
	{
		if (mChunkStack.empty())
			return 0;
		else
			return mChunkStack.back();
	}
	//---------------------------------------------------------------------
	bool StreamSerialiser::eof() const
	{ 
		checkStream();
		return mStream->eof(); 
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::checkStream(bool failOnEof, bool validateReadable, bool validateWriteable) const
	{
		if (mStream.isNull())
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
			"Invalid operation, stream is null", "StreamSerialiser::checkStream");

		if (failOnEof && mStream->eof())
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
			"Invalid operation, end of file on stream", "StreamSerialiser::checkStream");

		if (validateReadable && !mStream->isReadable())
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
			"Invalid operation, file is not readable", "StreamSerialiser::checkStream");

		if (validateWriteable && !mStream->isWriteable())
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
			"Invalid operation, file is not writeable", "StreamSerialiser::checkStream");
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::writeHeader()
	{
		if (mEndian == ENDIAN_AUTO)
			determineEndianness();

		// Header chunk has zero data size
		writeChunkImpl(HEADER_ID, 1);
		writeChunkEnd(HEADER_ID);

		mReadWriteHeader = false;
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::writeChunkBegin(uint32 id, uint16 version /* = 1 */)
	{
		checkStream(false, false, true);

		if (mReadWriteHeader)
			writeHeader();

		if (mEndian == ENDIAN_AUTO)
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
				"Endian mode has not been determined, did you disable header without setting?", 
				"StreamSerialiser::writeChunkBegin");

		writeChunkImpl(id, version);

	}
	//---------------------------------------------------------------------
	void StreamSerialiser::writeChunkEnd(uint32 id)
	{
		checkStream(false, false, true);

		Chunk* c = popChunk(id);

		// update the sizes
		size_t currPos = mStream->tell();
		c->length = static_cast<uint32>(currPos - c->offset - CHUNK_HEADER_SIZE);

		// seek to 'length' position in stream for this chunk
		// skip id (32) and version (16)
		mStream->seek(c->offset + sizeof(uint32) + sizeof(uint16));
		write(&c->length);
		// write updated checksum
		uint32 checksum = calculateChecksum(c);
		write(&checksum);

		// seek back to previous position
		mStream->seek(currPos);

		OGRE_DELETE c;

	}
	//---------------------------------------------------------------------
	StreamSerialiser::Chunk* StreamSerialiser::readChunkImpl()
	{
		Chunk *chunk = OGRE_NEW Chunk();
		chunk->offset = static_cast<uint32>(mStream->tell());
		read(&chunk->id);
		read(&chunk->version);
		read(&chunk->length);
		
		uint32 checksum;
		read(&checksum);
		
		if (checksum != calculateChecksum(chunk))
		{
			// no good, this is an invalid chunk
			uint32 off = chunk->offset;
			OGRE_DELETE chunk;
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
				"Corrupt chunk detected in stream " + mStream->getName() + " at byte "
				+ StringConverter::toString(off), 
				"StreamSerialiser::readChunkImpl");
		}
		else
		{
			return chunk;
		}


	}
	//---------------------------------------------------------------------
	void StreamSerialiser::writeChunkImpl(uint32 id, uint16 version)
	{
		Chunk* c = OGRE_NEW Chunk();
		c->id = id;
		c->version = version;
		c->offset = mStream->tell();
		c->length = 0;

		mChunkStack.push_back(c);

		write(&c->id);
		write(&c->version);
		write(&c->length);
		// write length again, this is just a placeholder for the checksum (to come later)
		write(&c->length);

	}
	//---------------------------------------------------------------------
	void StreamSerialiser::writeData(const void* buf, size_t size, size_t count)
	{
		checkStream(false, false, true);

		size_t totSize = size * count;
		if (mFlipEndian)
		{
			void* pToWrite = OGRE_MALLOC(totSize, MEMCATEGORY_GENERAL);
			memcpy(pToWrite, buf, totSize);

			flipEndian(pToWrite, size, count);
			mStream->write(pToWrite, totSize);

			OGRE_FREE(pToWrite, MEMCATEGORY_GENERAL);
		}
		else
		{
			mStream->write(buf, totSize);
		}

	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Vector2* vec)
	{
		write(vec->ptr(), 2);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Vector3* vec)
	{
		write(vec->ptr(), 3);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Vector4* vec)
	{
		write(vec->ptr(), 4);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Quaternion* q)
	{
		write(q->ptr(), 4);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const String* string)
	{
		mStream->write(string->c_str(), string->size());
		// write terminator (newline)
		char eol = '\n';
		mStream->write(&eol, 1);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Matrix3* m)
	{
		write((*m)[0], 9);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Matrix4* m)
	{
		write((*m)[0], 12);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::readData(void* buf, size_t size, size_t count)
	{
		checkStream(true, true, false);

		size_t totSize = size * count;
		mStream->read(buf, totSize);

		if (mFlipEndian)
			flipEndian(buf, size, count);

	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Vector2* vec)
	{
		read(vec->ptr(), 2);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Vector3* vec)
	{
		read(vec->ptr(), 3);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Vector4* vec)
	{
		read(vec->ptr(), 4);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Quaternion* q)
	{
		read(q->ptr(), 4);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Matrix3* m)
	{
		read((*m)[0], 9);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Matrix4* m)
	{
		read((*m)[0], 12);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(String* string)
	{
		String readStr = mStream->getLine(false);
		string->swap(readStr);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::flipEndian(void * pBase, size_t size, size_t count)
	{
		for (size_t c = 0; c < count; ++c)
		{
			void *pData = (void *)((long)pBase + (c * size));
			char swapByte;
			for(size_t byteIndex = 0; byteIndex < size/2; byteIndex++)
			{
				swapByte = *(char *)((long)pData + byteIndex);
				*(char *)((long)pData + byteIndex) = *(char *)((long)pData + size - byteIndex - 1);
				*(char *)((long)pData + size - byteIndex - 1) = swapByte;
			}
		}

	}
	//---------------------------------------------------------------------
	void StreamSerialiser::flipEndian(void * pData, size_t size)
	{
		flipEndian(pData, size, 1);
	}
	//---------------------------------------------------------------------
	uint32 StreamSerialiser::calculateChecksum(Chunk* c)
	{
		return FastHash((const char*)c, sizeof(uint32) * 2 + sizeof(uint16));
	}
	//---------------------------------------------------------------------
	StreamSerialiser::Chunk* StreamSerialiser::popChunk(uint id)
	{
		if (mChunkStack.empty())
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
			"No active chunk!", "StreamSerialiser::popChunk");

		const Chunk* chunk = mChunkStack.back();
		if (chunk->id != id)
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
			"Incorrect chunk id!", "StreamSerialiser::popChunk");

		Chunk* c = mChunkStack.back();
		mChunkStack.pop_back();
		return c;

	}



}


