INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/SkeletalAnimation/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = SkeletalAnimation

SkeletalAnimation_SOURCES = SkeletalAnimation.cpp
SkeletalAnimation_LDFLAGS = -L$(top_builddir)/OgreMain/src
SkeletalAnimation_LDADD = -lOgreMain $(OIS_LIBS)
