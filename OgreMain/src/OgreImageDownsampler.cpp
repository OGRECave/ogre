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

#ifndef ITERATING

#include "OgreVector3.h"
#include "OgreMatrix3.h"

#include "OgreImageDownsampler.h"

namespace Ogre
{
    struct CubemapUVI
    {
        Real    u;
        Real    v;
        int     face;
    };

    enum MajorAxis
    {
        MajorAxisX,
        MajorAxisY,
        MajorAxisZ,
    };

    struct FaceComponents
    {
        uint8   uIdx;
        uint8   vIdx;
        Real    uSign;
        Real    vSign;

        FaceComponents( uint8 _uIdx, uint8 _vIdx, Real _uSign, Real _vSign ) :
            uIdx( _uIdx ), vIdx( _vIdx ), uSign( _uSign ), vSign( _vSign )
        {
        }
    };

    const FaceComponents c_faceComponents[6] =
    {
        FaceComponents( 2, 1, -0.5f, -0.5f ),
        FaceComponents( 2, 1,  0.5f, -0.5f ),
        FaceComponents( 0, 2,  0.5f,  0.5f ),
        FaceComponents( 0, 2,  0.5f, -0.5f ),
        FaceComponents( 0, 1,  0.5f, -0.5f ),
        FaceComponents( 0, 1, -0.5f, -0.5f )
    };

    struct FaceSwizzle
    {
        int8    iX, iY, iZ;
        Real    signX, signY, signZ;
        FaceSwizzle( int8 _iX, int8 _iY, int8 _iZ, Real _signX, Real _signY, Real _signZ ) :
            iX( _iX ), iY( _iY ), iZ( _iZ ), signX( _signX ), signY( _signY ), signZ( _signZ ) {}
    };

    static const FaceSwizzle c_faceSwizzles[6] =
    {
        FaceSwizzle( 2, 1, 0,  1,  1, -1 ),
        FaceSwizzle( 2, 1, 0, -1,  1,  1 ),
        FaceSwizzle( 0, 2, 1,  1,  1, -1 ),
        FaceSwizzle( 0, 2, 1,  1, -1,  1 ),
        FaceSwizzle( 0, 1, 2,  1,  1,  1 ),
        FaceSwizzle( 0, 1, 2, -1,  1, -1 ),
    };

    inline CubemapUVI cubeMapProject( Vector3 vDir )
    {
        CubemapUVI uvi;

        Vector3 absDir = Vector3( fabsf( vDir.x ), fabsf( vDir.y ), fabsf( vDir.z ) );

        bool majorX = (absDir.x >= absDir.y) & (absDir.x >= absDir.z);
        bool majorY = (absDir.y >= absDir.x) & (absDir.y >= absDir.z);
        bool majorZ = (absDir.z >= absDir.x) & (absDir.z >= absDir.y);

        Real fNorm;
        MajorAxis majorAxis;
        majorAxis   = majorX ? MajorAxisX : MajorAxisY;
        fNorm       = majorX ? absDir.x : absDir.y;

        majorAxis   = majorY ? MajorAxisY : majorAxis;
        fNorm       = majorY ? absDir.y : fNorm;

        majorAxis   = majorZ ? MajorAxisZ : majorAxis;
        fNorm       = majorZ ? absDir.z : fNorm;

        const Real *fDirs = vDir.ptr();

        uvi.face = fDirs[majorAxis] >= 0 ? majorAxis * 2 : majorAxis * 2 + 1;

        fNorm = 1.0f / fNorm;

        uvi.u =  c_faceComponents[uvi.face].uSign *
                    fDirs[c_faceComponents[uvi.face].uIdx] * fNorm + 0.5f;
        uvi.v =  c_faceComponents[uvi.face].vSign *
                    fDirs[c_faceComponents[uvi.face].vIdx] * fNorm + 0.5f;

        return uvi;
    }

