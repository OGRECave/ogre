INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Instancing/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include -I$(top_srcdir)/Samples/Common/CEGUIRenderer/include @CEGUI_CFLAGS@ $(OIS_CFLAGS)

ogreexec_PROGRAMS = Instancing Crowd

Instancing_SOURCES = Instancing.cpp
Instancing_LDFLAGS = -L$(top_builddir)/OgreMain/src -L$(top_builddir)/Samples/Common/CEGUIRenderer/src
Instancing_LDADD = -lOgreMain @CEGUI_LIBS@ -lCEGUIOgreRenderer $(OIS_LIBS)

Crowd_SOURCES = crowd.cpp
Crowd_LDFLAGS = -L$(top_builddir)/OgreMain/src -L$(top_builddir)/Samples/Common/CEGUIRenderer/src
Crowd_LDADD = -lOgreMain @CEGUI_LIBS@ -lCEGUIOgreRenderer $(OIS_LIBS)
