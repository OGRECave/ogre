/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include <gtest/gtest.h>

#include "OgreBitwise.h"
#include "OgreStringConverter.h"

using namespace Ogre;

//--------------------------------------------------------------------------
TEST(BitwiseTests,FixedPointConversion)
{
    EXPECT_EQ(Bitwise::fixedToFixed(0x0,  1,8), (unsigned int)0x00);
    EXPECT_EQ(Bitwise::fixedToFixed(0x1,  1,8), (unsigned int)0xFF);
    EXPECT_EQ(Bitwise::fixedToFixed(0x2,  2,8), (unsigned int)0xAA);     // 10101010
    EXPECT_EQ(Bitwise::fixedToFixed(0x1,  2,8), (unsigned int)0x55);     // 01010101
    EXPECT_EQ(Bitwise::fixedToFixed(0x2,  2,9), (unsigned int)0x155);    // 1 01010101
    EXPECT_EQ(Bitwise::fixedToFixed(0x1,  2,9), (unsigned int)0x0AA);    // 0 10101010
    EXPECT_EQ(Bitwise::fixedToFixed(0xFE, 8,3), (unsigned int)0x7);      // 111
    EXPECT_EQ(Bitwise::fixedToFixed(0xFE, 8,9), (unsigned int)0x1FD);    // 111111101

    EXPECT_EQ(Bitwise::fixedToFloat(0xFF, 8), 1.0f);
    EXPECT_EQ(Bitwise::fixedToFloat(0x00, 8), 0.0f);

    EXPECT_EQ(Bitwise::floatToFixed(1.0f, 8), (unsigned int)0xFF);
    EXPECT_EQ(Bitwise::floatToFixed(0.0f, 8), (unsigned int)0x00);

    // Test clamping
    EXPECT_EQ(Bitwise::floatToFixed(-1.0f,8), (unsigned int)0x00);
    EXPECT_EQ(Bitwise::floatToFixed(2.0f, 8), (unsigned int)0xFF);

    // Test circular conversion
    bool failed = false;
    for(unsigned int x = 0; x < 0x0010; x++)
        if(Bitwise::floatToFixed(Bitwise::fixedToFloat(x, 4), 4) != x)
            failed = true;
    EXPECT_TRUE(!failed) << "circular floatToFixed/fixedToFloat for 4 bit failed";

    failed = false;
    for(unsigned int x = 0; x < 0x0100; x++)
        if(Bitwise::floatToFixed(Bitwise::fixedToFloat(x, 8), 8) != x)
            failed = true;
    EXPECT_TRUE(!failed) << "circular floatToFixed/fixedToFloat for 8 bit failed";

    failed = false;
    for(unsigned int x = 0; x < 0xFFE; x++) // originally loop ran till 0x1000, but precision issues sometimes prevent that
        if(Bitwise::floatToFixed(Bitwise::fixedToFloat(x, 12), 12) != x)
            failed = true;
    EXPECT_TRUE(!failed) << "circular floatToFixed/fixedToFloat for 12 bit failed";
}
//--------------------------------------------------------------------------
TEST(BitwiseTests,IntReadWrite)
{    
    // Test reading and writing integers
    uint32 testje = 0x12345678;
    EXPECT_TRUE(Bitwise::intRead(&testje, 4) == 0x12345678);
    uint16 testje2 = 0x1234;
    EXPECT_TRUE(Bitwise::intRead(&testje2, 2) == 0x1234);
    uint8 testje3 = 0xD3;
    EXPECT_TRUE(Bitwise::intRead(&testje3, 1) == 0xD3);
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    uint8 testje4[] = {0x12, 0x34, 0x56};
#else
    uint8 testje4[] = {0x56, 0x34, 0x12};
#endif
    EXPECT_TRUE(Bitwise::intRead(&testje4, 3) == 0x123456);

    Bitwise::intWrite(&testje, 4, 0x87654321);
    EXPECT_TRUE(testje == 0x87654321);

    Bitwise::intWrite(&testje2, 2, 0x4321);
    EXPECT_TRUE(testje2 == 0x4321);

    Bitwise::intWrite(&testje3, 1, 0x12);
    EXPECT_TRUE(testje3 == 0x12);
}
//--------------------------------------------------------------------------
TEST(BitwiseTests,Half)
{
    for(float f : {1.f, -1.f, float(INFINITY), 65504.f})
    {
        uint16 g = Bitwise::floatToHalf(f);
        float h = Bitwise::halfToFloat(g);
        EXPECT_EQ(f, h);
    }
}
//--------------------------------------------------------------------------