    const FilterKernel c_filterKernels[3] =
    {
        {
            //Point
            {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            },
            0, 0,
            0, 0
        },
        {
            //Linear
            {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 1, 1, 0,
                0, 0, 1, 1, 0,
                0, 0, 0, 0, 0
            },
            0, 1,
            0, 1
        },
        {
            //Gaussian
            {
                1,  4,  7,  4, 1,
                4, 16, 26, 16, 4,
                7, 26, 41, 26, 7,
                4, 16, 26, 16, 4,
                1,  4,  7,  4, 1
            },
            -2, 2,
            -2, 2
        }
    };
}

    #define OGRE_GAM_TO_LIN( x ) x
    #define OGRE_LIN_TO_GAM( x ) x
    #define OGRE_UINT8 uint8
    #define OGRE_UINT32 uint32

    #define ITERATING
    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_DOWNSAMPLE_B 2
    #define OGRE_DOWNSAMPLE_A 3
    #define OGRE_TOTAL_SIZE 4
    #define DOWNSAMPLE_NAME downscale2x_XXXA8888
    #define DOWNSAMPLE_CUBE_NAME downscale2x_XXXA8888_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_DOWNSAMPLE_B 2
    #define OGRE_TOTAL_SIZE 3
    #define DOWNSAMPLE_NAME downscale2x_XXX888
	#define DOWNSAMPLE_CUBE_NAME downscale2x_XXX888_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_TOTAL_SIZE 2
    #define DOWNSAMPLE_NAME downscale2x_XX88
	#define DOWNSAMPLE_CUBE_NAME downscale2x_XX88_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_TOTAL_SIZE 1
    #define DOWNSAMPLE_NAME downscale2x_X8
	#define DOWNSAMPLE_CUBE_NAME downscale2x_X8_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_A 0
    #define OGRE_TOTAL_SIZE 1
    #define DOWNSAMPLE_NAME downscale2x_A8
	#define DOWNSAMPLE_CUBE_NAME downscale2x_A8_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_A 1
    #define OGRE_TOTAL_SIZE 2
    #define DOWNSAMPLE_NAME downscale2x_XA88
	#define DOWNSAMPLE_CUBE_NAME downscale2x_XA88_cube
    #include "OgreImageDownsampler.cpp"

    //-----------------------------------------------------------------------------------
    //Signed versions
    //-----------------------------------------------------------------------------------

    #undef OGRE_UINT8
    #undef OGRE_UINT32
    #define OGRE_UINT8 int8
    #define OGRE_UINT32 int32

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_DOWNSAMPLE_B 2
    #define OGRE_DOWNSAMPLE_A 3
    #define OGRE_TOTAL_SIZE 4
    #define DOWNSAMPLE_NAME downscale2x_Signed_XXXA8888
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Signed_XXXA8888_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_DOWNSAMPLE_B 2
    #define OGRE_TOTAL_SIZE 3
    #define DOWNSAMPLE_NAME downscale2x_Signed_XXX888
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Signed_XXX888_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_TOTAL_SIZE 2
    #define DOWNSAMPLE_NAME downscale2x_Signed_XX88
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Signed_XX88_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_TOTAL_SIZE 1
    #define DOWNSAMPLE_NAME downscale2x_Signed_X8
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Signed_X8_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_A 0
    #define OGRE_TOTAL_SIZE 1
    #define DOWNSAMPLE_NAME downscale2x_Signed_A8
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Signed_A8_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_A 1
    #define OGRE_TOTAL_SIZE 2
    #define DOWNSAMPLE_NAME downscale2x_Signed_XA88
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Signed_XA88_cube
    #include "OgreImageDownsampler.cpp"

    //-----------------------------------------------------------------------------------
    //Float32 versions
    //-----------------------------------------------------------------------------------

    #undef OGRE_UINT8
    #undef OGRE_UINT32
    #define OGRE_UINT8 float
    #define OGRE_UINT32 float

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_DOWNSAMPLE_B 2
    #define OGRE_DOWNSAMPLE_A 3
    #define OGRE_TOTAL_SIZE 4
    #define DOWNSAMPLE_NAME downscale2x_Float32_XXXA
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Float32_XXXA_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_DOWNSAMPLE_B 2
    #define OGRE_TOTAL_SIZE 3
    #define DOWNSAMPLE_NAME downscale2x_Float32_XXX
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Float32_XXX_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_TOTAL_SIZE 2
    #define DOWNSAMPLE_NAME downscale2x_Float32_XX
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Float32_XX_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_TOTAL_SIZE 1
    #define DOWNSAMPLE_NAME downscale2x_Float32_X
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Float32_X_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_A 0
    #define OGRE_TOTAL_SIZE 1
    #define DOWNSAMPLE_NAME downscale2x_Float32_A
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Float32_A_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_A 1
    #define OGRE_TOTAL_SIZE 2
    #define DOWNSAMPLE_NAME downscale2x_Float32_XA
    #define DOWNSAMPLE_CUBE_NAME downscale2x_Float32_XA_cube
    #include "OgreImageDownsampler.cpp"

    //-----------------------------------------------------------------------------------
    //sRGB versions
    //-----------------------------------------------------------------------------------

    #undef OGRE_GAM_TO_LIN
    #undef OGRE_LIN_TO_GAM
    #define OGRE_GAM_TO_LIN( x ) x * x
    #define OGRE_LIN_TO_GAM( x ) sqrtf( x )

    #undef OGRE_UINT8
    #undef OGRE_UINT32
    #define OGRE_UINT8 uint8
    #define OGRE_UINT32 uint32

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_DOWNSAMPLE_B 2
    #define OGRE_DOWNSAMPLE_A 3
    #define OGRE_TOTAL_SIZE 4
    #define DOWNSAMPLE_NAME downscale2x_sRGB_XXXA8888
    #define DOWNSAMPLE_CUBE_NAME downscale2x_sRGB_XXXA8888_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_A 0
    #define OGRE_DOWNSAMPLE_R 1
    #define OGRE_DOWNSAMPLE_G 2
    #define OGRE_DOWNSAMPLE_B 3
    #define OGRE_TOTAL_SIZE 4
    #define DOWNSAMPLE_NAME downscale2x_sRGB_AXXX8888
    #define DOWNSAMPLE_CUBE_NAME downscale2x_sRGB_AXXX8888_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_DOWNSAMPLE_B 2
    #define OGRE_TOTAL_SIZE 3
    #define DOWNSAMPLE_NAME downscale2x_sRGB_XXX888
    #define DOWNSAMPLE_CUBE_NAME downscale2x_sRGB_XXX888_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_G 1
    #define OGRE_TOTAL_SIZE 2
    #define DOWNSAMPLE_NAME downscale2x_sRGB_XX88
    #define DOWNSAMPLE_CUBE_NAME downscale2x_sRGB_XX88_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_TOTAL_SIZE 1
    #define DOWNSAMPLE_NAME downscale2x_sRGB_X8
    #define DOWNSAMPLE_CUBE_NAME downscale2x_sRGB_X8_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_A 0
    #define OGRE_TOTAL_SIZE 1
    #define DOWNSAMPLE_NAME downscale2x_sRGB_A8
    #define DOWNSAMPLE_CUBE_NAME downscale2x_sRGB_A8_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_R 0
    #define OGRE_DOWNSAMPLE_A 1
    #define OGRE_TOTAL_SIZE 2
    #define DOWNSAMPLE_NAME downscale2x_sRGB_XA88
    #define DOWNSAMPLE_CUBE_NAME downscale2x_sRGB_XA88_cube
    #include "OgreImageDownsampler.cpp"

    #define OGRE_DOWNSAMPLE_A 0
    #define OGRE_DOWNSAMPLE_R 1
    #define OGRE_TOTAL_SIZE 2
    #define DOWNSAMPLE_NAME downscale2x_sRGB_AX88
    #define DOWNSAMPLE_CUBE_NAME downscale2x_sRGB_AX88_cube
    #include "OgreImageDownsampler.cpp"

    #undef OGRE_GAM_TO_LIN
    #undef OGRE_LIN_TO_GAM
