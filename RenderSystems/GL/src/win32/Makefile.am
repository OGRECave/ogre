INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/GL/include \
           -I$(top_srcdir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_srcdir)/PlatformManagers/$(OGRE_PLATFORM)/include \
           $(CFLAGS_VISIBILITY)

noinst_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES = ../OgreWin32GLSupport.cpp \
		../OgreWin32RenderTexture.cpp \
		../OgreWin32Window.cpp \
		OgreWin32Context.cpp
libsupport_la_LDFLAGS =
