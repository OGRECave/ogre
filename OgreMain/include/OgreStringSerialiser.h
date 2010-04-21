#ifndef __StringSerialiser_H__
#define __StringSerialiser_H__

#include <OgrePrerequisites.h>

namespace Ogre{

	/// Serializes data values into a string using sprintf functions
	class StringSerialiser
	{
	private:
		char *mBuffer;
		size_t mBufferSize, mTotalSize;
	public:
		/// Initializes the serialiser with a starting buffer size
		StringSerialiser(size_t size = 0);
		~StringSerialiser();

		/// Returns the generated string
		String str() const;

		StringSerialiser &operator << (const char *str);
		StringSerialiser &operator << (const String &str);
		StringSerialiser &operator << (char val);
		StringSerialiser &operator << (short val);
		StringSerialiser &operator << (int val);
		StringSerialiser &operator << (unsigned char val);
		StringSerialiser &operator << (unsigned short val);
		StringSerialiser &operator << (unsigned int val);
		StringSerialiser &operator << (float val);
		StringSerialiser &operator << (double val);
	private:
		void growBuffer(size_t n);
	};

}

#endif
