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

#include "Ogre.h"
#include "OgreConfigFile.h"
// Static plugin headers
#ifdef ENABLE_PLUGIN_CgProgramManager
#  include "OgreCgPlugin.h"
#endif
#ifdef ENABLE_PLUGIN_ParticleFX
#  include "OgreParticleFXPlugin.h"
#endif
#ifdef ENABLE_PLUGIN_GL
#  include "OgreGLPlugin.h"
#endif
#ifdef ENABLE_PLUGIN_GLES
#  include "OgreGLESPlugin.h"
#endif
#ifdef ENABLE_PLUGIN_Direct3D9
#  include "OgreD3D9Plugin.h"
#endif

namespace Ogre
{
    /** Utility class for loading some plugins statically.
    @remarks
        When loading plugins statically, you are limited to loading plugins 
        that are known about at compile time. You should define preprocessor
        symbols depending on which plugins you want to load - the symbol being
        ENABLE_PLUGIN_<pluginname>, with pluginname being the usual name of the
        plugin DLL (no file extension, no debug suffix, and without the Plugin_ 
        or RenderSystem_ prefix.)
    */
    class StaticPluginLoader
    {
    protected:
#ifdef ENABLE_PLUGIN_CgProgramManager
        CgPlugin* mCgPlugin;
#endif
#ifdef ENABLE_PLUGIN_ParticleFX
        ParticleFXPlugin* mParticleFXPlugin;
#endif
#ifdef ENABLE_PLUGIN_GL
        GLPlugin* mGLPlugin;
#endif
#ifdef ENABLE_PLUGIN_GLES
        GLESPlugin* mGLESPlugin;
#endif
#ifdef ENABLE_PLUGIN_Direct3D9
        D3D9Plugin* mD3D9Plugin;
#endif
    public:
        StaticPluginLoader() {}

        /** Load all the enabled plugins against the passed in root object. */
        void load(Root& root)
        {
#ifdef ENABLE_PLUGIN_GL
            mGLPlugin = new GLPlugin();
            root.installPlugin(mGLPlugin);
#endif
#ifdef ENABLE_PLUGIN_GLES
            mGLESPlugin = new GLESPlugin();
            root.installPlugin(mGLESPlugin);
#endif
#ifdef ENABLE_PLUGIN_Direct3D9
            mD3D9Plugin = new D3D9Plugin();
            root.installPlugin(mD3D9Plugin);
#endif
#ifdef ENABLE_PLUGIN_CgProgramManager
            mCgPlugin = new CgPlugin();
            root.installPlugin(mCgPlugin);
#endif
#ifdef ENABLE_PLUGIN_ParticleFX
            mParticleFXPlugin = new ParticleFXPlugin();
            root.installPlugin(mParticleFXPlugin);
#endif
        }

        void unload()
        {
            // don't unload plugins, since Root will have done that. Destroy here.
#ifdef ENABLE_PLUGIN_ParticleFX
            delete mParticleFXPlugin;
#endif
#ifdef ENABLE_PLUGIN_CgProgramManager
            delete mCgPlugin;
#endif
#ifdef ENABLE_PLUGIN_Direct3D9
            delete mD3D9Plugin;
#endif
#ifdef ENABLE_PLUGIN_GLES
            delete mGLESPlugin;
#endif

        }



    };

}


#endif

