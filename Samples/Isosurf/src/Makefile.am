INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Isosurf/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = Isosurf

Isosurf_SOURCES = Isosurf.cpp ProceduralTools.cpp
Isosurf_LDFLAGS = -L$(top_builddir)/OgreMain/src
Isosurf_LDADD = -lOgreMain $(OIS_LIBS)

noinst_HEADERS = ProceduralTools.h
