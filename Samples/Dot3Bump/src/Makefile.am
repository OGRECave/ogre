INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = Dot3Bump

Dot3Bump_SOURCES = Dot3Bump.cpp
Dot3Bump_LDFLAGS = -L$(top_builddir)/OgreMain/src
Dot3Bump_LDADD = -lOgreMain $(OIS_LIBS)
