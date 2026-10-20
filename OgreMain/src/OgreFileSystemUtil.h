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
#ifndef __OgreFileSystemUtil_H__
#define __OgreFileSystemUtil_H__

#include "OgrePrerequisites.h"
#include <ctime>
#include <fstream>

// This header is internal to OgreMain. It is deliberately not installed and
// none of its functions are exported, so that the Windows path encoding
// conversion stays contained in OgreMain.
//
// On Windows, Ogre::String is expected to hold UTF-8 encoded paths (see
// OGRE_CONFIG_FILESYSTEM_UNICODE). The narrow CRT and std::fstream interfaces
// instead interpret their argument using the ANSI code page (CP_ACP), which
// mangles any path that is not representable in it. Everything in OgreMain
// that touches the filesystem must therefore go through this header (or
// through the public Root::openFileStream / Root::createFileStream /
// FileSystemLayer wrappers), never through the narrow APIs directly.

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && \
    OGRE_CONFIG_FILESYSTEM_UNICODE
#    define OGRE_FILESYSTEM_USE_WIDECHAR 1
#else
#    define OGRE_FILESYSTEM_USE_WIDECHAR 0
#endif

namespace Ogre {
namespace FileSystemUtil {

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    /** Convert a path to the wide representation used by the Windows
        filesystem APIs. If OGRE_CONFIG_FILESYSTEM_UNICODE is enabled the input
        is decoded as UTF-8, otherwise the ANSI code page is used (matching the
        behaviour of the narrow CRT).
    */
    std::wstring toWide(const String& path);

    /** Convert a wide path returned by the Windows filesystem APIs to the
        encoding used by Ogre::String.
    */
    String fromWide(const std::wstring& wide);
#endif

    /** Get the size of a file in bytes.
        @return 0 if the file does not exist or can not be accessed.
    */
    size_t getFileSize(const String& path);

    /** Get the time the given file was last modified.
        @return 0 if the file does not exist or the time can not be read.
    */
    time_t getFileModifiedTime(const String& path);

    /** Open a std::ofstream with a correctly decoded path. */
    inline void openOutputFile(std::ofstream& stream, const String& path,
                               std::ios::openmode mode = std::ios::out)
    {
#if OGRE_FILESYSTEM_USE_WIDECHAR
        stream.open(toWide(path).c_str(), mode);
#else
        stream.open(path.c_str(), mode);
#endif
    }

    /** Open a std::ifstream with a correctly decoded path. */
    inline void openInputFile(std::ifstream& stream, const String& path,
                              std::ios::openmode mode = std::ios::in)
    {
#if OGRE_FILESYSTEM_USE_WIDECHAR
        stream.open(toWide(path).c_str(), mode);
#else
        stream.open(path.c_str(), mode);
#endif
    }

    /** Open a std::fstream with a correctly decoded path. */
    inline void openInputOutputFile(std::fstream& stream, const String& path,
                                    std::ios::openmode mode)
    {
#if OGRE_FILESYSTEM_USE_WIDECHAR
        stream.open(toWide(path).c_str(), mode);
#else
        stream.open(path.c_str(), mode);
#endif
    }

} // namespace FileSystemUtil
} // namespace Ogre

#endif
