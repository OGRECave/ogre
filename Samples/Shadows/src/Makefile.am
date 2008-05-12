INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/CEGUIRenderer/include -I$(top_srcdir)/Samples/Common/include @CEGUI_CFLAGS@ $(OIS_CFLAGS) 

ogreexec_PROGRAMS = Shadows
Shadows_SOURCES = Shadows.cpp
Shadows_LDFLAGS = -L$(top_builddir)/OgreMain/src -L$(top_builddir)/Samples/Common/CEGUIRenderer/src
Shadows_LDADD = -lOgreMain @CEGUI_LIBS@ -lCEGUIOgreRenderer $(OIS_LIBS)
