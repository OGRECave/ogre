INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/SkyDome/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = SkyDome
SkyDome_SOURCES = SkyDome.cpp
SkyDome_LDFLAGS = -L$(top_builddir)/OgreMain/src
SkyDome_LDADD = -lOgreMain $(OIS_LIBS)
