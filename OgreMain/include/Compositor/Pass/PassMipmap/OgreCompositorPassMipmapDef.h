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

#ifndef __CompositorPassMipmapDef_H__
#define __CompositorPassMipmapDef_H__

#include "OgreHeaderPrefix.h"

#include "../OgreCompositorPassDef.h"
#include "OgreCommon.h"
#include "OgreColourValue.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    class _OgreExport CompositorPassMipmapDef : public CompositorPassDef
    {
    public:
        enum MipmapGenerationMethods
        {
            /// Use the default based on the API selected. On DX12 that means using Compute.
            ApiDefault,
            /// Tell the API's to use high quality. Not recommended
            /// ("quality" varies a lot across vendors).
            //ApiDefaultHQ,
            /** Use a compute shader. Ogre must be compiled with JSON and the
                Compute shaders bundled at Samples/Media/2.0/scripts/materials/Common
                must be in the resource path.
            @remarks
                Will use ApiDefault if Compute Shaders are not supported.
                At the time of writing (2017/01/16) Compute uses ComputeHQ
            */
            Compute,
            /// See Compute, but use a high quality gaussian filter.
            ComputeHQ
        };

        MipmapGenerationMethods mMipmapGenerationMethod;

        /// Used when mMipmapGenerationMethod == ComputeHQ
        float mGaussianDeviationFactor;
        /// Used when mMipmapGenerationMethod == ComputeHQ
        uint8 mKernelRadius;

    public:
        CompositorPassMipmapDef( CompositorTargetDef *parentTargetDef ) :
            CompositorPassDef( PASS_MIPMAP, parentTargetDef ),
            mMipmapGenerationMethod( ApiDefault ),
            mGaussianDeviationFactor( 0.5f ),
            mKernelRadius( 8 )
        {
        }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
