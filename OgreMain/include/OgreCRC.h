// the code is from here: http://create.stephan-brumme.com/crc32/
// //////////////////////////////////////////////////////////
// Crc32.h
// Copyright (c) 2011-2013 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

// g++ -o Crc32 Crc32.cpp -O3 -march=native -mtune=native

//changes from original code:
// * Added #include "OgreHeaderPrefix.h"
// * Added #include "OgrePlatform.h"
// * Added _OgreExport prefix to "uint32_t crc32_8bytes(const void*, size_t, uint32_t)" function
// * Changed the name of the file from crc.h to OgreCRC.h
// * Removed  #include <stdlib.h>
// * Removed typedef unsigned __int8  uint8_t;
// * Removed typedef unsigned __int32 uint32_t;
// * Added typdefs for uint32_t and uint8_t for old MS compilers
// * Added #include <stdint.h> for new compilers
// * Added namespace Ogre


#include "OgreHeaderPrefix.h"
#include "OgrePlatform.h"

namespace Ogre
{
#if OGRE_COMPILER == OGRE_COMPILER_MSVC && _MSC_VER <= 1500
    //Visual studio 2008 or earlier 
    typedef uint32 uint32_t;
    typedef uint8 uint8_t;

#else
#include <stdint.h>
#endif

#ifndef _crc_h
#define _crc_h

    typedef uint32_t crc;

    _OgreExport uint32_t crc32_8bytes(const void* data, size_t length, uint32_t previousCrc32 = 0);
}
#endif /* _crc_h */