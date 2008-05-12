INCLUDES = $(STLPORT_CFLAGS) \
	   -I$(top_srcdir)/RenderSystems/GL/include \
	   -I$(top_srcdir)/RenderSystems/GL/include/GLX \
           -I$(top_srcdir)/RenderSystems/GL/src/GLSL/include \
	   -I$(top_srcdir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
	   $(CFLAGS_VISIBILITY)

noinst_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES = OgreGLXGLSupport.cpp \
			OgreGLXWindow.cpp \
			OgreGLXRenderTexture.cpp \
			OgreGLXContext.cpp
