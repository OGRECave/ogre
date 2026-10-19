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
#ifndef __FileSystem_H__
#define __FileSystem_H__

#include "OgrePrerequisites.h"

#include "OgreArchive.h"
#include "OgreArchiveFactory.h"
#include "OgreHeaderPrefix.h"

struct AAssetManager;

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** internal method to open a FileStreamDataStream

     The internal file access methods below are intentionally not exported, to keep the
     platform specific path handling (e.g. UTF-8 paths on Windows) contained inside OgreMain.
     Outside of OgreMain use Root::openFileStream, Root::createFileStream and the static
     helpers of FileSystemLayer instead.
     */
    DataStreamPtr _openFileStream(const String& path, std::ios::openmode mode, const String& name = "");

    /** internal method to open a std::fstream
     *
     * Behaves like calling `stream.open(path, mode)`, but supports UTF-8 paths on Windows
     * if OGRE_CONFIG_FILESYSTEM_UNICODE is enabled. Check `stream.is_open()` for success.
     */
    void _openStdFstream(std::ifstream& stream, const String& path, std::ios::openmode mode = std::ios::in);
    /// @overload
    void _openStdFstream(std::ofstream& stream, const String& path, std::ios::openmode mode = std::ios::out);
    /// @overload
    void _openStdFstream(std::fstream& stream, const String& path,
                         std::ios::openmode mode = std::ios::in | std::ios::out);

    /** internal method to delete a file
     *
     * Behaves like calling `remove(path)`, but supports UTF-8 paths on Windows
     * if OGRE_CONFIG_FILESYSTEM_UNICODE is enabled.
     * @return true on success
     */
    bool _removeFile(const String& path);

    /** Specialisation of the ArchiveFactory to allow reading of files from
        filesystem folders / directories.
    */
    class _OgreExport FileSystemArchiveFactory : public ArchiveFactory
    {
    public:
        /// @copydoc FactoryObj::getType
        const String& getType(void) const override;

        //! @cond Doxygen_Suppress
        using ArchiveFactory::createInstance;
        //! @endcond

        Archive *createInstance( const String& name, bool readOnly ) override;

        /// Set whether filesystem enumeration will include hidden files or not.
        /// This should be called prior to declaring and/or initializing filesystem
        /// resource locations. The default is true (ignore hidden files).
        static void setIgnoreHidden(bool ignore);

        /// Get whether hidden files are ignored during filesystem enumeration.
        static bool getIgnoreHidden();
    };

    class APKFileSystemArchiveFactory : public ArchiveFactory
    {
    public:
        APKFileSystemArchiveFactory(AAssetManager* assetMgr) : mAssetMgr(assetMgr) {}
        virtual ~APKFileSystemArchiveFactory() {}
        /// @copydoc FactoryObj::getType
        const String& getType(void) const override;
        /// @copydoc ArchiveFactory::createInstance
        Archive *createInstance( const String& name, bool readOnly ) override;
    private:
        AAssetManager* mAssetMgr;
    };

    /** @} */
    /** @} */

} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __FileSystem_H__
