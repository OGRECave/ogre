INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/SkyBox/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = SkyBox
SkyBox_SOURCES = SkyBox.cpp
SkyBox_LDFLAGS = -L$(top_builddir)/OgreMain/src
SkyBox_LDADD = -lOgreMain $(OIS_LIBS)
