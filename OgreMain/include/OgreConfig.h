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
#ifndef __Config_H_
#define __Config_H_

// Include the CMake-generated build settings.
// If you get complaints that this file is missing, then you're probably
// trying to link directly against your source dir. You must then add
// %BUILD_DIR%/include to your include search path to find OgreBuildSettings.h.
#include "OgreBuildSettings.h"

/** If set to 1, profiling code will be included in the application. When you
	are deploying your application you will probably want to set this to 0 */
#ifndef OGRE_PROFILING
#define OGRE_PROFILING 0
#endif

/** There are three modes for handling asserts in OGRE:
0 - STANDARD - Standard asserts in debug builds, nothing in release builds
1 - RELEASE_EXCEPTIONS - Standard asserts in debug builds, exceptions in release builds
2 - EXCEPTIONS - Exceptions in debug builds, exceptions in release builds
*/
#ifndef OGRE_ASSERT_MODE
#define OGRE_ASSERT_MODE 0
#endif

/** If set to >0, OGRE will always 'think' that the graphics card only has the
    number of texture units specified. Very useful for testing multipass fallback.
*/
#define OGRE_PRETEND_TEXTURE_UNITS 0

/** If set to 1, Real is typedef'ed to double. Otherwise, Real is typedef'ed
    to float. Setting this allows you to perform mathematical operations in the
	CPU (Quaternion, Vector3 etc) with more precision, but bear in mind that the
	GPU still operates in single-precision mode.
*/
#ifndef OGRE_DOUBLE_PRECISION
#define OGRE_DOUBLE_PRECISION 0
#endif

/** Define number of texture coordinate sets allowed per vertex.
*/
#define OGRE_MAX_TEXTURE_COORD_SETS 8

/** Define max number of texture layers allowed per pass on any card.
*/
#define OGRE_MAX_TEXTURE_LAYERS 16

/** Define max number of lights allowed per pass.
*/
#define OGRE_MAX_SIMULTANEOUS_LIGHTS 8

/** Define max number of blending weights allowed per vertex.
*/
#define OGRE_MAX_BLEND_WEIGHTS 4

// define the memory allocator configuration to use
#define OGRE_MEMORY_ALLOCATOR_STD 1
#define OGRE_MEMORY_ALLOCATOR_NED 2
#define OGRE_MEMORY_ALLOCATOR_USER 3
#define OGRE_MEMORY_ALLOCATOR_NEDPOOLING 4

#ifndef OGRE_MEMORY_ALLOCATOR
#  define OGRE_MEMORY_ALLOCATOR OGRE_MEMORY_ALLOCATOR_NEDPOOLING
#endif

// Whether to use the custom memory allocator in STL containers
#ifndef OGRE_CONTAINERS_USE_CUSTOM_MEMORY_ALLOCATOR
#  define OGRE_CONTAINERS_USE_CUSTOM_MEMORY_ALLOCATOR 1
#endif

//if you want to make Ogre::String use the custom memory allocator then set:
//#define OGRE_STRING_USE_CUSTOM_MEMORY_ALLOCATOR 1
// Doing this will mean Ogre's strings will not be compatible with std::string however
#ifndef OGRE_STRING_USE_CUSTOM_MEMORY_ALLOCATOR
#	define OGRE_STRING_USE_CUSTOM_MEMORY_ALLOCATOR 0
#endif

// enable or disable the memory tracker, recording the memory allocations & tracking leaks
// default is to disable since it's expensive, but you can enable if needed per build target

#ifndef OGRE_MEMORY_TRACKER_DEBUG_MODE
#  define OGRE_MEMORY_TRACKER_DEBUG_MODE 0
#endif

#ifndef OGRE_MEMORY_TRACKER_RELEASE_MODE
#  define OGRE_MEMORY_TRACKER_RELEASE_MODE 0
#endif
/** Define max number of multiple render targets (MRTs) to render to at once.
*/
#define OGRE_MAX_MULTIPLE_RENDER_TARGETS 8

/** Support for multithreading, there are 3 options

OGRE_THREAD_SUPPORT = 0
	No support for threading.		
OGRE_THREAD_SUPPORT = 1
	Thread support for background loading, by both loading and constructing resources
	in a background thread. Resource management and SharedPtr handling becomes
	thread-safe, and resources may be completely loaded in the background. 
	The places where threading is available are clearly
	marked, you should assume state is NOT thread safe unless otherwise
	stated in relation to this flag.
OGRE_THREAD_SUPPORT = 2
	Thread support for background resource preparation. This means that resource
	data can streamed into memory in the background, but the final resource
	construction (including RenderSystem dependencies) is still done in the primary
	thread. Has a lower synchronisation primitive overhead than full threading
	while still allowing the major blocking aspects of resource management (I/O)
	to be done in the background.
*/
#ifndef OGRE_THREAD_SUPPORT
#define OGRE_THREAD_SUPPORT 0
#endif
#if OGRE_THREAD_SUPPORT != 0 && OGRE_THREAD_SUPPORT != 1 && OGRE_THREAD_SUPPORT != 2
#define OGRE_THREAD_SUPPORT 0
#endif

/** Provider for threading functionality, there are 4 options.

OGRE_THREAD_PROVIDER = 0
	No support for threading.
OGRE_THREAD_PROVIDER = 1
	Boost libraries provide threading functionality.
OGRE_THREAD_PROVIDER = 2
	Poco libraries provide threading functionality.
OGRE_THREAD_PROVIDER = 3
	TBB library provides threading functionality.
*/
#ifndef OGRE_THREAD_PROVIDER
#define OGRE_THREAD_PROVIDER 0
#endif

/** Disables use of the FreeImage image library for loading images.
WARNING: Use only when you want to provide your own image loading code via codecs.
*/
#ifndef OGRE_NO_FREEIMAGE
#define OGRE_NO_FREEIMAGE 0
#endif

/** Disables use of the internal image codec for loading DDS files.
WARNING: Use only when you want to provide your own image loading code via codecs.
*/
#ifndef OGRE_NO_DDS_CODEC
#define OGRE_NO_DDS_CODEC 0
#endif

/** Disables use of the internal image codec for loading ETC files.
 WARNING: Use only when you want to provide your own image loading code via codecs.
 */
#ifndef OGRE_NO_ETC_CODEC
#define OGRE_NO_ETC_CODEC 1
#endif

/** Disables use of the ZIP archive support.
WARNING: Disabling this will make the samples unusable.
*/
#ifndef OGRE_NO_ZIP_ARCHIVE
#define OGRE_NO_ZIP_ARCHIVE 0
#endif

#endif
