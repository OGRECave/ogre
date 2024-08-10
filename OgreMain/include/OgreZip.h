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
#ifndef __Zip_H__
#define __Zip_H__

#include "OgrePrerequisites.h"

#include "OgreArchive.h"
#include "OgreArchiveFactory.h"
#include "OgreHeaderPrefix.h"
#include "Threading/OgreThreadHeaders.h"

struct AAssetManager;
struct AAsset;

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** Specialisation to allow reading of files from a zip
        format source archive.

        This archive format supports all archives compressed in the standard
        zip format, including iD pk3 files.
    */
    class _OgreExport ZipArchiveFactory : public ArchiveFactory
    {
    public:
        virtual ~ZipArchiveFactory() {}
        /// @copydoc FactoryObj::getType
        const String& getType(void) const override;

        //! @cond Doxygen_Suppress
        using ArchiveFactory::createInstance;
        //! @endcond

        Archive *createInstance( const String& name, bool readOnly ) override;
    };

    /** Specialisation of ZipArchiveFactory for embedded Zip files. */
    class _OgreExport EmbeddedZipArchiveFactory : public ZipArchiveFactory
    {
    public:
        EmbeddedZipArchiveFactory();
        virtual ~EmbeddedZipArchiveFactory();

        const String& getType(void) const override;

        //! @cond Doxygen_Suppress
        using ArchiveFactory::createInstance;
        //! @endcond

        Archive *createInstance( const String& name, bool readOnly ) override;
        void destroyInstance( Archive* ptr) override;
        
        /** a function type to decrypt embedded zip file
        @param pos pos in file
        @param buf current buffer to decrypt
        @param len - length of buffer
        @return success
        */  
        typedef bool (*DecryptEmbeddedZipFileFunc)(size_t pos, void* buf, size_t len);

        /// Add an embedded file to the embedded file list
        static void addEmbbeddedFile(const String& name, const uint8 * fileData, 
                        size_t fileSize, DecryptEmbeddedZipFileFunc decryptFunc);

        /// Remove an embedded file to the embedded file list
        static void removeEmbbeddedFile(const String& name);

    };

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    class APKZipArchiveFactory : public EmbeddedZipArchiveFactory
    {
        std::map<String, AAsset*> mOpenAssets;
    private:
        AAssetManager* mAssetMgr;
    public:
        APKZipArchiveFactory(AAssetManager* assetMgr) : mAssetMgr(assetMgr) {}
        virtual ~APKZipArchiveFactory() {}

        const String& getType(void) const override;
        Archive *createInstance( const String& name, bool readOnly ) override;
        void destroyInstance( Archive* ptr) override;
    };
#endif

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
