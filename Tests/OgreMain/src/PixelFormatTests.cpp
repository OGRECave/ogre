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
#include "PixelFormatTests.h"
#include <cstdlib>

// Register the suite
CPPUNIT_TEST_SUITE_REGISTRATION( PixelFormatTests );

void PixelFormatTests::setUp()
{
    size = 4096;
    randomData = new uint8[size];
    temp = new uint8[size];
    temp2 = new uint8[size];
    // Generate reproducable random data
    srand(0);
    for(unsigned int x=0; x<size; x++)
        randomData[x] = (uint8)rand();
}

void PixelFormatTests::tearDown()
{
    delete [] randomData;
    delete [] temp;
    delete [] temp2;
}


void PixelFormatTests::testIntegerPackUnpack()
{

}

void PixelFormatTests::testFloatPackUnpack()
{
    // Float32
    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float r,g,b,a;
    PixelUtil::unpackColour(&r, &g, &b, &a, PF_FLOAT32_RGBA, data);
    CPPUNIT_ASSERT_EQUAL(r, 1.0f);
    CPPUNIT_ASSERT_EQUAL(g, 2.0f);
    CPPUNIT_ASSERT_EQUAL(b, 3.0f);
    CPPUNIT_ASSERT_EQUAL(a, 4.0f);

    // Float16
    setupBoxes(PF_A8B8G8R8, PF_FLOAT16_RGBA);
    dst2.format = PF_A8B8G8R8;
    unsigned int eob = src.getWidth()*4;

    PixelUtil::bulkPixelConversion(src, dst1);
    PixelUtil::bulkPixelConversion(dst1, dst2);

    // Locate errors
    std::stringstream s;
    unsigned int x;
    for(x=0; x<eob; x++) {
        if(temp2[x] != randomData[x])
            s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) randomData[x]
              << "!= " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) temp2[x] << " ";
    }

    // src and dst2 should match
    CPPUNIT_ASSERT_MESSAGE("PF_FLOAT16_RGBA<->PF_A8B8G8R8 conversion was not lossless "+s.str(),
        memcmp(src.data, dst2.data, eob) == 0);
}

// Pure 32 bit float precision brute force pixel conversion; for comparision
void naiveBulkPixelConversion(const PixelBox &src, const PixelBox &dst)
{
    uint8 *srcptr = static_cast<uint8*>(src.data);
    uint8 *dstptr = static_cast<uint8*>(dst.data);
    unsigned int srcPixelSize = PixelUtil::getNumElemBytes(src.format);
    unsigned int dstPixelSize = PixelUtil::getNumElemBytes(dst.format);

    // Calculate pitches+skips in bytes
    int srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
    int srcSliceSkipBytes = src.getSliceSkip()*srcPixelSize;

    int dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
    int dstSliceSkipBytes = dst.getSliceSkip()*dstPixelSize;

	// The brute force fallback
	float r,g,b,a;
	for(size_t z=src.front; z<src.back; z++)
	{
		for(size_t y=src.top; y<src.bottom; y++)
		{
			for(size_t x=src.left; x<src.right; x++)
			{
				PixelUtil::unpackColour(&r, &g, &b, &a, src.format, srcptr);
				PixelUtil::packColour(r, g, b, a, dst.format, dstptr);
				srcptr += srcPixelSize;
				dstptr += dstPixelSize;
			}
			srcptr += srcRowSkipBytes;
			dstptr += dstRowSkipBytes;
		}
		srcptr += srcSliceSkipBytes;
		dstptr += dstSliceSkipBytes;
	}

}

void PixelFormatTests::setupBoxes(PixelFormat srcFormat, PixelFormat dstFormat)
{
    unsigned int width = (size-4) / PixelUtil::getNumElemBytes(srcFormat);
    unsigned int width2 = (size-4) / PixelUtil::getNumElemBytes(dstFormat);
    if(width > width2)
        width = width2;

    src = PixelBox(width, 1, 1, srcFormat, randomData);
	dst1 = PixelBox(width, 1, 1, dstFormat, temp);
	dst2 = PixelBox(width, 1, 1, dstFormat, temp2);

}

