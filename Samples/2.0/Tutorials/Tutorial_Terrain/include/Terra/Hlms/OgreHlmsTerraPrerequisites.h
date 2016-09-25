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
#ifndef _OgreHlmsTerraPrerequisites_H_
#define _OgreHlmsTerraPrerequisites_H_

namespace Ogre
{
    enum TerraTextureTypes
    {
        TERRA_DIFFUSE,
        TERRA_DETAIL_WEIGHT,
        TERRA_DETAIL0,
        TERRA_DETAIL1,
        TERRA_DETAIL2,
        TERRA_DETAIL3,
        TERRA_DETAIL0_NM,
        TERRA_DETAIL1_NM,
        TERRA_DETAIL2_NM,
        TERRA_DETAIL3_NM,
        TERRA_DETAIL_ROUGHNESS0,
        TERRA_DETAIL_ROUGHNESS1,
        TERRA_DETAIL_ROUGHNESS2,
        TERRA_DETAIL_ROUGHNESS3,
        TERRA_DETAIL_METALNESS0,
        TERRA_DETAIL_METALNESS1,
        TERRA_DETAIL_METALNESS2,
        TERRA_DETAIL_METALNESS3,
        TERRA_REFLECTION,
        NUM_TERRA_TEXTURE_TYPES
    };

    class HlmsTerra;
}

#endif
