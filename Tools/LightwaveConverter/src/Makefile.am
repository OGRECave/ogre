INCLUDES=-I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Tools/LightwaveConverter/include
bin_PROGRAMS=LightwaveConverter
LightwaveConverter_CXXFLAGS=
LightwaveConverter_SOURCES= BitArray.cpp \
							lwEnvelope.cpp \
							lwLayer.cpp \
							lwo2mesh.cpp \
							lwPolygon.cpp \
							lwReader.cpp \
							main.cpp \
							Vector3.cpp
LightwaveConverter_LDFLAGS= -L$(top_builddir)/OgreMain/src
LightwaveConverter_LDADD= -lOgreMain
