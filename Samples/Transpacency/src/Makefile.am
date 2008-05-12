INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Transpacency/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = Transparency
Transparency_SOURCES = Transparency.cpp
Transparency_LDFLAGS = -L$(top_builddir)/OgreMain/src
Transparency_LDADD = -lOgreMain $(OIS_LIBS)
