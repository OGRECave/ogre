#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL ES 2.0 RenderSystem build

set(HEADER_FILES
  include/OgreGLES2Context.h
  include/OgreGLES2DefaultHardwareBufferManager.h
  include/OgreGLES2FBOMultiRenderTarget.h
  include/OgreGLES2FBORenderTexture.h
  include/OgreGLES2FrameBufferObject.h
  include/OgreGLES2GpuProgram.h
  include/OgreGLES2GpuProgramManager.h
  include/OgreGLES2HardwareBufferManager.h
  include/OgreGLES2HardwareIndexBuffer.h
  include/OgreGLES2HardwarePixelBuffer.h
  include/OgreGLES2HardwareVertexBuffer.h
  include/OgreGLES2PixelFormat.h
  include/OgreGLES2Plugin.h
  include/OgreGLES2Prerequisites.h
  include/OgreGLES2RenderSystem.h
  include/OgreGLES2RenderTexture.h
  include/OgreGLES2Support.h
  include/OgreGLES2Texture.h
  include/OgreGLES2TextureManager.h
)

set(SOURCE_FILES
  src/OgreGLES2Context.cpp
  src/OgreGLES2DefaultHardwareBufferManager.cpp
  src/OgreGLES2EngineDll.cpp
  src/OgreGLES2FBOMultiRenderTarget.cpp
  src/OgreGLES2FBORenderTexture.cpp
  src/OgreGLES2FrameBufferObject.cpp
  src/OgreGLES2GpuProgram.cpp
  src/OgreGLES2GpuProgramManager.cpp
  src/OgreGLES2HardwareBufferManager.cpp
  src/OgreGLES2HardwareIndexBuffer.cpp
  src/OgreGLES2HardwarePixelBuffer.cpp
  src/OgreGLES2HardwareVertexBuffer.cpp
  src/OgreGLES2PixelFormat.cpp
  src/OgreGLES2Plugin.cpp
  src/OgreGLES2RenderSystem.cpp
  src/OgreGLES2RenderTexture.cpp
  src/OgreGLES2Support.cpp
  src/OgreGLES2Texture.cpp
  src/OgreGLES2TextureManager.cpp
)

set(GLSLES_FILES
  src/GLSLES/include/OgreGLSLESExtSupport.h
  src/GLSLES/include/OgreGLSLESGpuProgram.h
  src/GLSLES/include/OgreGLSLESLinkProgram.h
  src/GLSLES/include/OgreGLSLESLinkProgramManager.h
  src/GLSLES/include/OgreGLSLESPreprocessor.h
  src/GLSLES/include/OgreGLSLESProgramFactory.h
  src/GLSLES/include/OgreGLSLESProgram.h
  src/GLSLES/src/OgreGLSLESExtSupport.cpp
  src/GLSLES/src/OgreGLSLESGpuProgram.cpp
  src/GLSLES/src/OgreGLSLESLinkProgram.cpp
  src/GLSLES/src/OgreGLSLESLinkProgramManager.cpp
  src/GLSLES/src/OgreGLSLESPreprocessor.cpp
  src/GLSLES/src/OgreGLSLESProgram.cpp
  src/GLSLES/src/OgreGLSLESProgramFactory.cpp
)
# Add system specific settings
if (WIN32)
  set(PLATFORM_HEADERS 
    include/EGL/WIN32/OgreWin32EGLContext.h
    include/EGL/WIN32/OgreWin32EGLSupport.h
    include/EGL/WIN32/OgreWin32EGLRenderTexture.h
    include/EGL/WIN32/OgreWin32EGLWindow.h
    include/EGL/WIN32/OgreGLES2Util.h
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
  link_directories(${OPENGLES2_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "WIN32")
elseif (APPLE)
  set(PLATFORM_HEADERS 
    include/EAGL/OgreEAGLES2Context.h
    include/EAGL/OgreEAGL2Support.h
    include/EAGL/OgreEAGL2Window.h
    include/EAGL/OgreGLES2Util.h
  )
  set(PLATFORM_SOURCES
    src/EAGL/OgreEAGL2Support.mm
    src/EAGL/OgreEAGL2Window.mm
    src/EAGL/OgreEAGLES2Context.mm
  )
  include_directories(src/EAGL include/EAGL)
  link_directories(${OPENGLES2_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "APPLE")
elseif (UNIX)
  set(PLATFORM_HEADERS
    include/EGL/GTK/OgreGtkEGLContext.h
    include/EGL/GTK/OgreGtkEGLSupport.h
    include/EGL/GTK/OgreGtkEGLRenderTexture.h
    include/EGL/GTK/OgreGtkEGLWindow.h
    include/EGL/GTK/OgreGLES2Util.h
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

# Add support for the RT Shader System
add_definitions(-DUSE_RTSHADER_SYSTEM)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
  ${CMAKE_CURRENT_SOURCE_DIR}/include/EAGL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/include
  ${CMAKE_SOURCE_DIR}/Components/RTShaderSystem/include
  ${OPENGLES2_INCLUDE_DIR}
)

add_library(RenderSystem_GLES2 ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${GLSLES_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
target_link_libraries(RenderSystem_GLES2 OgreMain ${OPENGLES2_LIBRARIES} ${PLATFORM_LIBS})

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_GLES2 PROPERTIES
    COMPILE_DEFINITIONS OGRE_GLES2PLUGIN_EXPORTS
  )
endif ()

if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_GLES2 ${Boost_LIBRARIES})
endif ()

ogre_config_plugin(RenderSystem_GLES2)
if (OGRE_INSTALL_PLUGINS_HEADERS)
  install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/GLES2)
  install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/${PLATFORM_HEADER_INSTALL})
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/include/ DESTINATION include/OGRE/RenderSystems/GLES2/${PLATFORM_HEADER_INSTALL})
endif ()