#else

namespace Ogre
{
    void DOWNSAMPLE_NAME( uint8 *_dstPtr, uint8 const *_srcPtr,
                          int32 dstWidth, int32 dstHeight,
                          int32 srcWidth,
                          const uint8 kernel[5][5],
                          const int8 kernelStartX, const int8 kernelEndX,
                          const int8 kernelStartY, const int8 kernelEndY )
    {
        OGRE_UINT8 *dstPtr = reinterpret_cast<OGRE_UINT8*>( _dstPtr );
        OGRE_UINT8 const *srcPtr = reinterpret_cast<OGRE_UINT8 const *>( _srcPtr );

        for( int32 y=0; y<dstHeight; ++y )
        {
            for( int32 x=0; x<dstWidth; ++x )
            {
                int kStartY = std::max<int>( -y, kernelStartY );
                int kEndY   = std::min<int>( dstHeight - y, kernelEndY );

    #ifdef OGRE_DOWNSAMPLE_R
                OGRE_UINT32 accumR = 0;
    #endif
    #ifdef OGRE_DOWNSAMPLE_G
                OGRE_UINT32 accumG = 0;
    #endif
    #ifdef OGRE_DOWNSAMPLE_B
                OGRE_UINT32 accumB = 0;
    #endif
    #ifdef OGRE_DOWNSAMPLE_A
                OGRE_UINT32 accumA = 0;
    #endif

                uint32 divisor = 0;

                for( int k_y=kStartY; k_y<=kEndY; ++k_y )
                {
                    int kStartX = std::max<int>( -x, kernelStartX );
                    int kEndX   = std::min<int>( dstWidth - 1 - x, kernelEndX );

                    for( int k_x=kStartX; k_x<=kEndX; ++k_x )
                    {
                        uint32 kernelVal = kernel[k_y+2][k_x+2];

    #ifdef OGRE_DOWNSAMPLE_R
                        OGRE_UINT32 r = srcPtr[(k_y * srcWidth + k_x) * OGRE_TOTAL_SIZE + OGRE_DOWNSAMPLE_R];
                        accumR += OGRE_GAM_TO_LIN( r ) * kernelVal;
    #endif
    #ifdef OGRE_DOWNSAMPLE_G
                        OGRE_UINT32 g = srcPtr[(k_y * srcWidth + k_x) * OGRE_TOTAL_SIZE + OGRE_DOWNSAMPLE_G];
                        accumG += OGRE_GAM_TO_LIN( g ) * kernelVal;
    #endif
    #ifdef OGRE_DOWNSAMPLE_B
                        OGRE_UINT32 b = srcPtr[(k_y * srcWidth + k_x) * OGRE_TOTAL_SIZE + OGRE_DOWNSAMPLE_B];
                        accumB += OGRE_GAM_TO_LIN( b ) * kernelVal;
    #endif
    #ifdef OGRE_DOWNSAMPLE_A
                        OGRE_UINT32 a = srcPtr[(k_y * srcWidth + k_x) * OGRE_TOTAL_SIZE + OGRE_DOWNSAMPLE_A];
                        accumA += a * kernelVal;
    #endif

                        divisor += kernelVal;
                    }
                }

    #if defined( OGRE_DOWNSAMPLE_R ) || defined( OGRE_DOWNSAMPLE_G ) || defined( OGRE_DOWNSAMPLE_B )
                float invDivisor = 1.0f / divisor;
    #endif

    #ifdef OGRE_DOWNSAMPLE_R
                dstPtr[OGRE_DOWNSAMPLE_R] = static_cast<OGRE_UINT8>( OGRE_LIN_TO_GAM( accumR * invDivisor ) + 0.5f );
    #endif
    #ifdef OGRE_DOWNSAMPLE_G
                dstPtr[OGRE_DOWNSAMPLE_G] = static_cast<OGRE_UINT8>( OGRE_LIN_TO_GAM( accumG * invDivisor ) + 0.5f );
    #endif
    #ifdef OGRE_DOWNSAMPLE_B
                dstPtr[OGRE_DOWNSAMPLE_B] = static_cast<OGRE_UINT8>( OGRE_LIN_TO_GAM( accumB * invDivisor ) + 0.5f );
    #endif
    #ifdef OGRE_DOWNSAMPLE_A
                dstPtr[OGRE_DOWNSAMPLE_A] = static_cast<OGRE_UINT8>( (accumA + divisor - 1u) / divisor );
    #endif

                dstPtr += OGRE_TOTAL_SIZE;
                srcPtr += OGRE_TOTAL_SIZE * 2;
            }

            srcPtr += (srcWidth - dstWidth * 2) * OGRE_TOTAL_SIZE;
            srcPtr += srcWidth * OGRE_TOTAL_SIZE;
        }
    }
    //-----------------------------------------------------------------------------------
    void DOWNSAMPLE_CUBE_NAME( uint8 *_dstPtr, uint8 const **_allPtr,
                               int32 dstWidth, int32 dstHeight,
                               int32 srcWidth, int32 srcHeight,
                               const uint8 kernel[5][5],
                               const int8 kernelStartX, const int8 kernelEndX,
                               const int8 kernelStartY, const int8 kernelEndY,
                               uint8 currentFace )
    {
        OGRE_UINT8 *dstPtr = reinterpret_cast<OGRE_UINT8*>( _dstPtr );
        OGRE_UINT8 const **allPtr = reinterpret_cast<OGRE_UINT8 const **>( _allPtr );

        Quaternion kRotations[5][5];

        {
            Radian xRadStep( Ogre::Math::PI / (srcWidth * 2.0f) );
            Radian yRadStep( Ogre::Math::PI / (srcHeight * 2.0f) );

            for( int y=kernelStartY; y<=kernelEndY; ++y )
            {
                for( int x=kernelStartX; x<=kernelEndX; ++x )
                {
                    Matrix3 m3;
                    m3.FromEulerAnglesXYZ( (Real)-y * yRadStep, (Real)x * xRadStep, Radian( 0 ) );
                    kRotations[y+2][x+2] = Quaternion( m3 );
                }
            }
        }

        const FaceSwizzle &faceSwizzle = c_faceSwizzles[currentFace];

        Real invSrcWidth  = 1.0f / srcWidth;
        Real invSrcHeight = 1.0f / srcHeight;

        OGRE_UINT8 const *srcPtr = 0;

        for( int32 y=0; y<dstHeight; ++y )
        {
            for( int32 x=0; x<dstWidth; ++x )
            {
    #ifdef OGRE_DOWNSAMPLE_R
                OGRE_UINT32 accumR = 0;
    #endif
    #ifdef OGRE_DOWNSAMPLE_G
                OGRE_UINT32 accumG = 0;
    #endif
    #ifdef OGRE_DOWNSAMPLE_B
                OGRE_UINT32 accumB = 0;
    #endif
    #ifdef OGRE_DOWNSAMPLE_A
                OGRE_UINT32 accumA = 0;
    #endif

                uint32 divisor = 0;

                Vector3 vForwardSample( (x * 2 + 0.5f) * invSrcWidth  *  2.0f - 1.0f,
                                        (y * 2 + 0.5f) * invSrcHeight * -2.0f + 1.0f,
                                        1.0f );

                for( int k_y=kernelStartY; k_y<=kernelEndY; ++k_y )
                {
                    for( int k_x=kernelStartX; k_x<=kernelEndX; ++k_x )
                    {
                        uint32 kernelVal = kernel[k_y+2][k_x+2];

                        Vector3 tmp = kRotations[k_y+2][k_x+2] * vForwardSample;

                        Vector3 vSample;
                        vSample.ptr()[0] = tmp.ptr()[faceSwizzle.iX] * faceSwizzle.signX;
                        vSample.ptr()[1] = tmp.ptr()[faceSwizzle.iY] * faceSwizzle.signY;
                        vSample.ptr()[2] = tmp.ptr()[faceSwizzle.iZ] * faceSwizzle.signZ;

                        CubemapUVI uvi = cubeMapProject( vSample );

                        int iu = std::min( static_cast<int>( floorf( uvi.u * srcWidth ) ),
                                           srcWidth - 1 );
                        int iv = std::min( static_cast<int>( floorf( uvi.v * srcHeight ) ),
                                           srcHeight - 1 );

                        srcPtr = allPtr[uvi.face] + (iv * srcWidth + iu) * OGRE_TOTAL_SIZE;

    #ifdef OGRE_DOWNSAMPLE_R
                        OGRE_UINT32 r = srcPtr[OGRE_DOWNSAMPLE_R];
                        accumR += OGRE_GAM_TO_LIN( r ) * kernelVal;
    #endif
    #ifdef OGRE_DOWNSAMPLE_G
                        OGRE_UINT32 g = srcPtr[OGRE_DOWNSAMPLE_G];
                        accumG += OGRE_GAM_TO_LIN( g ) * kernelVal;
    #endif
    #ifdef OGRE_DOWNSAMPLE_B
                        OGRE_UINT32 b = srcPtr[OGRE_DOWNSAMPLE_B];
                        accumB += OGRE_GAM_TO_LIN( b ) * kernelVal;
    #endif
    #ifdef OGRE_DOWNSAMPLE_A
                        OGRE_UINT32 a = srcPtr[OGRE_DOWNSAMPLE_A];
                        accumA += a * kernelVal;
    #endif

                        divisor += kernelVal;
                    }
                }

    #if defined( OGRE_DOWNSAMPLE_R ) || defined( OGRE_DOWNSAMPLE_G ) || defined( OGRE_DOWNSAMPLE_B )
                float invDivisor = 1.0f / divisor;
    #endif

    #ifdef OGRE_DOWNSAMPLE_R
                dstPtr[OGRE_DOWNSAMPLE_R] = static_cast<OGRE_UINT8>( OGRE_LIN_TO_GAM( accumR * invDivisor ) + 0.5f );
    #endif
    #ifdef OGRE_DOWNSAMPLE_G
                dstPtr[OGRE_DOWNSAMPLE_G] = static_cast<OGRE_UINT8>( OGRE_LIN_TO_GAM( accumG * invDivisor ) + 0.5f );
    #endif
    #ifdef OGRE_DOWNSAMPLE_B
                dstPtr[OGRE_DOWNSAMPLE_B] = static_cast<OGRE_UINT8>( OGRE_LIN_TO_GAM( accumB * invDivisor ) + 0.5f );
    #endif
    #ifdef OGRE_DOWNSAMPLE_A
                dstPtr[OGRE_DOWNSAMPLE_A] = static_cast<OGRE_UINT8>( (accumA + divisor - 1u) / divisor );
    #endif

                dstPtr += OGRE_TOTAL_SIZE;
            }
        }
    }
}

    #undef OGRE_DOWNSAMPLE_A
    #undef OGRE_DOWNSAMPLE_R
    #undef OGRE_DOWNSAMPLE_G
    #undef OGRE_DOWNSAMPLE_B
    #undef DOWNSAMPLE_NAME
    #undef DOWNSAMPLE_CUBE_NAME
    #undef OGRE_TOTAL_SIZE
#endif
