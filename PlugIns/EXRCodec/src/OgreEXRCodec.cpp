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
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreImage.h"
#include "OgreException.h"
#include "OgreEXRCodecExports.h"

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

EXRCodec::EXRCodec() 
{
    LogManager::getSingleton().logMessage("EXRCodec initialised");
}
EXRCodec::~EXRCodec() 
{
    LogManager::getSingleton().logMessage("EXRCodec deinitialised");
}

void EXRCodec::decode(const DataStreamPtr& input, const Any& output) const
{
    Image* image = any_cast<Image*>(output);

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
        
        auto format = components==3 ? PF_FLOAT32_RGB : PF_FLOAT32_RGBA;

        // Allocate memory
        image->create(format, width, height);
    
        // Construct frame buffer
        uchar *pixels = image->getData();
        FrameBuffer frameBuffer;
        frameBuffer.insert("R",             // name
                    Slice (PixelType::FLOAT,       // type
                       ((char *) pixels)+0, // base
                       4 * components,      // xStride
                    4 * components * width));    // yStride
        frameBuffer.insert("G",             // name
                    Slice (PixelType::FLOAT,       // type
                       ((char *) pixels)+4, // base
                       4 * components,      // xStride
                    4 * components * width));    // yStride
        frameBuffer.insert("B",             // name
                    Slice (PixelType::FLOAT,       // type
                       ((char *) pixels)+8, // base
                       4 * components,      // xStride
                    4 * components * width));    // yStride
        if(components==4) {
            frameBuffer.insert("A",                 // name
                        Slice (PixelType::FLOAT,           // type
                           ((char *) pixels)+12,        // base
                           4 * components,      // xStride
                        4 * components * width));    // yStride
        }
      
        file.setFrameBuffer (frameBuffer);
        file.readPixels (dw.min.y, dw.max.y);
    } catch (const std::exception &exc) {
        throw(Exception(Exception::ERR_INTERNAL_ERROR,
            "OpenEXR Error",
            exc.what()));
    }
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

#ifndef OGRE_STATIC_LIB
static Codec *mEXRCodec;

extern "C" _OgreEXRPluginExport void dllStartPlugin();
extern "C" _OgreEXRPluginExport void dllStopPlugin();

extern "C" _OgreEXRPluginExport void dllStartPlugin()
{
    mEXRCodec = new EXRCodec;
    Codec::registerCodec( mEXRCodec );
}
extern "C" _OgreEXRPluginExport void dllStopPlugin()
{
    Codec::unregisterCodec( mEXRCodec );
    delete mEXRCodec;
}
#endif
}
