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

namespace Ogre
{
    static bool widePathToOgreString(String& dest, const WCHAR* wpath)
    {
        // need to convert to narrow (OEM or ANSI) codepage so that fstream can use it 
        // properly on international systems.
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        // Note, that on legacy CRT versions codepage for narrow CRT file functions can be changed using 
        // SetFileApisANSI/OEM, but on modern runtimes narrow pathes are always widened using ANSI codepage.
        // We suppose that on such runtimes file APIs codepage is left in default, ANSI state.
        UINT codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        // Runtime is modern, narrow calls are widened inside CRT using CP_ACP codepage.
        UINT codepage = CP_ACP;
#endif
        const int wlength = static_cast<int>(wcslen(wpath));
        const int length = WideCharToMultiByte(codepage, 0 /* Use default flags */, wpath, wlength, NULL, 0, NULL, NULL);
        if(length <= 0)
        {
            dest.clear();
            return false;
        }

        // success
        dest.resize(length);
        WideCharToMultiByte(codepage, 0 /* Use default flags */, wpath, wlength, &dest[0], (int)dest.size(), NULL, NULL);
        return true;
    }

    static String getModulePath(bool dllpath)
    {
        HMODULE hm = NULL;

        // get path of this DLL
        if (dllpath && !GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                          (LPCSTR)&getModulePath, &hm))
        {
            return "";
        }

        DWORD bufsize = 256;
        char* resolved = 0;
        do
        {
            char* buf = OGRE_ALLOC_T(char, bufsize, Ogre::MEMCATEGORY_GENERAL);
            DWORD retval = GetModuleFileName(hm, buf, bufsize);
            if (retval == 0)
            {
                // failed
                OGRE_FREE(buf, Ogre::MEMCATEGORY_GENERAL);
                break;
            }

            if (retval < bufsize)
            {
                // operation was successful.
                resolved = buf;
            }
            else
            {
                // buffer was too small, grow buffer and try again
                OGRE_FREE(buf, Ogre::MEMCATEGORY_GENERAL);
                bufsize <<= 1;
            }
        } while (!resolved);

        String ret = resolved;
        OGRE_FREE(resolved, Ogre::MEMCATEGORY_GENERAL);

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
        Ogre::String appPath;
        widePathToOgreString(appPath, Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data());
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
            widePathToOgreString(mHomePath, wpath);
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        widePathToOgreString(mHomePath, Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data());
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
        return _access(path.c_str(), 04) == 0; // Use CRT API rather than GetFileAttributesExA to pass Windows Store validation
    }
    //---------------------------------------------------------------------
    bool FileSystemLayer::createDirectory(const Ogre::String& path)
    {
        return !_mkdir(path.c_str()) || errno == EEXIST; // Use CRT API rather than CreateDirectoryA to pass Windows Store validation
    }
    //---------------------------------------------------------------------
    bool FileSystemLayer::removeDirectory(const Ogre::String& path)
    {
        return !_rmdir(path.c_str()) || errno == ENOENT; // Use CRT API to pass Windows Store validation
    }
    //---------------------------------------------------------------------
    bool FileSystemLayer::removeFile(const Ogre::String& path)
    {
        return !_unlink(path.c_str()) || errno == ENOENT; // Use CRT API to pass Windows Store validation
    }
    //---------------------------------------------------------------------
    bool FileSystemLayer::renameFile(const Ogre::String& oldname, const Ogre::String& newname)
    {
        if(fileExists(oldname) && fileExists(newname))
            removeFile(newname);
        return !rename(oldname.c_str(), newname.c_str()); // Use CRT API to pass Windows Store validation
    }
}
