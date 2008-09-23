INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/CgProgramManager/include -I$(top_srcdir)/OgreMain/include

pkglib_LTLIBRARIES = Plugin_CgProgramManager.la

Plugin_CgProgramManager_la_SOURCES = OgreCgProgram.cpp \
							 OgreCgFxScriptLoader.cpp \
							 OgreCgPlugin.cpp \
                             OgreCgProgramFactory.cpp \
                             OgreCgProgramManagerDll.cpp 

Plugin_CgProgramManager_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src -Wl,-z,defs
Plugin_CgProgramManager_la_LIBADD = -lOgreMain -lCg $(OGRE_THREAD_LIBS)
