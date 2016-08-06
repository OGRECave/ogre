//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros
#include "OgrePlatform.h"

namespace Ogre
{
    void _OgreExport MurmurHash3_x86_32  ( const void * key, int len, uint32 seed, void * out );

    void _OgreExport MurmurHash3_x86_128 ( const void * key, int len, uint32 seed, void * out );

    void _OgreExport MurmurHash3_x64_128 ( const void * key, int len, uint32 seed, void * out );
}

//-----------------------------------------------------------------------------

#endif // _MURMURHASH3_H_
