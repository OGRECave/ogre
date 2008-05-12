INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/OctreeSceneManager/include -I$(top_srcdir)/OgreMain/include -DPLUGIN_TERRAIN_EXPORTS

pkglib_LTLIBRARIES = Plugin_OctreeSceneManager.la

Plugin_OctreeSceneManager_la_SOURCES = \
							OgreHeightmapTerrainPageSource.cpp \
							OgreOctreeSceneManagerDll.cpp \
                            OgreOctreeSceneManager.cpp \
														OgreOctreeSceneQuery.cpp \
                            OgreOctreeNode.cpp \
                            OgreOctreeCamera.cpp \
							OgreOctree.cpp \
							OgreOctreePlugin.cpp \
							OgreTerrainPage.cpp \
							OgreTerrainPageSource.cpp \
							OgreTerrainRenderable.cpp \
							OgreTerrainSceneManager.cpp \
							OgreTerrainVertexProgram.cpp 


Plugin_OctreeSceneManager_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src -Wl,-z,defs
Plugin_OctreeSceneManager_la_LIBADD = -lOgreMain $(OGRE_THREAD_LIBS)
