/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2012 Torus Knot Software Ltd
 
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
