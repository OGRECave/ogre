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
#ifndef _OgreShaderParams_H_
#define _OgreShaderParams_H_

#include "OgreGpuProgramParams.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    /** The purpose of this class is to contain a set of both auto and
        manual parameters that may apply to multiple shaders; without
        having the shader to be created first (the drawback of GpuProgramParameters).
    @par
        Useful when loading an Hlms material via JSON (e.g. a Compute Job)
        which may generate multiple shaders depending on the property settings.
    @par
        Parameters are kept unsorted in mParams
    */
    class _OgreExport ShaderParams : public PassAlloc
    {
    public:
        enum ElementType
        {
            ElementInt,
            ElementUInt,
            ElementFloat
        };

        struct AutoParam
        {
            ElementType                             extreParamType;
            double                                  extraParamValue;
            GpuProgramParameters::AutoConstantType  acType;
        };
        struct ManualParam
        {
            ElementType elementType;
            /// Size in bytes from dataBytes
            uint8       dataSizeBytes;
            /// Enough data to cover up to a 4x4 matrix.
            /// Gets reinterpret_cast'ed based on elementType
            uint8       dataBytes[64];
        };
        struct ShaderParam
        {
            String  name;
            bool    isAutomatic;

            union
            {
                AutoParam ap;
                ManualParam mp;
            };
        };

        typedef vector<ShaderParam>::type ShaderParamVec;

        /// Don't log exceptions about missing parameters
        bool mSilenceMissingParameterWarnings;
        ShaderParamVec mParams;

        ShaderParams();

        void updateParameters( GpuProgramPtr &gpuProgram );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
