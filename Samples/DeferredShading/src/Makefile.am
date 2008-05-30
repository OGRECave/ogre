INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/DeferredShading/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = DeferredShading
DeferredShading_SOURCES = DeferredShading.cpp \
DeferredShadingDemo.cpp \
LightMaterialGenerator.cpp \
MaterialGenerator.cpp \
MLight.cpp \
AmbientLight.cpp \
GeomUtils.cpp

DeferredShading_LDFLAGS = -L$(top_builddir)/OgreMain/src
DeferredShading_LDADD = -lOgreMain $(OIS_LIBS)
