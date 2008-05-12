INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/Direct3D9/include \
           -I$(top_srcdir)/OgreMain/include \
           -I$(top_srcdir)/PlatformManagers/$(OGRE_PLATFORM)/include


pkglib_LTLIBRARIES = RenderSystem_Direct3D9.la

RenderSystem_Direct3D9_la_SOURCES = \
OgreD3D9Driver.cpp \
OgreD3D9DriverList.cpp \
OgreD3D9EngineDll.cpp \
OgreD3D9GpuProgram.cpp \
OgreD3D9GpuProgramManager.cpp \
OgreD3D9HLSLProgram.cpp \
OgreD3D9HLSLProgramFactory.cpp \
OgreD3D9HardwareBufferManager.cpp \
OgreD3D9HardwareIndexBuffer.cpp \
OgreD3D9HardwareOcclusionQuery.cpp \
OgreD3D9HardwareVertexBuffer.cpp \
OgreD3D9Mappings.cpp \
OgreD3D9Plugin.cpp \
OgreD3D9RenderSystem.cpp \
OgreD3D9RenderWindow.cpp \
OgreD3D9Texture.cpp \
OgreD3D9TextureManager.cpp \
OgreD3D9VertexDeclaration.cpp \
OgreD3D9VideoMode.cpp \
OgreD3D9VideoModeList.cpp \
OgreD3D9HardwarePixelBuffer.cpp

RenderSystem_Direct3D9_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src
RenderSystem_Direct3D9_la_LIBADD =  -lOgreMain -ld3d9 -ld3dx9d -lDxErr9 -lgdi32 -ldxguid

