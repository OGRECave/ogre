#include "OgreStringSerialiser.h"

#include <cstdio>

namespace Ogre{
	
	StringSerialiser::StringSerialiser(size_t size)
		:mBuffer(0), mBufferSize(0), mTotalSize(0)
	{
		if(size > 0)
		{
			mBuffer = (char*)malloc(size);
			mTotalSize = size;
		}
	}

	StringSerialiser::~StringSerialiser()
	{
		if(mBuffer)
			free(mBuffer);
	}

	String StringSerialiser::str() const
	{
		return mBuffer ? std::string(mBuffer, mBufferSize) : std::string();
	}

	StringSerialiser &StringSerialiser::operator << (const char *str)
	{
		size_t n = strlen(str);
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
		memcpy(dst, str, n);
		mBufferSize += n;

		return *this;
	}

	StringSerialiser &StringSerialiser::operator << (const String &str)
	{
		size_t n = str.size();
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
		memcpy(dst, str.c_str(), n);
		mBufferSize += n;

		return *this;
	}

	StringSerialiser &StringSerialiser::operator << (char val)
	{
		size_t n = 1;
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
#ifdef WIN32
		n = _snprintf(dst, n, "%c", val);
#else
		n = snprintf(dst, n, "%c", val);
#endif
		mBufferSize += n;

		return *this;
	}

	StringSerialiser &StringSerialiser::operator << (short val)
	{
		size_t n = 5;
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
#ifdef WIN32
		n = _snprintf(dst, n, "%hd", val);
#else
		n = snprintf(dst, n, "%hd", val);
#endif
		mBufferSize += n;

		return *this;
	}

	StringSerialiser &StringSerialiser::operator << (int val)
	{
		size_t n = 10;
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
#ifdef WIN32
		n = _snprintf(dst, n, "%d", val);
#else
		n = snprintf(dst, n, "%d", val);
#endif
		mBufferSize += n;

		return *this;
	}

	StringSerialiser &StringSerialiser::operator << (unsigned char val)
	{
		size_t n = 3;
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
#ifdef WIN32
		n = _snprintf(dst, n, "%u", val);
#else
		n = snprintf(dst, n, "%u", val);
#endif
		mBufferSize += n;

		return *this;
	}

	StringSerialiser &StringSerialiser::operator << (unsigned short val)
	{
		size_t n = 5;
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
#ifdef WIN32
		n = _snprintf(dst, n, "%hu", val);
#else
		n = snprintf(dst, n, "%hu", val);
#endif
		mBufferSize += n;

		return *this;
	}

	StringSerialiser &StringSerialiser::operator << (unsigned int val)
	{
		size_t n = 10;
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
#ifdef WIN32
		n = _snprintf(dst, n, "%u", val);
#else
		n = snprintf(dst, n, "%u", val);
#endif
		mBufferSize += n;

		return *this;
	}

	StringSerialiser &StringSerialiser::operator << (float val)
	{
		size_t n = 100;
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
#ifdef WIN32
		n = _snprintf(dst, n, "%f", val);
#else
		n = snprintf(dst, n, "%f", val);
#endif
		mBufferSize += n;

		return *this;
	}

	StringSerialiser &StringSerialiser::operator << (double val)
	{
		size_t n = 100;
		growBuffer(n);

		char *dst = mBuffer + mBufferSize;
#ifdef WIN32
		n = _snprintf(dst, n, "%f", val);
#else
		n = snprintf(dst, n, "%f", val);
#endif
		mBufferSize += n;

		return *this;
	}

	void StringSerialiser::growBuffer(size_t n)
	{
		if(mBuffer == 0)
		{
			mBuffer = (char*)malloc(128);
			mTotalSize = 128;
		}
		else
		{
			if(mBufferSize + n > mTotalSize)
			{
				n = n > 128 ? n : 128;

				char *temp = (char*)malloc(mTotalSize + n);
				memcpy(temp, mBuffer, mTotalSize);
				free(mBuffer);

				mBuffer = temp;
				mTotalSize += n;
			}
		}
	}

}
