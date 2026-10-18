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
#include "windowsUtils.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Ogre
{
    std::wstring windowsUtf8ToWide(const String& str)
    {
        std::wstring ret;
        const int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
        if (length > 0)
        {
            ret.resize(length);
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &ret[0], length);
        }
        return ret;
    }

    String windowsWideToUtf8(const wchar_t* wstr)
    {
        String ret;
        const int wlength = (int)wcslen(wstr);
        const int length = WideCharToMultiByte(CP_UTF8, 0, wstr, wlength, NULL, 0, NULL, NULL);
        if (length > 0)
        {
            ret.resize(length);
            WideCharToMultiByte(CP_UTF8, 0, wstr, wlength, &ret[0], length, NULL, NULL);
        }
        return ret;
    }

    String windowsTempFileName()
    {
        String ret;
#ifdef _OGRE_FILESYSTEM_ARCHIVE_UNICODE
        wchar_t* tmpname = _wtempnam(L".", L"ogre");
        if (!tmpname)
            return ret;

        ret = windowsWideToUtf8(tmpname);
#else
        char* tmpname = _tempnam(".", "ogre");
        if (!tmpname)
            return ret;

        ret = tmpname;
#endif
        free(tmpname);
        return ret;
    }
}
