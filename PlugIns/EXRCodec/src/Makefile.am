INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/EXRCodec/include -I$(top_srcdir)/OgreMain/include $(OPENEXR_CFLAGS)

pkglib_LTLIBRARIES = Plugin_EXRCodec.la

Plugin_EXRCodec_la_SOURCES = OgreEXRCodec.cpp \
			     O_IStream.cpp \
                             OgreEXRCodecDll.cpp 

Plugin_EXRCodec_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src -Wl,-z,defs
Plugin_EXRCodec_la_LIBADD = -lOgreMain $(OPENEXR_LIBS) $(OGRE_THREAD_LIBS)
