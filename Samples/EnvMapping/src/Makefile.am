INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/EnvMapping/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = EnvMapping

EnvMapping_SOURCES = EnvMapping.cpp
EnvMapping_LDFLAGS = -L$(top_builddir)/OgreMain/src
EnvMapping_LDADD = -lOgreMain $(OIS_LIBS)
