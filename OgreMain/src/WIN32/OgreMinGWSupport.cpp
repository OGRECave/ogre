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

#include <stdint.h>

// Define some symbols referred to by MSVC libs. Not having these symbols
// may cause linking errors when linking against the DX SDK.

extern "C" {
    // MSVC libs use _chkstk for stack-probing. MinGW equivalent is _alloca.
    // But calling it to ensure Ogre build with D3D9 static does cause a crash.
    // Might have to investige further here, later.
    //void _alloca();
    void _chkstk()
    {
        //_alloca();
    }
    
    // MSVC uses security cookies to prevent some buffer overflow attacks.
    // provide dummy implementations.
    intptr_t __security_cookie;
    
    void _fastcall __security_check_cookie(intptr_t i)
    {
    }
}
