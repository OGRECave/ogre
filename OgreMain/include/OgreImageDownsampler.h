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
#ifndef _OgreImageDownsampler_H_
#define _OgreImageDownsampler_H_

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Image
    *  @{
    */
    /** Range is [kernelStart; kernelEnd]
    @param dstPtr
    @param srcPtr
    @param width
    @param height
    @param kernel
    @param kernelStartX
    @param kernelEndX
    @param kernelStartY
    @param kernelEndY
     */
    typedef void (ImageDownsampler2D)( uint8 *dstPtr, uint8 const *srcPtr,
                                       int32 dstWidth, int32 dstHeight,
                                       int32 srcWidth,
                                       const uint8 kernel[5][5],
                                       const int8 kernelStartX, const int8 kernelEndX,
                                       const int8 kernelStartY, const int8 kernelEndY );

    ImageDownsampler2D downscale2x_XXXA8888;
    ImageDownsampler2D downscale2x_XXX888;
    ImageDownsampler2D downscale2x_XX88;
    ImageDownsampler2D downscale2x_X8;
    ImageDownsampler2D downscale2x_A8;
    ImageDownsampler2D downscale2x_XA88;

    //
    //  CUBEMAP versions
    //

    typedef void (ImageDownsamplerCube)( uint8 *dstPtr, uint8 const **srcPtr,
                                       int32 dstWidth, int32 dstHeight,
                                       int32 srcWidth, int32 srcHeight,
                                       const uint8 kernel[5][5],
                                       const int8 kernelStartX, const int8 kernelEndX,
                                       const int8 kernelStartY, const int8 kernelEndY,
                                       uint8 currentFace );

    ImageDownsamplerCube downscale2x_XXXA8888_cube;
    ImageDownsamplerCube downscale2x_XXX888_cube;
    ImageDownsamplerCube downscale2x_XX88_cube;
    ImageDownsamplerCube downscale2x_X8_cube;
    ImageDownsamplerCube downscale2x_A8_cube;
    ImageDownsamplerCube downscale2x_XA88_cube;

    /** Range is [kernelStart; kernelEnd]
    @param _tmpPtr
        Temporary buffer. Must be able to hold a copy of _srcDstPtr
    @param _srcDstPtr
        Input image to blur, output image blurred
    @param width
    @param height
    @param kernel
    @param kernelStart
    @param kernelEnd
     */
    typedef void (ImageBlur2D)( uint8 *_tmpPtr, uint8 *_srcDstPtr,
                                int32 width, int32 height,
                                const uint8 kernel[5],
                                const int8 kernelStart, const int8 kernelEnd );

    ImageBlur2D separableBlur_XXXA8888;
    ImageBlur2D separableBlur_XXX888;
    ImageBlur2D separableBlur_XX88;
    ImageBlur2D separableBlur_X8;
    ImageBlur2D separableBlur_A8;
    ImageBlur2D separableBlur_XA88;

    //-----------------------------------------------------------------------------------
    //Signed versions
    //-----------------------------------------------------------------------------------


    ImageDownsampler2D downscale2x_Signed_XXXA8888;
    ImageDownsampler2D downscale2x_Signed_XXX888;
    ImageDownsampler2D downscale2x_Signed_XX88;
    ImageDownsampler2D downscale2x_Signed_X8;
    ImageDownsampler2D downscale2x_Signed_A8;
    ImageDownsampler2D downscale2x_Signed_XA88;

    //
    //  CUBEMAP Signed versions
    //

    ImageDownsamplerCube downscale2x_Signed_XXXA8888_cube;
    ImageDownsamplerCube downscale2x_Signed_XXX888_cube;
    ImageDownsamplerCube downscale2x_Signed_XX88_cube;
    ImageDownsamplerCube downscale2x_Signed_X8_cube;
    ImageDownsamplerCube downscale2x_Signed_A8_cube;
    ImageDownsamplerCube downscale2x_Signed_XA88_cube;

    //
    //  Blur Signed versions
    //

    ImageBlur2D separableBlur_Signed_XXXA8888;
    ImageBlur2D separableBlur_Signed_XXX888;
    ImageBlur2D separableBlur_Signed_XX88;
    ImageBlur2D separableBlur_Signed_X8;
    ImageBlur2D separableBlur_Signed_A8;
    ImageBlur2D separableBlur_Signed_XA88;

    //-----------------------------------------------------------------------------------
    //Float32 versions
    //-----------------------------------------------------------------------------------


    ImageDownsampler2D downscale2x_Float32_XXXA;
    ImageDownsampler2D downscale2x_Float32_XXX;
    ImageDownsampler2D downscale2x_Float32_XX;
    ImageDownsampler2D downscale2x_Float32_X;
    ImageDownsampler2D downscale2x_Float32_A;
    ImageDownsampler2D downscale2x_Float32_XA;

    //
    //  CUBEMAP Float32 versions
    //

    ImageDownsamplerCube downscale2x_Float32_XXXA_cube;
    ImageDownsamplerCube downscale2x_Float32_XXX_cube;
    ImageDownsamplerCube downscale2x_Float32_XX_cube;
    ImageDownsamplerCube downscale2x_Float32_X_cube;
    ImageDownsamplerCube downscale2x_Float32_A_cube;
    ImageDownsamplerCube downscale2x_Float32_XA_cube;

    //
    //  Blur Float32 versions
    //

    ImageBlur2D separableBlur_Float32_XXXA;
    ImageBlur2D separableBlur_Float32_XXX;
    ImageBlur2D separableBlur_Float32_XX;
    ImageBlur2D separableBlur_Float32_X;
    ImageBlur2D separableBlur_Float32_A;
    ImageBlur2D separableBlur_Float32_XA;

    //-----------------------------------------------------------------------------------
    //sRGB versions
    //-----------------------------------------------------------------------------------


    ImageDownsampler2D downscale2x_sRGB_XXXA8888;
    ImageDownsampler2D downscale2x_sRGB_AXXX8888;
    ImageDownsampler2D downscale2x_sRGB_XXX888;
    ImageDownsampler2D downscale2x_sRGB_XX88;
    ImageDownsampler2D downscale2x_sRGB_X8;
    ImageDownsampler2D downscale2x_sRGB_A8;
    ImageDownsampler2D downscale2x_sRGB_XA88;
    ImageDownsampler2D downscale2x_sRGB_AX88;

    //
    //  CUBEMAP sRGB versions
    //

    ImageDownsamplerCube downscale2x_sRGB_XXXA8888_cube;
    ImageDownsamplerCube downscale2x_sRGB_AXXX8888_cube;
    ImageDownsamplerCube downscale2x_sRGB_XXX888_cube;
    ImageDownsamplerCube downscale2x_sRGB_XX88_cube;
    ImageDownsamplerCube downscale2x_sRGB_X8_cube;
    ImageDownsamplerCube downscale2x_sRGB_A8_cube;
    ImageDownsamplerCube downscale2x_sRGB_XA88_cube;
    ImageDownsamplerCube downscale2x_sRGB_AX88_cube;

    //
    //  Blur sRGB versions
    //

    ImageBlur2D separableBlur_sRGB_XXXA8888;
    ImageBlur2D separableBlur_sRGB_AXXX8888;
    ImageBlur2D separableBlur_sRGB_XXX888;
    ImageBlur2D separableBlur_sRGB_XX88;
    ImageBlur2D separableBlur_sRGB_X8;
    ImageBlur2D separableBlur_sRGB_A8;
    ImageBlur2D separableBlur_sRGB_XA88;
    ImageBlur2D separableBlur_sRGB_AX88;

    struct FilterKernel
    {
        uint8   kernel[5][5];
        int8    kernelStartX;
        int8    kernelEndX;
        int8    kernelStartY;
        int8    kernelEndY;
    };
    struct FilterSeparableKernel
    {
        uint8   kernel[5];
        int8    kernelStart;
        int8    kernelEnd;
    };

    extern const FilterKernel c_filterKernels[3];
    extern const FilterSeparableKernel c_filterSeparableKernels[1];

    /** @} */
    /** @} */
}

#endif
