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


