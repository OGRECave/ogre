INCLUDES=-I/usr/src/ogrenew/OgreMain/include -I/usr/src/ogrenew/Tools/XSIExport/include -I/home/jc/XSI_4.2/XSISDK/include
#bin_PROGRAMS=XSIExport
AM_CPPFLAGS = -DEXT_HASH -I/usr/local/include/OGRE

lib_LTLIBRARIES=libXSIExport.la

libXSIExport_la_SOURCES= OgreXSIHelper.cpp \
				OgreXSIMaterialExporter.cpp \
				OgreXSIMeshExporter.cpp \
				OgreXSIExport.cpp \
				OgreXSISkeletonExporter.cpp
libXSIExport_la_LDFLAGS= -L/usr/local/lib -lOgreMain
libXSIExport_la_LIBADD= -lOgreMain
