INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/GL/include \
           -I$(top_srcdir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_srcdir)/PlatformManagers/$(OGRE_PLATFORM)/include \
           -I$(top_srcdir)/RenderSystems/GL/src/$(OGRE_GLSUPPORT) \
           -I$(top_srcdir)/RenderSystems/GL/src/atifs/include \
           $(CFLAGS_VISIBILITY)

noinst_LTLIBRARIES=libatifs.la
libatifs_la_SOURCES = ATI_FS_GLGpuProgram.cpp \
                     Compiler2Pass.cpp \
                     ps_1_4.cpp
