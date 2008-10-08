INCLUDES = $(STLPORT_CFLAGS) \
			-I$(top_srcdir)/RenderSystems/GLES/include \
			-I$(top_srcdir)/RenderSystems/GLES/include/$(OGRE_GLESSUPPORT) \
			-I$(top_srcdir)/OgreMain/include \
			-I$(top_srcdir)/RenderSystems/GLES/src/$(OGRE_GLESSUPPORT) \
			$(GLESSUPPORT_CFLAGS) \
			$(CFLAGS_VISIBILITY)

DIST_SUBDIRS = EGL
SUBDIRS = $(OGRE_GLESSUPPORT)

pkglib_LTLIBRARIES = RenderSystem_GLES.la

RenderSystem_GLES_la_SOURCES = \
	OgreGLESContext.cpp \
	OgreGLESDefaultHardwareBufferManager.cpp \
	OgreGLESEngineDll.cpp \
	OgreGLESGpuProgramManager.cpp \
	OgreGLESHardwareBufferManager.cpp \
	OgreGLESHardwareIndexBuffer.cpp \
	OgreGLESHardwarePixelBuffer.cpp \
	OgreGLESHardwareVertexBuffer.cpp \
	OgreGLESPBRenderTexture.cpp \
	OgreGLESPixelFormat.cpp \
	OgreGLESPlugin.cpp \
	OgreGLESRenderSystem.cpp \
	OgreGLESRenderTexture.cpp \
	OgreGLESSupport.cpp \
	OgreGLESTexture.cpp \
	OgreGLESTextureManager.cpp

RenderSystem_GLES_la_LDFLAGS = \
	-module $(PLUGIN_FLAGS) \
	-L$(top_builddir)/OgreMain/src -Wl,-z,defs

RenderSystem_GLES_la_LIBADD = \
	$(top_builddir)/RenderSystems/GLES/src/$(OGRE_GLESSUPPORT)/libsupport.la \
	$(GLESSUPPORT_LIBS) \
	-lOgreMain \
	$(OGRE_THREAD_LIBS)
