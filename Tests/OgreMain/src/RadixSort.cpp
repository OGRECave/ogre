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
#include "RadixSortTests.h"
#include "OgreRadixSort.h"
#include "OgreMath.h"

using namespace Ogre;

// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( RadixSortTests );

void RadixSortTests::setUp()
{
	srand(time(0));
}
void RadixSortTests::tearDown()
{
}

class FloatSortFunctor
{
public:
	float operator()(const float& p) const
	{
		return p;
	}

};
class IntSortFunctor
{
public:
	int operator()(const int& p) const
	{
		return p;
	}

};

class UnsignedIntSortFunctor
{
public:
	unsigned int operator()(const unsigned int& p) const
	{
		return p;
	}

};


void RadixSortTests::testFloatVector()
{
	std::vector<float> container;
	FloatSortFunctor func;
	RadixSort<std::vector<float>, float, float> sorter;

	for (int i = 0; i < 1000; ++i)
	{
		container.push_back((float)Math::RangeRandom(-1e10, 1e10));
	}

	sorter.sort(container, func);

	std::vector<float>::iterator v = container.begin();
	float lastValue = *v++;
	for (;v != container.end(); ++v)
	{
		CPPUNIT_ASSERT(*v >= lastValue);
		lastValue = *v;
	}


}
void RadixSortTests::testFloatList()
{
	std::list<float> container;
	FloatSortFunctor func;
	RadixSort<std::list<float>, float, float> sorter;

	for (int i = 0; i < 1000; ++i)
	{
		container.push_back((float)Math::RangeRandom(-1e10, 1e10));
	}

	sorter.sort(container, func);

	std::list<float>::iterator v = container.begin();
	float lastValue = *v++;
	for (;v != container.end(); ++v)
	{
		CPPUNIT_ASSERT(*v >= lastValue);
		lastValue = *v;
	}
}
void RadixSortTests::testUnsignedIntList()
{
	std::list<unsigned int> container;
	UnsignedIntSortFunctor func;
	RadixSort<std::list<unsigned int>, unsigned int, unsigned int> sorter;

	for (int i = 0; i < 1000; ++i)
	{
		container.push_back((unsigned int)Math::RangeRandom(0, 1e10));
	}

	sorter.sort(container, func);

	std::list<unsigned int>::iterator v = container.begin();
	unsigned int lastValue = *v++;
	for (;v != container.end(); ++v)
	{
		CPPUNIT_ASSERT(*v >= lastValue);
		lastValue = *v;
	}
}
void RadixSortTests::testIntList()
{
	std::list<int> container;
	IntSortFunctor func;
	RadixSort<std::list<int>, int, int> sorter;

	for (int i = 0; i < 1000; ++i)
	{
		container.push_back((int)Math::RangeRandom(-1e10, 1e10));
	}

	sorter.sort(container, func);

	std::list<int>::iterator v = container.begin();
	int lastValue = *v++;
	for (;v != container.end(); ++v)
	{
		CPPUNIT_ASSERT(*v >= lastValue);
		lastValue = *v;
	}
}
void RadixSortTests::testUnsignedIntVector()
{
	std::vector<unsigned int> container;
	UnsignedIntSortFunctor func;
	RadixSort<std::vector<unsigned int>, unsigned int, unsigned int> sorter;

	for (int i = 0; i < 1000; ++i)
	{
		container.push_back((unsigned int)Math::RangeRandom(0, 1e10));
	}

	sorter.sort(container, func);

	std::vector<unsigned int>::iterator v = container.begin();
	unsigned int lastValue = *v++;
	for (;v != container.end(); ++v)
	{
		CPPUNIT_ASSERT(*v >= lastValue);
		lastValue = *v;
	}
}
void RadixSortTests::testIntVector()
{
	std::vector<int> container;
	IntSortFunctor func;
	RadixSort<std::vector<int>, int, int> sorter;

	for (int i = 0; i < 1000; ++i)
	{
		container.push_back((int)Math::RangeRandom(-1e10, 1e10));
	}

	sorter.sort(container, func);

	std::vector<int>::iterator v = container.begin();
	int lastValue = *v++;
	for (;v != container.end(); ++v)
	{
		CPPUNIT_ASSERT(*v >= lastValue);
		lastValue = *v;
	}
}


