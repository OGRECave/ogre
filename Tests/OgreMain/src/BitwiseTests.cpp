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
#include "BitwiseTests.h"
#include "OgreBitwise.h"
// Register the suite

using namespace Ogre;

CPPUNIT_TEST_SUITE_REGISTRATION( BitwiseTests );

void BitwiseTests::setUp()
{
}

void BitwiseTests::tearDown()
{
}


void BitwiseTests::testFixedPointConversion()
{
    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFixed(0x0,1,8), (unsigned int)0x00);
    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFixed(0x1,1,8), (unsigned int)0xFF);
    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFixed(0x2,2,8), (unsigned int)0xAA); // 10101010
    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFixed(0x1,2,8), (unsigned int)0x55); // 01010101
    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFixed(0x2,2,9), (unsigned int)0x155); // 1 01010101
    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFixed(0x1,2,9), (unsigned int)0x0AA); // 0 10101010
    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFixed(0xFE,8,3), (unsigned int)0x7); // 111
    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFixed(0xFE,8,9), (unsigned int)0x1FD); // 111111101

    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFloat(0xFF,8), 1.0f);
    CPPUNIT_ASSERT_EQUAL(Bitwise::fixedToFloat(0x00,8), 0.0f);

    CPPUNIT_ASSERT_EQUAL(Bitwise::floatToFixed(1.0f,8), (unsigned int)0xFF);
    CPPUNIT_ASSERT_EQUAL(Bitwise::floatToFixed(0.0f,8), (unsigned int)0x00);

    // Test clamping
    CPPUNIT_ASSERT_EQUAL(Bitwise::floatToFixed(-1.0f,8), (unsigned int)0x00);
    CPPUNIT_ASSERT_EQUAL(Bitwise::floatToFixed(2.0f,8), (unsigned int)0xFF);

    // Test circular conversion
    bool failed = false;
    for(unsigned int x=0; x<0x100; x++)
        if(Bitwise::floatToFixed(Bitwise::fixedToFloat(x,8),8) != x)
            failed = true;
    CPPUNIT_ASSERT_MESSAGE("circular floatToFixed/fixedToFloat for 8 bit failed",!failed);

    failed = false;
    for(unsigned int x=0; x<0x10; x++)
        if(Bitwise::floatToFixed(Bitwise::fixedToFloat(x,4),4) != x)
            failed = true;
    CPPUNIT_ASSERT_MESSAGE("circular floatToFixed/fixedToFloat for 4 bit failed",!failed);

    failed = false;
    for(unsigned int x=0; x<0x1000; x++)
        if(Bitwise::floatToFixed(Bitwise::fixedToFloat(x,12),12) != x)
            failed = true;
    CPPUNIT_ASSERT_MESSAGE("circular floatToFixed/fixedToFloat for 12 bit failed",!failed);
}

void BitwiseTests::testIntReadWrite()
{
    // Test reading and writing ints
    uint32 testje = 0x12345678;
    assert(Bitwise::intRead(&testje, 4) == 0x12345678);
    uint16 testje2 = 0x1234;
    assert(Bitwise::intRead(&testje2, 2) == 0x1234);
    uint8 testje3 = 0xD3;
    assert(Bitwise::intRead(&testje3, 1) == 0xD3);
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    uint8 testje4[] = {0x12, 0x34, 0x56};
#else
    uint8 testje4[] = {0x56, 0x34, 0x12};
#endif
    assert(Bitwise::intRead(&testje4, 3) == 0x123456);

    Bitwise::intWrite(&testje, 4, 0x87654321);
    assert(testje == 0x87654321);

    Bitwise::intWrite(&testje2, 2, 0x4321);
    assert(testje2 == 0x4321);

    Bitwise::intWrite(&testje3, 1, 0x12);
    assert(testje3 == 0x12);
}

void BitwiseTests::testHalf()
{
    /*
    for(int x=0; x<0x100; x++)
    {
        float f = (float)x/255.0f;
        uint32 fi = *reinterpret_cast<uint32*>(&f);
        uint16 g = Bitwise::floatToHalf(f);
        float h = Bitwise::halfToFloat(g);
        uint32 hi = *reinterpret_cast<uint32*>(&h);
        int i = h*256.0f;
        std::cerr << x << " " << fi << " " << std::hex << std::setw(4) << g << " " << hi << " " << i << std::endl;
    }
    */
}
