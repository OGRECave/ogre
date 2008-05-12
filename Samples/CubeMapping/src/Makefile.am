INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/CubeMapping/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = CubeMapping
CubeMapping_SOURCES = CubeMapping.cpp
CubeMapping_LDFLAGS = -L$(top_builddir)/OgreMain/src
CubeMapping_LDADD = -lOgreMain $(OIS_LIBS)
