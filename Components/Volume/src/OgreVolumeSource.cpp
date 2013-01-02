/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd
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
#include "OgreVolumeSource.h"

#include "OgreRoot.h"
#include "OgreDeflate.h"
#include "OgreStreamSerialiser.h"
#include "OgreBitwise.h"

namespace Ogre {
namespace Volume {
    
    const uint32 Source::VOLUME_CHUNK_ID = StreamSerialiser::makeIdentifier("VOLU");
    const uint16 Source::VOLUME_CHUNK_VERSION = 1;

    Source::~Source(void)
    {
    }

    void Source::serialize(const Vector3 &from, const Vector3 &to, float voxelWidth, const String &file)
    {
     
        
        // Compress
        DataStreamPtr stream = Root::getSingleton().createFileStream(file);
        DataStreamPtr compressStream(OGRE_NEW DeflateStream(file, stream));
        StreamSerialiser ser(compressStream);
        ser.writeChunkBegin(VOLUME_CHUNK_ID, VOLUME_CHUNK_VERSION);

        // Write Metadata
        ser.write(&from);
        ser.write(&to);
        ser.write(&voxelWidth);

        // Go over the volume and write the density data.
        Vector3 pos;
        Real max = (from - to).length() / (Real)8.0;
        uint16 val;
        Real realVal;
        for (pos.z = from.z; pos.z <= to.z; pos.z += voxelWidth)
        {
            for (pos.x = from.x; pos.x <= to.x; pos.x += voxelWidth)
            {
                for (pos.y = from.y; pos.y <= to.y; pos.y += voxelWidth)
                {
                    realVal = Math::Clamp<Real>(getValue(pos), -max, max);
                    val = Bitwise::floatToHalf(realVal);
                    ser.write(&val);
                }
            }
        }

        ser.writeChunkEnd(VOLUME_CHUNK_ID);
        
    }

}
}