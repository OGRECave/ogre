INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/ParticleFX/include -I$(top_srcdir)/OgreMain/include -DPLUGIN_ParticleFX_EXPORTS

pkglib_LTLIBRARIES = Plugin_ParticleFX.la

Plugin_ParticleFX_la_SOURCES = OgreAreaEmitter.cpp \
                        OgreBoxEmitter.cpp \
                        OgreColourFaderAffector.cpp \
                        OgreColourFaderAffector2.cpp \
						OgreColourImageAffector.cpp \
						OgreColourInterpolatorAffector.cpp \
                        OgreCylinderEmitter.cpp \
						OgreDeflectorPlaneAffector.cpp \
						OgreDirectionRandomiserAffector.cpp \
                        OgreEllipsoidEmitter.cpp \
                        OgreHollowEllipsoidEmitter.cpp \
                        OgreLinearForceAffector.cpp \
                        OgreParticleFX.cpp \
                        OgreParticleFXPlugin.cpp \
                        OgrePointEmitter.cpp \
                        OgreRingEmitter.cpp \
						OgreRotationAffector.cpp \
						OgreScaleAffector.cpp 

Plugin_ParticleFX_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src -Wl,-z,defs
Plugin_ParticleFX_la_LIBADD = -lOgreMain $(OGRE_THREAD_LIBS)
