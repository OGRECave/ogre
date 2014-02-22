#include "Android/OgreAPKZipArchive.h"

#include <OgreStringConverter.h>
#include <OgreLogManager.h>

namespace Ogre{
    //-----------------------------------------------------------------------
    const String &APKZipArchiveFactory::getType() const
    {
        static String type = "APKZip";
        return type;
    }
    //-----------------------------------------------------------------------
}
