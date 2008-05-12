INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/BSP/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = BSP

BSP_SOURCES = BSP.cpp
BSP_LDFLAGS = -L$(top_builddir)/OgreMain/src
BSP_LDADD = -lOgreMain $(OIS_LIBS)
