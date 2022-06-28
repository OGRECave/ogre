#ifndef __StdHeaders_H__
#define __StdHeaders_H__

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
#include <unordered_map>

// STL algorithms & functions
#include <algorithm>

// C++ Stream stuff
#include <ostream>
#include <iosfwd>

#include <atomic>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#  undef min
#  undef max
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
#   include <emscripten/emscripten.h>
#endif

#endif
