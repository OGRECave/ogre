INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/CameraTrack/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = CameraTrack

CameraTrack_SOURCES = CameraTrack.cpp
CameraTrack_LDFLAGS = -L$(top_builddir)/OgreMain/src
CameraTrack_LDADD = -lOgreMain $(OIS_LIBS)
