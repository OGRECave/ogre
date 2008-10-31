INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/ParticleGS/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = ParticleGS

ParticleGS_SOURCES = ParticleGS.cpp ProceduralManualObject.cpp RandomTools.cpp
ParticleGS_LDFLAGS = -L$(top_builddir)/OgreMain/src
ParticleGS_LDADD = -lOgreMain $(OIS_LIBS)

noinst_HEADERS = ProceduralManualObject.h RandomTools.h
