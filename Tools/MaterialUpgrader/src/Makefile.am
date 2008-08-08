INCLUDES=$(STLPORT_CFLAGS) -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Tools/MaterialUpgrader/include
bin_PROGRAMS=OgreMaterialUpgrade
OgreMaterialUpgrade_SOURCES= main.cpp OldMaterialReader.cpp 
OgreMaterialUpgrade_LDFLAGS= -L$(top_builddir)/OgreMain/src
OgreMaterialUpgrade_LDADD= -lOgreMain
