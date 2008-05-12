INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/TextureFX/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = TextureFX
TextureFX_SOURCES = TextureFX.cpp
TextureFX_LDFLAGS = -L$(top_builddir)/OgreMain/src
TextureFX_LDADD = -lOgreMain $(OIS_LIBS)
