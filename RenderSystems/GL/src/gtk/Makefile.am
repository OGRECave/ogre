INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/GL/include \
           -I$(top_srcdir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_srcdir)/PlatformManagers/$(OGRE_PLATFORM)/include \
           -I$(top_srcdir)/RenderSystems/GL/include/$(OGRE_GLSUPPORT) \
           -I$(top_srcdir)/RenderSystems/GL/src/$(OGRE_GLSUPPORT)

noinst_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES = OgreGTKGLSupport.cpp OgreGTKWindow.cpp
