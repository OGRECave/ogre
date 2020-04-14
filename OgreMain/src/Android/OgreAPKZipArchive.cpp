#include "OgreStableHeaders.h"
#include <android/asset_manager.h>

namespace Ogre{
    //-----------------------------------------------------------------------
    const String &APKZipArchiveFactory::getType() const
    {
        static String type = "APKZip";
        return type;
    }
    //-----------------------------------------------------------------------
    Archive *APKZipArchiveFactory::createInstance( const String& name, bool readOnly )
    {
        String apkName = name;
        if (apkName.size() > 0 && apkName[0] == '/')
            apkName.erase(apkName.begin());

        AAsset* asset = AAssetManager_open(mAssetMgr, apkName.c_str(), AASSET_MODE_BUFFER);
        if(asset)
        {
            EmbeddedZipArchiveFactory::addEmbbeddedFile(apkName, (const Ogre::uint8*)AAsset_getBuffer(asset), AAsset_getLength(asset), 0);
            mOpenAssets.emplace(apkName, asset);
        }

        return EmbeddedZipArchiveFactory::createInstance(apkName, readOnly);
    }
    void APKZipArchiveFactory::destroyInstance(Archive* ptr)
    {
        AAsset_close(mOpenAssets[ptr->getName()]);
        EmbeddedZipArchiveFactory::destroyInstance(ptr);
    }
}
