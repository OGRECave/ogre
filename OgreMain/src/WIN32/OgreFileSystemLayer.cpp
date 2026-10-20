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
#include "OgreFileSystemLayer.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#  include <shlobj.h>
#endif
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <cerrno>

#include "OgreFileSystemUtil.h"

namespace Ogre
{
    static String getModulePath(bool dllpath)
    {
        HMODULE hm = NULL;

        // get path of this DLL
        if (dllpath && !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                           (LPCWSTR)&getModulePath, &hm))
        {
            return "";
        }

        DWORD bufsize = 256;
        std::wstring resolved;
        do
        {
            std::wstring buf(bufsize, L'\0');
            DWORD retval = GetModuleFileNameW(hm, &buf[0], bufsize);
            if (retval == 0)
                break;

            if (retval < bufsize)
            {
                // operation was successful
                buf.resize(retval);
                resolved.swap(buf);
            }
            else
            {
                // buffer was too small, grow buffer and try again
                bufsize <<= 1;
            }
        } while (resolved.empty());

        if (resolved.empty())
            return "";

        String ret = FileSystemUtil::fromWide(resolved);

        // need to strip the module filename from the path
        String::size_type pos = ret.rfind('\\');
        if (pos != String::npos)
            ret.erase(pos);

        return ret;
    }

    String FileSystemLayer::resolveBundlePath(String path)
    {
        return path;
    }
    void FileSystemLayer::getConfigPaths()
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        // try to determine the application's path
        String appPath = getModulePath(false);
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        Ogre::String appPath =
            FileSystemUtil::fromWide(Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data());
#endif

        // use application path as config search path
        if (!appPath.empty())
            mConfigPaths.push_back(appPath + '\\');
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        // look relative to the DLL according to PIP structure
        mConfigPaths.push_back(StringUtil::normalizeFilePath(getModulePath(true)+"/../../../bin/"));
#endif
    }
    //---------------------------------------------------------------------
    void FileSystemLayer::prepareUserHome(const Ogre::String& subdir)
    {
        // fill mHomePath
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        WCHAR wpath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, wpath)))
            mHomePath = FileSystemUtil::fromWide(wpath);
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        mHomePath = FileSystemUtil::fromWide(Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data());
#endif

        if(!mHomePath.empty())
        {
            mHomePath += '\\' + subdir + '\\';
            // create release subdir
            if (!createDirectory(mHomePath))
            {
                // couldn't create directory, fall back to current working dir
                mHomePath.clear();
            }
        }
    }
    //---------------------------------------------------------------------
    bool FileSystemLayer::fileExists(const Ogre::String& path)
    {
#if OGRE_FILESYSTEM_USE_WIDECHAR
        return ::_waccess(FileSystemUtil::toWide(path).c_str(), 0) == 0;
#else
        return ::_access(path.c_str(), 0) == 0;
#endif
    }
    //---------------------------------------------------------------------
    bool FileSystemLayer::createDirectory(const Ogre::String& path)
    {
#if OGRE_FILESYSTEM_USE_WIDECHAR
        return ::_wmkdir(FileSystemUtil::toWide(path).c_str()) == 0 || errno == EEXIST;
#else
        return ::_mkdir(path.c_str()) == 0 || errno == EEXIST;
#endif
    }
    //---------------------------------------------------------------------
    bool FileSystemLayer::removeDirectory(const Ogre::String& path)
    {
#if OGRE_FILESYSTEM_USE_WIDECHAR
        return ::_wrmdir(FileSystemUtil::toWide(path).c_str()) == 0 || errno == ENOENT;
#else
        return ::_rmdir(path.c_str()) == 0 || errno == ENOENT;
#endif
    }
    //---------------------------------------------------------------------
    bool FileSystemLayer::removeFile(const Ogre::String& path)
    {
#if OGRE_FILESYSTEM_USE_WIDECHAR
        return ::_wremove(FileSystemUtil::toWide(path).c_str()) == 0 || errno == ENOENT;
#else
        return ::remove(path.c_str()) == 0 || errno == ENOENT;
#endif
    }
    //---------------------------------------------------------------------
    bool FileSystemLayer::renameFile(const Ogre::String& oldname, const Ogre::String& newname)
    {
        if(fileExists(oldname) && fileExists(newname))
            removeFile(newname);
#if OGRE_FILESYSTEM_USE_WIDECHAR
        return ::_wrename(FileSystemUtil::toWide(oldname).c_str(), FileSystemUtil::toWide(newname).c_str()) == 0;
#else
        return ::rename(oldname.c_str(), newname.c_str()) == 0;
#endif
    }
}
