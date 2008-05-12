INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/DynTex/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = DynTex
DynTex_SOURCES = DynTex.cpp
DynTex_LDFLAGS = -L$(top_builddir)/OgreMain/src
DynTex_LDADD = -lOgreMain $(OIS_LIBS)
