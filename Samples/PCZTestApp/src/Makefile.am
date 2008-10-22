INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/PCZTestApp/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include -I$(top_srcdir)/PlugIns/PCZSceneManager/include -I$(top_srcdir)/PlugIns/OctreeZone/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = PCZTestApp

PCZTestApp_SOURCES = PCZTestApp.cpp RoomObject.cpp
PCZTestApp_LDFLAGS = -L$(top_builddir)/OgreMain/src -L$(top_builddir)/PlugIns/PCZSceneManager/src
PCZTestApp_LDADD = -lOgreMain -lPlugin_PCZSceneManager $(OIS_LIBS)
