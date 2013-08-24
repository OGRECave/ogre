#ifndef __StdHeaders_H__
#define __StdHeaders_H__

#ifdef __BORLANDC__
    #define __STD_ALGORITHM
#endif

#if defined ( OGRE_GCC_VISIBILITY ) && ((OGRE_PLATFORM == OGRE_PLATFORM_APPLE && !__LP64__) && OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS)
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
#elif (OGRE_COMPILER == OGRE_COMPILER_CLANG)
#   if defined(_LIBCPP_VERSION)
#       include <unordered_map>
#       include <unordered_set>
#   else
#       include <tr1/unordered_map>
#       include <tr1/unordered_set>
#   endif
#elif !defined(STLPORT)
#   if (OGRE_COMPILER == OGRE_COMPILER_MSVC) && _MSC_FULL_VER >= 150030729 // VC++ 9.0 SP1+
#    	include <unordered_map>
#    	include <unordered_set>
#   elif OGRE_THREAD_PROVIDER == 1
#       include <boost/unordered_map.hpp>
#       include <boost/unordered_set.hpp>
#   else
#   	error "Your compiler doesn't support unordered_set and unordered_map. Try to compile Ogre with Boost or STLPort."
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

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#  undef min
#  undef max
#  if defined( __MINGW32__ )
#    include <unistd.h>
#  endif
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
extern "C" {

#   include <unistd.h>
#   include <dlfcn.h>

}
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
extern "C" {
#   include <unistd.h>
#   include <sys/param.h>
#   include <CoreFoundation/CoreFoundation.h>
}
#endif

#if defined ( OGRE_GCC_VISIBILITY ) && ((OGRE_PLATFORM == OGRE_PLATFORM_APPLE && !__LP64__) && OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS)
#   pragma GCC visibility pop
#endif
#endif
