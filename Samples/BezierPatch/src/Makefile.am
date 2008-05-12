INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/BezierPatch/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = Bezier

Bezier_SOURCES = Bezier.cpp
Bezier_LDFLAGS = -L$(top_builddir)/OgreMain/src
Bezier_LDADD = -lOgreMain $(OIS_LIBS)
