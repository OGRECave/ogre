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
#include <iomanip>


// Register the test suite

//--------------------------------------------------------------------------
void PixelFormatTests::SetUp()
{    
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
void PixelFormatTests::TearDown()
{
    delete [] mRandomData;
    delete [] mTemp;
    delete [] mTemp2;
}
//--------------------------------------------------------------------------
TEST_F(PixelFormatTests,IntegerPackUnpack)
{
    ColourValue src(1./1023, 512./1023, 0, 1./3);

    // naive implementation
    const uint16 ir = static_cast<uint16>( Math::saturate( src.r ) * 1023.0f + 0.5f );
    const uint16 ig = static_cast<uint16>( Math::saturate( src.g ) * 1023.0f + 0.5f );
    const uint16 ib = static_cast<uint16>( Math::saturate( src.b ) * 1023.0f + 0.5f );
    const uint16 ia = static_cast<uint16>( Math::saturate( src.a ) * 3.0f + 0.5f );
    uint32 ref = (ia << 30u) | (ir << 20u) | (ig << 10u) | (ib);

    uint32 packed;
    PixelUtil::packColour(src, PF_A2R10G10B10, &packed);

    EXPECT_EQ(packed, ref);

    ColourValue dst;
    PixelUtil::unpackColour(dst, PF_A2R10G10B10, &packed);

    EXPECT_EQ(src, dst);
}
//--------------------------------------------------------------------------
TEST_F(PixelFormatTests,FloatPackUnpack)
{
    // Float32
    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float r,g,b,a;
    PixelUtil::unpackColour(&r, &g, &b, &a, PF_FLOAT32_RGBA, data);
    EXPECT_EQ(r, 1.0f);
    EXPECT_EQ(g, 2.0f);
    EXPECT_EQ(b, 3.0f);
    EXPECT_EQ(a, 4.0f);

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
    EXPECT_TRUE(memcmp(mSrc.data, mDst2.data, eob) == 0) << "PF_FLOAT16_RGBA<->PF_A8B8G8R8 conversion was not lossless "+s.str();
}
//--------------------------------------------------------------------------
// Pure 32 bit float precision brute force pixel conversion; for comparison
static void naiveBulkPixelConversion(const PixelBox &src, const PixelBox &dst)
{
    uint8 *srcptr = src.data;
    uint8 *dstptr = dst.data;
    size_t srcPixelSize = PixelUtil::getNumElemBytes(src.format);
    size_t dstPixelSize = PixelUtil::getNumElemBytes(dst.format);

    // Calculate pitches+skips in bytes
    unsigned long srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
    unsigned long srcSliceSkipBytes = src.getSliceSkip()*srcPixelSize;

    unsigned long dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
    unsigned long dstSliceSkipBytes = dst.getSliceSkip()*dstPixelSize;

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

    EXPECT_EQ(mTemp[eob], (unsigned char)0x56);
    EXPECT_EQ(mTemp[eob+1], (unsigned char)0x23);

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
    StringStream msg;
    msg << "Conversion mismatch [" << PixelUtil::getFormatName(srcFormat) << 
        "->" << PixelUtil::getFormatName(dstFormat) << "] " << s.str();
    EXPECT_TRUE(memcmp(mDst1.data, mDst2.data, eob) == 0) << msg.str().c_str();
}
//--------------------------------------------------------------------------
TEST_F(PixelFormatTests,BulkConversion)
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

    testCase(PF_A8B8G8R8, PF_R8);
    testCase(PF_R8, PF_A8B8G8R8);
    testCase(PF_A8R8G8B8, PF_R8);
    testCase(PF_R8, PF_A8R8G8B8);
    testCase(PF_B8G8R8A8, PF_R8);
    testCase(PF_R8, PF_B8G8R8A8);

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

