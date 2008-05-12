INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Terrain/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = Terrain 
Terrain_SOURCES = Terrain.cpp
Terrain_LDFLAGS = -L$(top_builddir)/OgreMain/src
Terrain_LDADD = -lOgreMain -lpthread $(OIS_LIBS)
