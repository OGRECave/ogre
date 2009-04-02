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
#include "OgreAxisAlignedBox.h"
#include "OgreNode.h"
#include "OgreRay.h"
#include "OgreSphere.h"

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
		bool autoHeader, RealStorageFormat realFormat)
		: mStream(stream)
		, mEndian(endianMode)
		, mFlipEndian(false)
		, mReadWriteHeader(autoHeader)
		, mRealFormat(realFormat)
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
	void StreamSerialiser::undoReadChunk(uint32 id)
	{
		Chunk* c = popChunk(id);

		checkStream();

		mStream->seek(c->offset);

		OGRE_DELETE c;

	}
	//---------------------------------------------------------------------
	void StreamSerialiser::readChunkEnd(uint32 id)
	{
		Chunk* c = popChunk(id);

		checkStream();

		// skip to the end of the chunk if we were not there already
		// this lets us quite reading a chunk anywhere and have the read marker
		// automatically skip to the next one
		if (mStream->tell() < (c->offset + CHUNK_HEADER_SIZE + c->length))
			mStream->seek(c->offset + CHUNK_HEADER_SIZE + c->length);

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

		// read real storage format
		bool realIsDouble;
		read(&realIsDouble);
		mRealFormat = realIsDouble? REAL_DOUBLE : REAL_FLOAT;

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

		// real format
		bool realIsDouble = (mRealFormat == REAL_DOUBLE);
		write(&realIsDouble);

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
	size_t StreamSerialiser::getOffsetFromChunkStart() const
	{
		checkStream(false, false, false);

		if (mChunkStack.empty())
		{
			return 0;
		}
		else
		{
			size_t pos = mStream->tell();
			size_t diff = pos - mChunkStack.back()->offset;
			if(diff >= CHUNK_HEADER_SIZE)
				return diff - CHUNK_HEADER_SIZE;
			else
				return 0; // not in a chunk?

		}

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
	void StreamSerialiser::write(const Vector2* vec, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++vec)
			write(vec->ptr(), 2);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Vector3* vec, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++vec)
			write(vec->ptr(), 3);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Vector4* vec, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++vec)
			write(vec->ptr(), 4);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Quaternion* q, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++q)
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
	void StreamSerialiser::write(const Matrix3* m, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++m)
			write((*m)[0], 9);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Matrix4* m, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++m)
			write((*m)[0], 12);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const AxisAlignedBox* aabb, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++aabb)
		{
			bool infinite = aabb->isInfinite();
			write(&infinite);
			write(&aabb->getMinimum());
			write(&aabb->getMaximum());
		}		
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Sphere* sphere, size_t count)
	{

		for (size_t i = 0; i < count; ++i, ++sphere)
		{
			write(&sphere->getCenter());
			Real radius = sphere->getRadius();
			write(&radius);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Plane* plane, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++plane)
		{
			write(&plane->normal);
			write(&plane->d);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Ray* ray, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++ray)
		{
			write(&ray->getOrigin());
			write(&ray->getDirection());
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Radian* rad, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++rad)
		{
			Real r = rad->valueRadians();
			write(&r);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Node* node, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++node)
		{
			write(&node->getPosition());
			write(&node->getOrientation());
			write(&node->getScale());
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const bool* val, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++val)
		{
			char c = (*val)? 1 : 0;
			write(&c);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::write(const Real* val, size_t count)
	{
#if OGRE_DOUBLE_PRECISION
		if (mRealFormat == REAL_DOUBLE)
			writeData(val, sizeof(double), count);
		else
			writeDoublesAsFloats(val, count);
#else
		if (mRealFormat == REAL_FLOAT)
			writeData(val, sizeof(float), count);
		else
			writeFloatsAsDoubles(val, count);
#endif
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
	void StreamSerialiser::read(Vector2* vec, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++vec)
		{
			read(vec->ptr(), 2);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Vector3* vec, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++vec)
		{
			read(vec->ptr(), 3);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Vector4* vec, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++vec)
		{
			read(vec->ptr(), 4);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Quaternion* q, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++q)
		{
			read(q->ptr(), 4);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Matrix3* m, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++m)
		{
			read((*m)[0], 9);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Matrix4* m, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++m)
		{
			read((*m)[0], 12);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(String* string)
	{
		String readStr = mStream->getLine(false);
		string->swap(readStr);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Real* val, size_t count)
	{
#if OGRE_DOUBLE_PRECISION
		if (mRealFormat == REAL_DOUBLE)
			readData(val, sizeof(double), count);
		else
			readFloatsAsDoubles(val, count);
#else
		if (mRealFormat == REAL_FLOAT)
			readData(val, sizeof(float), count);
		else
			readDoublesAsFloats(val, count);
#endif

	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(AxisAlignedBox* aabb, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++aabb)
		{
			bool infinite;
			read(&infinite);
			Vector3 tmpMin, tmpMax;
			read(&tmpMin);
			read(&tmpMax);

			if (infinite)
				aabb->setInfinite();
			else
				aabb->setExtents(tmpMin, tmpMax);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Sphere* sphere, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++sphere)
		{
			Vector3 center;
			Real radius;
			read(&center);
			read(&radius);
			sphere->setCenter(center);
			sphere->setRadius(radius);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Plane* plane, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++plane)
		{
			read(&plane->normal);
			read(&plane->d);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Ray* ray, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++ray)
		{
			Vector3 origin, dir;
			read(&origin);
			read(&dir);
			ray->setOrigin(origin);
			ray->setDirection(dir);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Radian* angle, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++angle)
		{
			Real rads;
			read(&rads);
			*angle = Radian(rads);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(Node* node, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++node)
		{
			Vector3 pos, scale;
			Quaternion orient;
			read(&pos);
			read(&orient);
			read(&scale);
			node->setPosition(pos);
			node->setOrientation(orient);
			node->setScale(scale);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::read(bool* val, size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++val)
		{
			char c;
			read(&c);
			*val = (c == 1);
		}
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::writeFloatsAsDoubles(const float* val, size_t count)
	{
		double t = 0;
		writeConverted(val, t, count);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::writeDoublesAsFloats(const double* val, size_t count)
	{
		float t = 0;
		writeConverted(val, t, count);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::readFloatsAsDoubles(double* val, size_t count)
	{
		float t = 0;
		readConverted(val, t, count);
	}
	//---------------------------------------------------------------------
	void StreamSerialiser::readDoublesAsFloats(float* val, size_t count)
	{
		double t = 0;
		readConverted(val, t, count);
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


