INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/GL/include \
           -I$(top_srcdir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_srcdir)/RenderSystems/GL/src/$(OGRE_GLSUPPORT) \
           -I$(top_srcdir)/RenderSystems/GL/src/nvparse \
           -I$(top_srcdir)/RenderSystems/GL/src/atifs/include \
           -I$(top_srcdir)/RenderSystems/GL/src/GLSL/include \
           $(CFLAGS_VISIBILITY)

DIST_SUBDIRS = win32 GLX
SUBDIRS = $(OGRE_GLSUPPORT) nvparse atifs GLSL

pkglib_LTLIBRARIES = RenderSystem_GL.la

RenderSystem_GL_la_SOURCES = OgreGLEngineDll.cpp \
                             OgreGLPlugin.cpp \
                             OgreGLGpuProgram.cpp \
                             OgreGLGpuNvparseProgram.cpp \
                             OgreGLGpuProgramManager.cpp \
                             OgreGLRenderSystem.cpp \
                             OgreGLSupport.cpp \
                             OgreGLTexture.cpp \
                             OgreGLTextureManager.cpp \
                             OgreGLHardwareBufferManager.cpp \
                             OgreGLHardwareIndexBuffer.cpp \
                             OgreGLHardwareOcclusionQuery.cpp \
                             OgreGLHardwareVertexBuffer.cpp \
                             OgreGLHardwarePixelBuffer.cpp \
                             OgreGLDefaultHardwareBufferManager.cpp \
                             OgreGLContext.cpp \
                             OgreGLPixelFormat.cpp \
                             OgreGLRenderTexture.cpp \
							 OgreGLRenderToVertexBuffer.cpp \
                             OgreGLFBORenderTexture.cpp \
                             OgreGLPBRenderTexture.cpp \
                             OgreGLFBOMultiRenderTarget.cpp \
                             OgreGLFrameBufferObject.cpp \
                             glew.cpp

RenderSystem_GL_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src -Wl,-z,defs
RenderSystem_GL_la_LIBADD = \
    $(top_builddir)/RenderSystems/GL/src/$(OGRE_GLSUPPORT)/libsupport.la \
    $(top_builddir)/RenderSystems/GL/src/GLSL/src/libGLSL.la \
    $(top_builddir)/RenderSystems/GL/src/nvparse/libnvparse.la \
    $(top_builddir)/RenderSystems/GL/src/atifs/src/libatifs.la \
    $(GL_LIBS) $(GLSUPPORT_LIBS) -lOgreMain $(OGRE_THREAD_LIBS)
