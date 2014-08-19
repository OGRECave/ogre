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
#ifndef _OgrePbsMobileShaderCreationData_H_
#define _OgrePbsMobileShaderCreationData_H_

#include "OgreHlmsPbsMobilePrerequisites.h"
#include "OgrePrerequisites.h"
#include "OgreVector4.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    struct PbsUvAtlasParams
    {
        float uOffset;
        float vOffset;
        float invDivisor;
        PbsUvAtlasParams() : uOffset( 0 ), vOffset( 0 ), invDivisor( 1.0f ) {}
    };

    struct PbsMobileShaderCreationData
    {
        uint8   uvSource[NUM_PBSM_SOURCES];
        uint8   blendModes[4];
        uint8   mFresnelTypeSizeBytes;              //4 if mFresnel is float, 12 if it is vec3
        float   mFresnelR, mFresnelG, mFresnelB;    //F0
        PbsUvAtlasParams mUvAtlasParams[4];
        float   mNormalMapWeight;
        float   mDetailNormalWeight[4];
        float   mDetailWeight[4];
        Vector4 mDetailsOffsetScale[8];

        PbsMobileShaderCreationData() :
            mFresnelTypeSizeBytes( 4 ),
            mFresnelR( 0.818f ), mFresnelG( 0.818f ), mFresnelB( 0.818f ),
            mNormalMapWeight( 1.0f )
        {
            mDetailNormalWeight[0] = mDetailNormalWeight[1] = 1.0f;
            mDetailNormalWeight[2] = mDetailNormalWeight[3] = 1.0f;
            mDetailWeight[0] = mDetailWeight[1] = mDetailWeight[2] = mDetailWeight[3] = 1.0f;
            for( size_t i=0; i<8; ++i )
                mDetailsOffsetScale[i] = Vector4( 0, 0, 1, 1 );
            memset( uvSource, 0, sizeof( uvSource ) );
            memset( blendModes, 0, sizeof( blendModes ) );
        }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
