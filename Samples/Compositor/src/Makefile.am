INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Compositor/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include -I$(top_srcdir)/Samples/Common/CEGUIRenderer/include @CEGUI_CFLAGS@ $(OIS_CFLAGS)

ogreexec_PROGRAMS = Compositor

Compositor_SOURCES = Compositor.cpp CompositorDemo_FrameListener.cpp ItemSelectorViewManager.cpp
Compositor_LDFLAGS = -L$(top_builddir)/OgreMain/src -L$(top_builddir)/Samples/Common/CEGUIRenderer/src
Compositor_LDADD = -lOgreMain @CEGUI_LIBS@ -lCEGUIOgreRenderer $(OIS_LIBS)
