INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Smoke/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = Smoke

Smoke_SOURCES = Smoke.cpp
Smoke_LDFLAGS = -L$(top_builddir)/OgreMain/src
Smoke_LDADD = -lOgreMain $(OIS_LIBS)
