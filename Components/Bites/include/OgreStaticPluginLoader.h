/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    StaticPluginLoader.h
Description: Utility class to load plugins statically
-----------------------------------------------------------------------------
*/

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
