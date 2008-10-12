INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/PCZSceneManager/include -I$(top_srcdir)/OgreMain/include

pkglib_LTLIBRARIES = Plugin_PCZSceneManager.la

Plugin_PCZSceneManager_la_SOURCES = \
	OgreCapsule.cpp \
	OgreDefaultZone.cpp \
	OgrePCPlane.cpp \
	OgrePCZCamera.cpp \
	OgrePCZFrustum.cpp \
	OgrePCZLight.cpp \
	OgrePCZone.cpp \
	OgrePCZoneFactory.cpp \
	OgrePCZPlugin.cpp \
	OgrePCZSceneManager.cpp \
	OgrePCZSceneManagerDll.cpp \
	OgrePCZSceneNode.cpp \
	OgrePCZSceneQuery.cpp \
	OgrePortal.cpp \
	OgreSegment.cpp

Plugin_PCZSceneManager_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src -Wl,-z,defs
Plugin_PCZSceneManager_la_LIBADD = -lOgreMain $(OGRE_THREAD_LIBS)
