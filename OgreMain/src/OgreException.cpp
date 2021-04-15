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

#ifdef __BORLANDC__
    #include <stdio.h>
#endif

namespace Ogre {

    Exception::Exception(int num, const String& desc, const String& src) :
        Exception(num, desc, src, "", "", 0)
    {
    }

    Exception::Exception(int num, const String& desc, const String& src, 
        const char* typ, const char* fil, long lin) :
        line( lin ),
        typeName(typ),
        description( desc ),
        source( src ),
        file( fil )
    {
        StringStream ss;

        ss << typeName << ": "
           << description
           << " in " << source;

        if( line > 0 )
        {
            ss << " at " << file << " (line " << line << ")";
        }

        fullDesc = ss.str();
    }

    Exception::Exception(const Exception& rhs)
        : line( rhs.line ), 
        typeName( rhs.typeName ), 
        description( rhs.description ), 
        source( rhs.source ), 
        file( rhs.file )
    {
    }
}

