//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros
#include "OgrePlatform.h"

#include <cstddef>

// Microsoft Visual Studio

#if defined( _MSC_VER ) && _MSC_VER < 1600

namespace Ogre
{
    typedef unsigned char uint8_t;
    typedef unsigned long uint32_t;
    typedef unsigned __int64 uint64_t;
}
// Other compilers

#else   // defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------

namespace Ogre
{
    void _OgreExport MurmurHash3_x86_32  ( const void * key, size_t len, uint32_t seed, void * out );

    void _OgreExport MurmurHash3_x86_128 ( const void * key, size_t len, uint32_t seed, void * out );

    void _OgreExport MurmurHash3_x64_128 ( const void * key, size_t len, uint32_t seed, void * out );

    inline void MurmurHash3_128( const void * key, size_t len, uint32_t seed, void * out ) {
#if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64
        MurmurHash3_x64_128(key, len, seed, out);
#else
        MurmurHash3_x86_128(key, len, seed, out);
#endif
    }
}

//-----------------------------------------------------------------------------

#endif // _MURMURHASH3_H_
