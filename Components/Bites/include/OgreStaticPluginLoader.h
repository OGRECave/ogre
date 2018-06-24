// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#ifndef __StaticPluginLoader_H__
#define __StaticPluginLoader_H__

#include "OgreBitesPrerequisites.h"
#include "OgrePrerequisites.h"

namespace OgreBites
{
    /** Utility class for loading the plugins statically.

        When loading plugins statically, you are limited to loading plugins
        that are known about at compile time. This class will load all built
        plugins based on OgreBuildSettings.h
    */
    class _OgreBitesExport StaticPluginLoader {
        std::vector<Ogre::Plugin*> mPlugins;

    public:
        /** Load all the enabled plugins */
        void load();

        void unload();
    };
}

#endif
