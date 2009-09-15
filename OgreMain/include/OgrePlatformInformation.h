/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __PlatformInformation_H__
#define __PlatformInformation_H__

#include "OgrePrerequisites.h"

namespace Ogre {
//
// TODO: Puts following macros into OgrePlatform.h?
//

/* Initial CPU stuff to set.
*/
#define OGRE_CPU_UNKNOWN    0
#define OGRE_CPU_X86        1
#define OGRE_CPU_PPC        2
#define OGRE_CPU_ARM        3

/* Find CPU type
*/
#if (defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))) || \
    (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
#   define OGRE_CPU OGRE_CPU_X86

#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE && defined(__BIG_ENDIAN__)
#   define OGRE_CPU OGRE_CPU_PPC
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#	define OGRE_CPU OGRE_CPU_X86
#elif OGRE_PLATFORM == OGRE_PLATFORM_IPHONE && (defined(__i386__) || defined(__x86_64__))
#	define OGRE_CPU OGRE_CPU_X86
#elif defined(__arm__)
#	define OGRE_CPU OGRE_CPU_ARM
#else
#   define OGRE_CPU OGRE_CPU_UNKNOWN
#endif

/* Find how to declare aligned variable.
*/
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#   define OGRE_ALIGNED_DECL(type, var, alignment)  __declspec(align(alignment)) type var

#elif OGRE_COMPILER == OGRE_COMPILER_GNUC
#   define OGRE_ALIGNED_DECL(type, var, alignment)  type var __attribute__((__aligned__(alignment)))

#else
#   define OGRE_ALIGNED_DECL(type, var, alignment)  type var
#endif

/** Find perfect alignment (should supports SIMD alignment if SIMD available)
*/
#if OGRE_CPU == OGRE_CPU_X86
#   define OGRE_SIMD_ALIGNMENT  16

#else
#   define OGRE_SIMD_ALIGNMENT  16
#endif

/* Declare variable aligned to SIMD alignment.
*/
#define OGRE_SIMD_ALIGNED_DECL(type, var)   OGRE_ALIGNED_DECL(type, var, OGRE_SIMD_ALIGNMENT)

/* Define whether or not Ogre compiled with SSE supports.
*/
#if OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC
#   define __OGRE_HAVE_SSE  1
#elif OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_GNUC
#   define __OGRE_HAVE_SSE  1
#endif

/* Define whether or not Ogre compiled with VFP supports.
 */
#if OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_ARM && OGRE_COMPILER == OGRE_COMPILER_GNUC && defined(__ARM_ARCH_6K__) && defined(__VFP_FP__)
#   define __OGRE_HAVE_VFP  1
#endif

/* Define whether or not Ogre compiled with NEON supports.
 */
#if OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_ARM && OGRE_COMPILER == OGRE_COMPILER_GNUC && defined(__ARM_ARCH_7A__) && defined(__ARM_NEON__)
#   define __OGRE_HAVE_NEON  1
#endif

#ifndef __OGRE_HAVE_SSE
#   define __OGRE_HAVE_SSE  0
#endif

#ifndef __OGRE_HAVE_VFP
#   define __OGRE_HAVE_VFP  0
#endif

#ifndef __OGRE_HAVE_NEON
#   define __OGRE_HAVE_NEON  0
#endif
    
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/


    /** Class which provides the run-time platform information Ogre runs on.
        @remarks
            Ogre is designed to be platform-independent, but some platform
            and run-time environment specific optimised functions are built-in
            to maximise performance, and those special optimised routines are
            need to determine run-time environment for select variant executing
            path.
        @par
            This class manages that provides a couple of functions to determine
            platform information of the run-time environment.
        @note
            This class is supposed to use by advanced user only.
    */
    class _OgreExport PlatformInformation
    {
    public:

        /// Enum describing the different CPU features we want to check for, platform-dependent
        enum CpuFeatures
        {
#if OGRE_CPU == OGRE_CPU_X86
            CPU_FEATURE_SSE         = 1 << 0,
            CPU_FEATURE_SSE2        = 1 << 1,
            CPU_FEATURE_SSE3        = 1 << 2,
            CPU_FEATURE_MMX         = 1 << 3,
            CPU_FEATURE_MMXEXT      = 1 << 4,
            CPU_FEATURE_3DNOW       = 1 << 5,
            CPU_FEATURE_3DNOWEXT    = 1 << 6,
            CPU_FEATURE_CMOV        = 1 << 7,
            CPU_FEATURE_TSC         = 1 << 8,
            CPU_FEATURE_FPU         = 1 << 9,
            CPU_FEATURE_PRO         = 1 << 10,
            CPU_FEATURE_HTT         = 1 << 11,
#elif OGRE_CPU == OGRE_CPU_ARM
            CPU_FEATURE_VFP         = 1 << 12,
            CPU_FEATURE_NEON        = 1 << 13,
#endif

            CPU_FEATURE_NONE        = 0
        };

        /** Gets a string of the CPU identifier.
        @note
            Actual detecting are performs in the first time call to this function,
            and then all future calls with return internal cached value.
        */
        static const String& getCpuIdentifier(void);

        /** Gets a or-masked of enum CpuFeatures that are supported by the CPU.
        @note
            Actual detecting are performs in the first time call to this function,
            and then all future calls with return internal cached value.
        */
        static uint getCpuFeatures(void);

		/** Gets whether a specific feature is supported by the CPU.
		@note
			Actual detecting are performs in the first time call to this function,
			and then all future calls with return internal cached value.
		*/
		static bool hasCpuFeature(CpuFeatures feature);


		/** Write the CPU information to the passed in Log */
		static void log(Log* pLog);

    };
	/** @} */
	/** @} */

}

#endif  // __PlatformInformation_H__
