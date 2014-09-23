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
#include "PixelFormatTests.h"
#include <cstdlib>

#include "UnitTestSuite.h"

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(PixelFormatTests);

//--------------------------------------------------------------------------
void PixelFormatTests::setUp()
{
    UnitTestSuite::getSingletonPtr()->startTestSetup(__FUNCTION__);
    
    mSize = 4096;
    mRandomData = new uint8[mSize];
    mTemp = new uint8[mSize];
    mTemp2 = new uint8[mSize];

    // Generate reproducible random data
    srand(0);
    for(unsigned int x=0; x<(unsigned int)mSize; x++)
        mRandomData[x] = (uint8)rand();
}
//--------------------------------------------------------------------------
void PixelFormatTests::tearDown()
{
    delete [] mRandomData;
    delete [] mTemp;
    delete [] mTemp2;
}
//--------------------------------------------------------------------------
void PixelFormatTests::testIntegerPackUnpack()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
}
//--------------------------------------------------------------------------
void PixelFormatTests::testFloatPackUnpack()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

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
    mDst2.format = PF_A8B8G8R8;
    unsigned int eob = mSrc.getWidth()*4;

    PixelUtil::bulkPixelConversion(mSrc, mDst1);
    PixelUtil::bulkPixelConversion(mDst1, mDst2);

    // Locate errors
    std::stringstream s;
    unsigned int x;
    for(x=0; x<eob; x++) {
        if(mTemp2[x] != mRandomData[x])
            s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) mRandomData[x]
              << "!= " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) mTemp2[x] << " ";
    }

    // src and dst2 should match
    CPPUNIT_ASSERT_MESSAGE("PF_FLOAT16_RGBA<->PF_A8B8G8R8 conversion was not lossless "+s.str(),
        memcmp(mSrc.data, mDst2.data, eob) == 0);
}
//--------------------------------------------------------------------------
// Pure 32 bit float precision brute force pixel conversion; for comparison
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
//--------------------------------------------------------------------------
void PixelFormatTests::setupBoxes(PixelFormat srcFormat, PixelFormat dstFormat)
{
    unsigned int width = (mSize-4) / PixelUtil::getNumElemBytes(srcFormat);
    unsigned int width2 = (mSize-4) / PixelUtil::getNumElemBytes(dstFormat);
    if(width > width2)
        width = width2;

    mSrc = PixelBox(width, 1, 1, srcFormat, mRandomData);
    mDst1 = PixelBox(width, 1, 1, dstFormat, mTemp);
    mDst2 = PixelBox(width, 1, 1, dstFormat, mTemp2);
}
//--------------------------------------------------------------------------
void PixelFormatTests::testCase(PixelFormat srcFormat, PixelFormat dstFormat)
{
    setupBoxes(srcFormat, dstFormat);
    // Check end of buffer
    unsigned long eob = mDst1.getWidth()*PixelUtil::getNumElemBytes(dstFormat);
    mTemp[eob] = (unsigned char)0x56;
    mTemp[eob+1] = (unsigned char)0x23;

    //std::cerr << "["+PixelUtil::getFormatName(srcFormat)+"->"+PixelUtil::getFormatName(dstFormat)+"]" << " " << eob << std::endl;

    // Do pack/unpacking with both naive and optimized version
    PixelUtil::bulkPixelConversion(mSrc, mDst1);
    naiveBulkPixelConversion(mSrc, mDst2);

    CPPUNIT_ASSERT_EQUAL(mTemp[eob], (unsigned char)0x56);
    CPPUNIT_ASSERT_EQUAL(mTemp[eob+1], (unsigned char)0x23);

    std::stringstream s;
    int x;
    s << "src=";
    for(x=0; x<16; x++)
        s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) mRandomData[x];
    s << " dst=";
    for(x=0; x<16; x++)
        s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) mTemp[x];
    s << " dstRef=";
    for(x=0; x<16; x++)
        s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) mTemp2[x];
    s << " ";

    // Compare result
	StringUtil::StrStreamType msg;
	msg << "Conversion mismatch [" << PixelUtil::getFormatName(srcFormat) << 
		"->" << PixelUtil::getFormatName(dstFormat) << "] " << s.str();
    CPPUNIT_ASSERT_MESSAGE(msg.str().c_str(),
        memcmp(mDst1.data, mDst2.data, eob) == 0);
}
//--------------------------------------------------------------------------
void PixelFormatTests::testBulkConversion()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

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
}
//--------------------------------------------------------------------------

