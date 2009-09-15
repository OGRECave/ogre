/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "StreamSerialiserTests.h"
#include "OgreStreamSerialiser.h"
#include "OgreFileSystem.h"
#include "OgreException.h"
#include "OgreVector3.h"

using namespace Ogre;

// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( StreamSerialiserTests );

void StreamSerialiserTests::setUp()
{
}
void StreamSerialiserTests::tearDown()
{
}

void StreamSerialiserTests::testWriteBasic()
{
	FileSystemArchive arch("./", "FileSystem");
	arch.load();

	String fileName = "testSerialiser.dat";
	Vector3 aTestVector(0.3, 15.2, -12.0);
	String aTestString = "Some text here";
	int aTestValue = 99;
	uint32 chunkID = StreamSerialiser::makeIdentifier("TEST");
	// write the data
	{

		DataStreamPtr stream = arch.create(fileName);

		StreamSerialiser serialiser(stream);

		serialiser.writeChunkBegin(chunkID);


		serialiser.write(&aTestVector);
		serialiser.write(&aTestString);
		serialiser.write(&aTestValue);
		serialiser.writeChunkEnd(chunkID);
	}

	// read it back
	{

		DataStreamPtr stream = arch.open(fileName);

		StreamSerialiser serialiser(stream);

		const StreamSerialiser::Chunk* c = serialiser.readChunkBegin();

		CPPUNIT_ASSERT_EQUAL(chunkID, c->id);
		CPPUNIT_ASSERT_EQUAL(sizeof(Vector3) + sizeof(int) + aTestString.size() + 4, (size_t)c->length);

		Vector3 inVector;
		String inString;
		int inValue;

		serialiser.read(&inVector);
		serialiser.read(&inString);
		serialiser.read(&inValue);
		serialiser.readChunkEnd(chunkID);

		CPPUNIT_ASSERT_EQUAL(aTestVector, inVector);
		CPPUNIT_ASSERT_EQUAL(aTestString, inString);
		CPPUNIT_ASSERT_EQUAL(aTestValue, inValue);

	}



	arch.remove(fileName);

	CPPUNIT_ASSERT(!arch.exists(fileName));
}
