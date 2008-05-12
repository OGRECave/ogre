INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/BSPSceneManager/include -I$(top_srcdir)/OgreMain/include

pkglib_LTLIBRARIES = Plugin_BSPSceneManager.la

Plugin_BSPSceneManager_la_SOURCES = OgreBspLevel.cpp \
                             OgreBspNode.cpp \
                             OgreBspResourceManager.cpp \
                             OgreBspSceneManager.cpp \
                             OgreBspSceneManagerDll.cpp \
							 OgreBspSceneManagerPlugin.cpp \
							 OgreBspSceneNode.cpp \
                             OgreQuake3ShaderManager.cpp \
                             OgreQuake3Shader.cpp \
                             OgreQuake3Level.cpp


Plugin_BSPSceneManager_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src -Wl,-z,defs
Plugin_BSPSceneManager_la_LIBADD = -lOgreMain -lc $(OGRE_THREAD_LIBS)
