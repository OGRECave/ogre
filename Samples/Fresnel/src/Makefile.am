INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Fresnel/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = Fresnel

Fresnel_SOURCES = Fresnel.cpp
Fresnel_LDFLAGS = -L$(top_builddir)/OgreMain/src
Fresnel_LDADD = -lOgreMain $(OIS_LIBS)
