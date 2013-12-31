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
#include "OgreStableHeaders.h"

#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "OgreEXRCodec.h"

#include "O_IStream.h"

#include <cmath>
#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>

using namespace Imath;
using namespace Imf;

namespace Ogre {

void writeEXRHalf(OStream *ost, const float *pixels,
	      int width, int height, int components) 
{
	//assert(components==3 || components==4);
	// TODO: throw std::exception if invalid number of components

	Header header (width, height);
	header.channels().insert ("R", Channel (HALF));
	header.channels().insert ("G", Channel (HALF));
	header.channels().insert ("B", Channel (HALF));
	if(components==4)
		header.channels().insert ("A", Channel (HALF));

	// Convert data to half
	half *data = new half [width*height*components];
	
	std::copy(pixels, pixels+(width*height*components), data);
	
	// And save it
	OutputFile file (*ost, header);
	FrameBuffer frameBuffer;

	frameBuffer.insert("R",				// name
			    Slice (HALF,		// type
				   ((char *) data)+0,	// base
				   2 * components,		// xStride
				   2 * components * width));	// yStride
	frameBuffer.insert("G",				// name
			    Slice (HALF,		// type
				   ((char *) data)+2,	// base
				   2 * components,		// xStride
				   2 * components * width));	// yStride
	frameBuffer.insert("B",				// name
			    Slice (HALF,		// type
				   ((char *) data)+4,	// base
				   2 * components,		// xStride
				   2 * components * width));	// yStride
	if(components==4) {
		frameBuffer.insert("A",					// name
				    Slice (HALF,			// type
					   ((char *) data)+6,		// base
					   2 * components,		// xStride
					   2 * components * width));	// yStride
	}

	file.setFrameBuffer(frameBuffer);
	file.writePixels(height);
	delete data;
}


EXRCodec::EXRCodec() 
{
    LogManager::getSingleton().logMessage("EXRCodec initialised");
}
EXRCodec::~EXRCodec() 
{
    LogManager::getSingleton().logMessage("EXRCodec deinitialised");
}

DataStreamPtr EXRCodec::code(MemoryDataStreamPtr& input, CodecDataPtr& pData) const
{

}

Codec::DecodeResult EXRCodec::decode(DataStreamPtr& input) const
{
    ImageData * imgData = new ImageData;
    MemoryDataStreamPtr output;

    try {
        // Make a mutable clone of input to be able to change file pointer
        MemoryDataStream myIn(input);
    
        // Now we can simulate an OpenEXR file with that
        O_IStream str(myIn, "SomeChunk.exr");
        InputFile file(str);
    
        Box2i dw = file.header().dataWindow();
        int width  = dw.max.x - dw.min.x + 1;
        int height = dw.max.y - dw.min.y + 1;
        int components = 3;
    
        // Alpha channel present?
        const ChannelList &channels = file.header().channels();
        if(channels.findChannel("A"))
            components = 4;
        
        // Allocate memory
        output.bind(new MemoryDataStream(width*height*components*4));
    
        // Construct frame buffer
        uchar *pixels = output->getPtr();
        FrameBuffer frameBuffer;
        frameBuffer.insert("R",             // name
                    Slice (FLOAT,       // type
                       ((char *) pixels)+0, // base
                       4 * components,      // xStride
                    4 * components * width));    // yStride
        frameBuffer.insert("G",             // name
                    Slice (FLOAT,       // type
                       ((char *) pixels)+4, // base
                       4 * components,      // xStride
                    4 * components * width));    // yStride
        frameBuffer.insert("B",             // name
                    Slice (FLOAT,       // type
                       ((char *) pixels)+8, // base
                       4 * components,      // xStride
                    4 * components * width));    // yStride
        if(components==4) {
            frameBuffer.insert("A",                 // name
                        Slice (FLOAT,           // type
                           ((char *) pixels)+12,        // base
                           4 * components,      // xStride
                        4 * components * width));    // yStride
        }
      
        file.setFrameBuffer (frameBuffer);
        file.readPixels (dw.min.y, dw.max.y);
    
        imgData->format = components==3 ? PF_FLOAT32_RGB : PF_FLOAT32_RGBA;
        imgData->width = width;
        imgData->height = height;
        imgData->depth = 1;
        imgData->size = width*height*components*4;
        imgData->num_mipmaps = 0;
        imgData->flags = 0;
    } catch (const std::exception &exc) {
        delete imgData;
        throw(Exception(Exception::ERR_INTERNAL_ERROR,
            "OpenEXR Error",
            exc.what()));
    }
    
    DecodeResult ret;
    ret.first = output; 
    ret.second = CodecDataPtr(imgData);
    return ret;
}

void EXRCodec::codeToFile(MemoryDataStreamPtr& input, const String& outFileName, CodecDataPtr& pData) const
{

}


String EXRCodec::getType() const 
{
    return "exr";
}


String EXRCodec::magicNumberToFileExt(const char* magicNumberPtr, size_t maxbytes) const
{
  // look for OpenEXR magic number
  if (maxbytes >= 4 
      && magicNumberPtr[0] == 0x76
      && magicNumberPtr[1] == 0x2f
      && magicNumberPtr[2] == 0x31
      && magicNumberPtr[3] == 0x01)
    return "exr";
    
  return "";
}

}
