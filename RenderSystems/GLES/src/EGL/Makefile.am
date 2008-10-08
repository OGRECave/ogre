INCLUDES = $(STLPORT_CFLAGS) \
	-I$(top_srcdir)/RenderSystems/GLES/include \
	-I$(top_srcdir)/RenderSystems/GLES/include/EGL \
	-I$(top_srcdir)/OgreMain/include \
	$(CFLAGS_VISIBILITY)

noinst_LTLIBRARIES = libsupport.la
libsupport_la_SOURCES = \
	OgreEGLContext.cpp \
	OgreEGLRenderTexture.cpp \
	OgreEGLSupport.cpp \
	OgreEGLWindow.cpp
