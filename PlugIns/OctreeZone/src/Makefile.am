INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/OctreeZone/include -I$(top_srcdir)/PlugIns/PCZSceneManager/include -I$(top_srcdir)/OgreMain/include

pkglib_LTLIBRARIES = Plugin_OctreeZone.la

Plugin_OctreeZone_la_SOURCES = \
	OgreHeightmapTerrainZonePageSource.cpp \
	OgreOctreeZoneOctree.cpp \
	OgreOctreeZone.cpp \
	OgreOctreeZoneDll.cpp \
	OgreOctreeZonePlugin.cpp \
	OgreTerrainVertexProgram.cpp \
	OgreTerrainZone.cpp \
	OgreTerrainZonePage.cpp \
	OgreTerrainZonePageSource.cpp \
	OgreTerrainZoneRenderable.cpp

Plugin_OctreeZone_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src -L$(top_builddir)/PlugIns/PCZSceneManager/src -Wl,-z,defs
Plugin_OctreeZone_la_LIBADD = -lOgreMain -lPlugin_PCZSceneManager $(OGRE_THREAD_LIBS)
