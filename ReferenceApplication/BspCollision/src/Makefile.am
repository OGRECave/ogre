INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/../OgreMain/include -I$(top_srcdir)/Common/include -I$(top_srcdir)/ReferenceAppLayer/include

noinst_PROGRAMS = BspCollision

BspCollision_SOURCES = BspCollision.cpp
BspCollision_LDFLAGS = -L$(top_builddir)/../OgreMain/src -L$(top_builddir)/ReferenceAppLayer/src
BspCollision_LDADD = -lOgreMain -lOgreReferenceAppLayer

install-exec-local:
	$(INSTALL_PROGRAM) .libs/BspCollision $(top_builddir)/../Samples/Common/bin/BspCollision
