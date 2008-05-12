INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/VolumeTex/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = VolumeTex
VolumeTex_SOURCES = VolumeTex.cpp VolumeRenderable.cpp ThingRenderable.cpp
VolumeTex_LDFLAGS = -L$(top_builddir)/OgreMain/src
VolumeTex_LDADD = -lOgreMain $(OIS_LIBS)
