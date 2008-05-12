INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Common/CEGUIRenderer/include -I$(top_srcdir)/OgreMain/include @CEGUI_CFLAGS@ $(CFLAGS_VISIBILITY)

lib_LTLIBRARIES = libCEGUIOgreRenderer.la

libCEGUIOgreRenderer_la_SOURCES = OgreCEGUIRenderer.cpp OgreCEGUITexture.cpp OgreCEGUIResourceProvider.cpp
libCEGUIOgreRenderer_la_LIBADD  = $(STLPORT_LIBS)
libCEGUIOgreRenderer_la_LDFLAGS = -release @PACKAGE_VERSION@