void PixelFormatTests::testCase(PixelFormat srcFormat, PixelFormat dstFormat)
{
    setupBoxes(srcFormat, dstFormat);
    // Check end of buffer
    unsigned int eob = dst1.getWidth()*PixelUtil::getNumElemBytes(dstFormat);
    temp[eob] = (unsigned char)0x56;
    temp[eob+1] = (unsigned char)0x23;

    //std::cerr << "["+PixelUtil::getFormatName(srcFormat)+"->"+PixelUtil::getFormatName(dstFormat)+"]" << " " << eob << std::endl;

    // Do pack/unpacking with both naive and optimized version
    PixelUtil::bulkPixelConversion(src, dst1);
    naiveBulkPixelConversion(src, dst2);

    CPPUNIT_ASSERT_EQUAL(temp[eob], (unsigned char)0x56);
    CPPUNIT_ASSERT_EQUAL(temp[eob+1], (unsigned char)0x23);

    std::stringstream s;
    int x;
    s << "src=";
    for(x=0; x<16; x++)
        s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) randomData[x];
    s << " dst=";
    for(x=0; x<16; x++)
        s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) temp[x];
    s << " dstRef=";
    for(x=0; x<16; x++)
        s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) temp2[x];
    s << " ";

    // Compare result
	StringUtil::StrStreamType msg;
	msg << "Conversion mismatch [" << PixelUtil::getFormatName(srcFormat) << 
		"->" << PixelUtil::getFormatName(dstFormat) << "] " << s.str();
    CPPUNIT_ASSERT_MESSAGE(msg.str().c_str(),
		memcmp(dst1.data, dst2.data, eob) == 0);
}

void PixelFormatTests::testBulkConversion()
{
    // Self match
    testCase(PF_A8R8G8B8, PF_A8R8G8B8);
    // Optimized
	testCase(PF_A8R8G8B8,PF_A8B8G8R8);
	testCase(PF_A8R8G8B8,PF_B8G8R8A8);
	testCase(PF_A8R8G8B8,PF_R8G8B8A8);
	testCase(PF_A8B8G8R8,PF_A8R8G8B8);
	testCase(PF_A8B8G8R8,PF_B8G8R8A8);
	testCase(PF_A8B8G8R8,PF_R8G8B8A8);
	testCase(PF_B8G8R8A8,PF_A8R8G8B8);
	testCase(PF_B8G8R8A8,PF_A8B8G8R8);
	testCase(PF_B8G8R8A8,PF_R8G8B8A8);
	testCase(PF_R8G8B8A8,PF_A8R8G8B8);
	testCase(PF_R8G8B8A8,PF_A8B8G8R8);
	testCase(PF_R8G8B8A8,PF_B8G8R8A8);

    testCase(PF_A8B8G8R8, PF_L8);
    testCase(PF_L8, PF_A8B8G8R8);
    testCase(PF_A8R8G8B8, PF_L8);
    testCase(PF_L8, PF_A8R8G8B8);
    testCase(PF_B8G8R8A8, PF_L8);
    testCase(PF_L8, PF_B8G8R8A8);
    testCase(PF_L8, PF_L16);
    testCase(PF_L16, PF_L8);
    testCase(PF_R8G8B8, PF_B8G8R8);
    testCase(PF_B8G8R8, PF_R8G8B8);
    testCase(PF_B8G8R8, PF_R8G8B8);
    testCase(PF_R8G8B8, PF_B8G8R8);
    testCase(PF_R8G8B8, PF_A8R8G8B8);
    testCase(PF_B8G8R8, PF_A8R8G8B8);
    testCase(PF_R8G8B8, PF_A8B8G8R8);
    testCase(PF_B8G8R8, PF_A8B8G8R8);
    testCase(PF_R8G8B8, PF_B8G8R8A8);
    testCase(PF_B8G8R8, PF_B8G8R8A8);
	testCase(PF_A8R8G8B8, PF_R8G8B8);
	testCase(PF_A8R8G8B8, PF_B8G8R8);
	testCase(PF_X8R8G8B8, PF_A8R8G8B8);
	testCase(PF_X8R8G8B8, PF_A8B8G8R8);
	testCase(PF_X8R8G8B8, PF_B8G8R8A8);
	testCase(PF_X8R8G8B8, PF_R8G8B8A8);
	testCase(PF_X8B8G8R8, PF_A8R8G8B8);
	testCase(PF_X8B8G8R8, PF_A8B8G8R8);
	testCase(PF_X8B8G8R8, PF_B8G8R8A8);
	testCase(PF_X8B8G8R8, PF_R8G8B8A8);

    //CPPUNIT_ASSERT_MESSAGE("Conversion mismatch", false);
}

