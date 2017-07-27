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

#ifndef _OgreCbShaderBuffer_H_
#define _OgreCbShaderBuffer_H_

#include "CommandBuffer/OgreCbCommon.h"
#include "OgreCommon.h"

namespace Ogre
{
    struct _OgreExport CbShaderBuffer : CbBase
    {
        uint16          slot;
        BufferPacked   *bufferPacked;
        uint32          bindOffset;
        uint32          bindSizeBytes;

        CbShaderBuffer( ShaderType shaderType, uint16 _slot, ConstBufferPacked *_bufferPacked,
                        uint32 _bindOffset, uint32 _bindSizeBytes );

        CbShaderBuffer( ShaderType shaderType, uint16 _slot, TexBufferPacked *_bufferPacked,
                        uint32 _bindOffset, uint32 _bindSizeBytes );
    };

    struct _OgreExport CbIndirectBuffer : CbBase
    {
        IndirectBufferPacked *indirectBuffer;

        CbIndirectBuffer( IndirectBufferPacked *_indirectBuffer );
    };
}

#endif
