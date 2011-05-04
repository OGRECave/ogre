#ifndef __StdHeaders_H__
#define __StdHeaders_H__

#ifdef __BORLANDC__
    #define __STD_ALGORITHM
#endif

#if defined ( OGRE_GCC_VISIBILITY ) && (OGRE_PLATFORM != OGRE_PLATFORM_APPLE && OGRE_PLATFORM != OGRE_PLATFORM_IPHONE)
/* Until libstdc++ for gcc 4.2 is released, we have to declare all
 * symbols in libstdc++.so externally visible, otherwise we end up
 * with them marked as hidden by -fvisible=hidden.
 *
 * See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=20218
 *
 * Due to a more strict linker included with Xcode 4, this is disabled on Mac OS X and iOS.
 * The reason? It changes the visibility of Boost functions.  The mismatch between visibility Boost when used in Ogre (default)
 * and Boost when compiled (hidden) results in mysterious link errors such as "Bad codegen, pointer diff".
 */
#   pragma GCC visibility push(default)
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <cmath>

// STL containers
#include <vector>
#include <map>
#include <string>
#include <set>
#include <list>
#include <deque>
#include <queue>
#include <bitset>

// Note - not in the original STL, but exists in SGI STL and STLport
// For gcc 4.3 see http://gcc.gnu.org/gcc-4.3/changes.html
#if (OGRE_COMPILER == OGRE_COMPILER_GNUC) && !defined(STLPORT)
#   if OGRE_COMP_VER >= 430
#       include <tr1/unordered_map>
#       include <tr1/unordered_set> 
#   else
#       include <ext/hash_map>
#       include <ext/hash_set>
#   endif
#else
#   if (OGRE_COMPILER == OGRE_COMPILER_MSVC) && !defined(STLPORT) && OGRE_COMP_VER >= 1600 // VC++ 10.0
#    	include <unordered_map>
#    	include <unordered_set>
#	else
#   	include <hash_set>
#   	include <hash_map>
#	endif
#endif 

// STL algorithms & functions
#include <algorithm>
#include <functional>
#include <limits>

// C++ Stream stuff
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef __BORLANDC__
namespace Ogre
{
    using namespace std;
}
#endif

extern "C" {

#   include <sys/types.h>
#   include <sys/stat.h>

}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#  undef min
#  undef max
#  if defined( __MINGW32__ )
#    include <unistd.h>
#  endif
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
extern "C" {

#   include <unistd.h>
#   include <dlfcn.h>

}
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
extern "C" {
#   include <unistd.h>
#   include <sys/param.h>
#   include <CoreFoundation/CoreFoundation.h>
}
#endif

#if OGRE_THREAD_SUPPORT
#	if !defined(NOMINMAX) && defined(_MSC_VER)
#		define NOMINMAX // required to stop windows.h messing up std::min
#	endif
#   include "Threading/OgreThreadHeaders.h"
#endif

#if defined ( OGRE_GCC_VISIBILITY ) && (OGRE_PLATFORM != OGRE_PLATFORM_APPLE && OGRE_PLATFORM != OGRE_PLATFORM_IPHONE)
#   pragma GCC visibility pop
#endif
#endif
