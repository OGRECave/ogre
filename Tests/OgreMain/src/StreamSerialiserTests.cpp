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
		CPPUNIT_ASSERT_EQUAL(sizeof(Vector3) + sizeof(int) + aTestString.size() + 1, c->length);

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
