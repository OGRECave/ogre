#include "ACPChunk.h"

#include <fstream>
#include <string.h>
#include <zlib.h>

ACPChunk::ACPChunk()
:mBuffer(0), mCompressed(false), mSize(0), mFullSize(0)
{
}

ACPChunk::~ACPChunk()
{
	free(mBuffer);
}

void ACPChunk::load(const char *name, const char *path)
{
	std::fstream stream(path, std::ios_base::in|std::ios_base::binary);
	if(stream.is_open())
	{
		stream.seekg(0, std::ios_base::end);
		mSize = mFullSize = stream.tellg();
		stream.seekg(0, std::ios_base::beg);

		if(mBuffer)
			free(mBuffer);

		mBuffer = malloc(mSize);
		stream.read((char*)mBuffer, mSize);
		stream.close();

		mCompressed = false;
		mName = name;
	}
}

void ACPChunk::load(const char *name, void *buffer, size_t size)
{
	mCompressed = false;
	mName = name;
	mSize = size;
	mFullSize = size;

	if(mBuffer)
		free(mBuffer);

	mBuffer = buffer;
}

void ACPChunk::loadCompressed(const char *name, void *buffer, size_t size)
{
	mCompressed = true;
	mName = name;
	mSize = size;

	if(mBuffer)
		free(mBuffer);

	mBuffer = buffer;

	if(size > 4)
	{
		size_t *ptr = (size_t*)buffer;
		mFullSize = *ptr;
	}
}

void ACPChunk::compress()
{
	if(!mCompressed)
	{
		uLong bufferSize = compressBound(mSize);
		void *temp = malloc(bufferSize);

		if(::compress((Bytef*)temp, &bufferSize, (Bytef*)mBuffer, mSize) == Z_OK)
		{
			// Copy buffer over
			free(mBuffer);

			mBuffer = malloc(bufferSize + sizeof(mSize));
			
			size_t *iptr = (size_t*)mBuffer;
			*iptr++ = (size_t)mFullSize;

			memcpy((void*)iptr, temp, bufferSize);
			mSize = bufferSize + sizeof(mSize);
			mCompressed = true;	
		}

		free(temp);
	}
}

void ACPChunk::uncompress()
{
	if(mCompressed)
	{
		size_t *tempPtr = (size_t*)mBuffer;
		tempPtr++;

		uLong bufferSize = mFullSize;
		void *buffer = malloc(mFullSize);
		if(::uncompress((Bytef*)buffer, &bufferSize, (const Bytef*)tempPtr, mSize - sizeof(mSize)) == Z_OK)
		{
			free(mBuffer);
			mBuffer = buffer;
			mSize = mFullSize;
			mCompressed = false;
		}
		else
		{
			free(buffer);
		}
	}
}

bool ACPChunk::getCompressed() const
{
	return mCompressed;
}

size_t ACPChunk::getSize() const
{
	return mSize;
}

size_t ACPChunk::getFullSize() const
{
	return mFullSize;
}

const std::string &ACPChunk::getName() const
{
	return mName;
}

void *ACPChunk::getBuffer() const
{
	return mBuffer;
}
