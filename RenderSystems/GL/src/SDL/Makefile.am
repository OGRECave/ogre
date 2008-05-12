INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/GL/include \
           -I$(top_srcdir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_srcdir)/PlatformManagers/$(OGRE_PLATFORM)/include 

noinst_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES = OgreSDLGLSupport.cpp OgreSDLWindow.cpp
