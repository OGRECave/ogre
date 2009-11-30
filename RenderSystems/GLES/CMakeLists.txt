#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL ES 1.x RenderSystem build

set(HEADER_FILES
  include/OgreGLESContext.h
  include/OgreGLESDefaultHardwareBufferManager.h
  include/OgreGLESFBOMultiRenderTarget.h
  include/OgreGLESFBORenderTexture.h
  include/OgreGLESFrameBufferObject.h
  include/OgreGLESGpuProgramManager.h
  include/OgreGLESHardwareBufferManager.h
  include/OgreGLESHardwareIndexBuffer.h
  include/OgreGLESHardwarePixelBuffer.h
  include/OgreGLESHardwareVertexBuffer.h
  include/OgreGLESPBRenderTexture.h
  include/OgreGLESPBuffer.h
  include/OgreGLESPixelFormat.h
  include/OgreGLESPlugin.h
  include/OgreGLESPrerequisites.h
  include/OgreGLESRenderSystem.h
  include/OgreGLESRenderTexture.h
  include/OgreGLESSupport.h
  include/OgreGLESTexture.h
  include/OgreGLESTextureManager.h
)


set(SOURCE_FILES
  src/OgreGLESContext.cpp
  src/OgreGLESDefaultHardwareBufferManager.cpp
  src/OgreGLESEngineDll.cpp
  src/OgreGLESFBOMultiRenderTarget.cpp
  src/OgreGLESFBORenderTexture.cpp
  src/OgreGLESFrameBufferObject.cpp
  src/OgreGLESGpuProgramManager.cpp
  src/OgreGLESHardwareBufferManager.cpp
  src/OgreGLESHardwareIndexBuffer.cpp
  src/OgreGLESHardwarePixelBuffer.cpp
  src/OgreGLESHardwareVertexBuffer.cpp
  src/OgreGLESPBRenderTexture.cpp
  src/OgreGLESPixelFormat.cpp
  src/OgreGLESPlugin.cpp
  src/OgreGLESRenderSystem.cpp
  src/OgreGLESRenderTexture.cpp
  src/OgreGLESSupport.cpp
  src/OgreGLESTexture.cpp
  src/OgreGLESTextureManager.cpp
)

# Add system specific settings
if (WIN32)
  set(PLATFORM_HEADERS 
    include/EGL/WIN32/OgreWin32EGLContext.h
    include/EGL/WIN32/OgreWin32EGLSupport.h
    include/EGL/WIN32/OgreWin32EGLRenderTexture.h
    include/EGL/WIN32/OgreWin32EGLWindow.h
    include/EGL/WIN32/OgreGLESUtil.h
    include/EGL/OgreEGLContext.h
    include/EGL/OgreEGLRenderTexture.h
    include/EGL/OgreEGLSupport.h
    include/EGL/OgreEGLWindow.h
  )
  set(PLATFORM_SOURCES
    src/EGL/WIN32/OgreWin32EGLSupport.cpp
    src/EGL/WIN32/OgreWin32EGLRenderTexture.cpp
    src/EGL/WIN32/OgreWin32EGLWindow.cpp
    src/EGL/WIN32/OgreWin32EGLContext.cpp
    src/EGL/OgreEGLContext.cpp
    src/EGL/OgreEGLRenderTexture.cpp
    src/EGL/OgreEGLSupport.cpp
    src/EGL/OgreEGLWindow.cpp
  )
  include_directories(src/EGL/WIN32 include/EGL/WIN32)
  link_directories(${OPENGLES_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "WIN32")
elseif (APPLE)
  set(PLATFORM_HEADERS 
    include/EAGL/OgreEAGLESContext.h
    include/EAGL/OgreEAGLSupport.h
    include/EAGL/OgreEAGLRenderTexture.h
    include/EAGL/OgreEAGLWindow.h
    include/EAGL/OgreGLESUtil.h
  )
  set(PLATFORM_SOURCES
    src/EAGL/OgreEAGLSupport.mm
    src/EAGL/OgreEAGLRenderTexture.mm
    src/EAGL/OgreEAGLWindow.mm
    src/EAGL/OgreEAGLESContext.mm
  )
  include_directories(src/EAGL include/EAGL)
  link_directories(${OPENGLES_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "APPLE")
elseif (UNIX)
  set(PLATFORM_HEADERS
    include/EGL/GTK/OgreGtkEGLContext.h
    include/EGL/GTK/OgreGtkEGLSupport.h
    include/EGL/GTK/OgreGtkEGLRenderTexture.h
    include/EGL/GTK/OgreGtkEGLWindow.h
    include/EGL/GTK/OgreGLESUtil.h
    include/EGL/OgreEGLContext.h
    include/EGL/OgreEGLRenderTexture.h
    include/EGL/OgreEGLSupport.h
    include/EGL/OgreEGLWindow.h
  )
  set(PLATFORM_SOURCES
    src/EGL/GTK/OgreGtkEGLContext.cpp
    src/EGL/GTK/OgreGtkEGLSupport.cpp
    src/EGL/GTK/OgreGtkEGLRenderTexture.cpp
    src/EGL/GTK/OgreGtkEGLWindow.cpp
    src/EGL/OgreEGLContext.cpp
    src/EGL/OgreEGLRenderTexture.cpp
    src/EGL/OgreEGLSupport.cpp
    src/EGL/OgreEGLWindow.cpp
  )
  include_directories(src/EGL/GTK include/EGL/GTK)
  set(PLATFORM_HEADER_INSTALL "GTK")
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
endif ()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
  ${CMAKE_CURRENT_SOURCE_DIR}/include/EAGL
  ${OPENGLES_INCLUDE_DIR}
)

add_library(RenderSystem_GLES ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
target_link_libraries(RenderSystem_GLES OgreMain ${OPENGLES_LIBRARIES} ${PLATFORM_LIBS})

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_GLES PROPERTIES
    COMPILE_DEFINITIONS OGRE_GLESPLUGIN_EXPORTS
  )
endif ()

if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_GLES ${Boost_LIBRARIES})
endif ()

ogre_config_plugin(RenderSystem_GLES)
if (OGRE_INSTALL_PLUGINS_HEADERS)
  install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/GLES)
  install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES/${PLATFORM_HEADER_INSTALL})
endif ()
