#include "OgreStaticPluginLoader.h"
#include "OgreBuildSettings.h"
#include "OgreComponents.h"
#include "OgreRoot.h"
#include "OgrePlugin.h"

#ifdef OGRE_BUILD_RENDERSYSTEM_GL
#define OGRE_STATIC_GL
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS
#define OGRE_STATIC_GL3Plus
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
#define OGRE_STATIC_GLES2
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_D3D9
#define OGRE_STATIC_Direct3D9
#endif
// dx11 will only work on vista and above, so be careful about statically linking
#ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
#define OGRE_STATIC_Direct3D11
#endif

#ifdef OGRE_BUILD_PLUGIN_BSP
#define OGRE_STATIC_BSPSceneManager
#endif
#ifdef OGRE_BUILD_PLUGIN_PFX
#define OGRE_STATIC_ParticleFX
#endif
#ifdef OGRE_BUILD_PLUGIN_CG
#define OGRE_STATIC_CgProgramManager
#endif

#ifdef OGRE_USE_PCZ
    #ifdef OGRE_BUILD_PLUGIN_PCZ
    #define OGRE_STATIC_PCZSceneManager
    #define OGRE_STATIC_OctreeZone
    #endif
#else
    #ifdef OGRE_BUILD_PLUGIN_OCTREE
    #define OGRE_STATIC_OctreeSceneManager
    #endif
#endif

#ifdef OGRE_STATIC_LIB
// Static plugin headers
#ifdef OGRE_STATIC_CgProgramManager
#  include "OgreCgPlugin.h"
#endif
#ifdef OGRE_BUILD_PLUGIN_ASSIMP
#  include "OgreAssimpLoader.h"
#endif
#ifdef OGRE_STATIC_OctreeSceneManager
#  include "OgreOctreePlugin.h"
#endif
#ifdef OGRE_STATIC_ParticleFX
#  include "OgreParticleFXPlugin.h"
#endif
#ifdef OGRE_STATIC_BSPSceneManager
#  include "OgreBspSceneManagerPlugin.h"
#endif
#ifdef OGRE_STATIC_GL
#  include "OgreGLPlugin.h"
#endif
#ifdef OGRE_STATIC_GL3Plus
#  include "OgreGL3PlusPlugin.h"
#endif
#ifdef OGRE_STATIC_GLES2
#  include "OgreGLES2Plugin.h"
#endif
#ifdef OGRE_STATIC_Direct3D9
#  include "OgreD3D9Plugin.h"
#endif
#ifdef OGRE_STATIC_Direct3D11
#  include "OgreD3D11Plugin.h"
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_TINY
#  include "OgreTinyPlugin.h"
#endif
#ifdef OGRE_STATIC_PCZSceneManager
#  include "OgrePCZPlugin.h"
#endif
#ifdef OGRE_STATIC_OctreeZone
#  include "OgreOctreeZonePlugin.h"
#endif
#ifdef OGRE_BUILD_PLUGIN_STBI
#   include "OgreSTBICodec.h"
#endif
#ifdef OGRE_BUILD_PLUGIN_DOT_SCENE
#   include "OgreDotSceneLoader.h"
#endif
#if defined(OGRE_BUILD_PLUGIN_FREEIMAGE) && !defined(OGRE_BUILD_PLUGIN_STBI)
#   include "OgreFreeImageCodec.h"
#endif
#endif

void OgreBites::StaticPluginLoader::load()
{
    using namespace Ogre;
#ifdef OGRE_STATIC_LIB
    Plugin* plugin = NULL;
#ifdef OGRE_STATIC_GL
    plugin = OGRE_NEW GLPlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_GL3Plus
    plugin = OGRE_NEW GL3PlusPlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_GLES2
    plugin = OGRE_NEW GLES2Plugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_Direct3D9
    plugin = OGRE_NEW D3D9Plugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_TINY
    plugin = OGRE_NEW TinyPlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_Direct3D11
    plugin = OGRE_NEW D3D11Plugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_CgProgramManager
    plugin = OGRE_NEW CgPlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_OctreeSceneManager
    plugin = OGRE_NEW OctreePlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_ParticleFX
    plugin = OGRE_NEW ParticleFXPlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_BSPSceneManager
    plugin = OGRE_NEW BspSceneManagerPlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_PCZSceneManager
    plugin = OGRE_NEW PCZPlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_STATIC_OctreeZone
    plugin = OGRE_NEW OctreeZonePlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_BUILD_PLUGIN_STBI
    plugin = OGRE_NEW STBIPlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_BUILD_PLUGIN_DOT_SCENE
    plugin = OGRE_NEW DotScenePlugin();
    mPlugins.push_back(plugin);
#endif
#if defined(OGRE_BUILD_PLUGIN_FREEIMAGE) && !defined(OGRE_BUILD_PLUGIN_STBI)
    plugin = OGRE_NEW FreeImagePlugin();
    mPlugins.push_back(plugin);
#endif
#ifdef OGRE_BUILD_PLUGIN_ASSIMP
    plugin = OGRE_NEW AssimpPlugin();
    mPlugins.push_back(plugin);
#endif
#endif

    Root& root  = Root::getSingleton();
    for (size_t i = 0; i < mPlugins.size(); ++i) {
        root.installPlugin(mPlugins[i]);
    }
}

void OgreBites::StaticPluginLoader::unload()
{
    // don't unload plugins, since Root will have done that. Destroy here.
    for (size_t i = 0; i < mPlugins.size(); ++i) {
        OGRE_DELETE mPlugins[i];
    }
    mPlugins.clear();
}
