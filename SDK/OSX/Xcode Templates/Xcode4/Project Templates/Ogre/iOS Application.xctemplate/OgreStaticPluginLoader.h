/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd
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
#ifdef OGRE_STATIC_CgProgramManager
#  include "OgreCgPlugin.h"
#endif
#ifdef OGRE_STATIC_ParticleFX
#  include "OgreParticleFXPlugin.h"
#endif
#ifdef OGRE_STATIC_GL
#  include "OgreGLPlugin.h"
#endif
#ifdef OGRE_STATIC_GLES
#  include "OgreGLESPlugin.h"
#endif
#ifdef OGRE_STATIC_GLES2
#  include "OgreGLES2Plugin.h"
#endif
#ifdef OGRE_STATIC_Direct3D9
#  include "OgreD3D9Plugin.h"
#endif
#ifdef OGRE_STATIC_Direct3D10
#  include "OgreD3D10Plugin.h"
#endif

namespace Ogre
{
    /** Utility class for loading some plugins statically.
    @remarks
        When loading plugins statically, you are limited to loading plugins 
        that are known about at compile time. You should define preprocessor
        symbols depending on which plugins you want to load - the symbol being
        OGRE_STATIC_<pluginname>, with pluginname being the usual name of the
        plugin DLL (no file extension, no debug suffix, and without the Plugin_ 
        or RenderSystem_ prefix.)
    */
    class StaticPluginLoader
    {
    protected:
#ifdef OGRE_STATIC_CgProgramManager
        CgPlugin* mCgPlugin;
#endif
#ifdef OGRE_STATIC_ParticleFX
        ParticleFXPlugin* mParticleFXPlugin;
#endif
#ifdef OGRE_STATIC_GL
        GLPlugin* mGLPlugin;
#endif
#ifdef OGRE_STATIC_GLES
        GLESPlugin* mGLESPlugin;
#endif
#ifdef OGRE_STATIC_GLES2
        GLES2Plugin* mGLES2Plugin;
#endif
#ifdef OGRE_STATIC_Direct3D9
        D3D9Plugin* mD3D9Plugin;
#endif
#ifdef OGRE_STATIC_Direct3D10
        D3D10Plugin* mD3D10Plugin;
#endif
    public:
        StaticPluginLoader() {}

        /** Load all the enabled plugins against the passed in root object. */
        void load()
        {
            Root& root  = Root::getSingleton();
#ifdef OGRE_STATIC_GL
            mGLPlugin = OGRE_NEW GLPlugin();
            root.installPlugin(mGLPlugin);
#endif
#ifdef OGRE_STATIC_GLES
            mGLESPlugin = OGRE_NEW GLESPlugin();
            root.installPlugin(mGLESPlugin);
#endif
#ifdef OGRE_STATIC_GLES2
            mGLES2Plugin = OGRE_NEW GLES2Plugin();
            root.installPlugin(mGLES2Plugin);
#endif
#ifdef OGRE_STATIC_Direct3D9
            mD3D9Plugin = OGRE_NEW D3D9Plugin();
            root.installPlugin(mD3D9Plugin);
#endif
#ifdef OGRE_STATIC_Direct3D10
            mD3D10Plugin = OGRE_NEW D3D10Plugin();
            root.installPlugin(mD3D10Plugin);
#endif
#ifdef OGRE_STATIC_CgProgramManager
            mCgPlugin = OGRE_NEW CgPlugin();
            root.installPlugin(mCgPlugin);
#endif
#ifdef OGRE_STATIC_ParticleFX
            mParticleFXPlugin = OGRE_NEW ParticleFXPlugin();
            root.installPlugin(mParticleFXPlugin);
#endif
        }

        void unload()
        {
            // don't unload plugins, since Root will have done that. Destroy here.
#ifdef OGRE_STATIC_ParticleFX
            OGRE_DELETE mParticleFXPlugin;
#endif
#ifdef OGRE_STATIC_CgProgramManager
            OGRE_DELETE mCgPlugin;
#endif
#ifdef OGRE_STATIC_Direct3D9
            OGRE_DELETE mD3D9Plugin;
#endif
#ifdef OGRE_STATIC_Direct3D10
            OGRE_DELETE mD3D10Plugin;
#endif
#ifdef OGRE_STATIC_GL
            OGRE_DELETE mGLPlugin;
#endif
#ifdef OGRE_STATIC_GLES
            OGRE_DELETE mGLESPlugin;
#endif
#ifdef OGRE_STATIC_GLES2
            OGRE_DELETE mGLES2Plugin;
#endif

        }

    };

}

#endif
