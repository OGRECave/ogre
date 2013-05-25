#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL 3+ RenderSystem build

set(HEADER_FILES
  include/OgreGL3PlusContext.h
  include/OgreGL3PlusDefaultHardwareBufferManager.h
  include/OgreGL3PlusDepthBuffer.h
  include/OgreGL3PlusFBOMultiRenderTarget.h
  include/OgreGL3PlusFBORenderTexture.h
  include/OgreGL3PlusFrameBufferObject.h
  include/OgreGL3PlusGpuProgram.h
  include/OgreGL3PlusGpuProgramManager.h
  include/OgreGL3PlusHardwareBufferManager.h
  include/OgreGL3PlusHardwareCounterBuffer.h
  include/OgreGL3PlusHardwareIndexBuffer.h
  include/OgreGL3PlusHardwareOcclusionQuery.h
  include/OgreGL3PlusHardwarePixelBuffer.h
  include/OgreGL3PlusHardwareUniformBuffer.h
  include/OgreGL3PlusHardwareVertexBuffer.h
  include/OgreGL3PlusPixelFormat.h
  include/OgreGL3PlusPlugin.h
  include/OgreGL3PlusPrerequisites.h
  include/OgreGL3PlusRenderSystem.h
  include/OgreGL3PlusRenderTexture.h
  include/OgreGL3PlusRenderToVertexBuffer.h
  include/OgreGL3PlusSupport.h
  include/OgreGL3PlusTexture.h
  include/OgreGL3PlusTextureManager.h
  include/OgreGL3PlusVertexArrayObject.h
)


set(SOURCE_FILES
  src/OgreGL3PlusContext.cpp
  src/OgreGL3PlusDefaultHardwareBufferManager.cpp
  src/OgreGL3PlusDepthBuffer.cpp
  src/OgreGL3PlusEngineDll.cpp
  src/OgreGL3PlusFBOMultiRenderTarget.cpp
  src/OgreGL3PlusFBORenderTexture.cpp
  src/OgreGL3PlusFrameBufferObject.cpp
  src/OgreGL3PlusGpuProgram.cpp
  src/OgreGL3PlusGpuProgramManager.cpp
  src/OgreGL3PlusHardwareBufferManager.cpp
  src/OgreGL3PlusHardwareCounterBuffer.cpp
  src/OgreGL3PlusHardwareIndexBuffer.cpp
  src/OgreGL3PlusHardwareOcclusionQuery.cpp
  src/OgreGL3PlusHardwarePixelBuffer.cpp
  src/OgreGL3PlusHardwareUniformBuffer.cpp
  src/OgreGL3PlusHardwareVertexBuffer.cpp
  src/OgreGL3PlusPixelFormat.cpp
  src/OgreGL3PlusPlugin.cpp
  src/OgreGL3PlusRenderSystem.cpp
  src/OgreGL3PlusRenderTexture.cpp
  src/OgreGL3PlusRenderToVertexBuffer.cpp
  src/OgreGL3PlusSupport.cpp
  src/OgreGL3PlusTexture.cpp
  src/OgreGL3PlusTextureManager.cpp
  src/OgreGL3PlusVertexArrayObject.cpp
  src/gl3w.cpp
)

set(GLSL_HEADERS
  src/GLSL/include/OgreGLSLExtSupport.h
  src/GLSL/include/OgreGLSLGpuProgram.h
  src/GLSL/include/OgreGLSLLinkProgram.h
  src/GLSL/include/OgreGLSLLinkProgramManager.h
  src/GLSL/include/OgreGLSLPreprocessor.h
  src/GLSL/include/OgreGLSLProgram.h
  src/GLSL/include/OgreGLSLProgramCommon.h
  src/GLSL/include/OgreGLSLProgramFactory.h
  src/GLSL/include/OgreGLSLProgramManagerCommon.h
  src/GLSL/include/OgreGLSLProgramPipeline.h
  src/GLSL/include/OgreGLSLProgramPipelineManager.h
  )

set(GLSL_SOURCE
  src/GLSL/src/OgreGLSLExtSupport.cpp
  src/GLSL/src/OgreGLSLGpuProgram.cpp
  src/GLSL/src/OgreGLSLLinkProgram.cpp
  src/GLSL/src/OgreGLSLLinkProgramManager.cpp
  src/GLSL/src/OgreGLSLPreprocessor.cpp
  src/GLSL/src/OgreGLSLProgram.cpp
  src/GLSL/src/OgreGLSLProgramCommon.cpp
  src/GLSL/src/OgreGLSLProgramFactory.cpp
  src/GLSL/src/OgreGLSLProgramManagerCommon.cpp
  src/GLSL/src/OgreGLSLProgramPipeline.cpp
  src/GLSL/src/OgreGLSLProgramPipelineManager.cpp)

# Add system specific settings
if (WIN32)
  set(PLATFORM_HEADERS
    include/win32/OgreWin32Context.h
    include/win32/OgreWin32GLSupport.h
    include/win32/OgreWin32Prerequisites.h
    include/win32/OgreWin32Window.h
    src/win32/OgreGL3PlusUtil.h
  )
  set(PLATFORM_SOURCES
    src/win32/OgreWin32GLSupport.cpp
    src/win32/OgreWin32Window.cpp
    src/win32/OgreWin32Context.cpp
  )
  include_directories(src/win32 include/win32)
  source_group(GLSL FILES ${GLSL_SOURCE} ${GLSL_HEADERS})
  set(PLATFORM_HEADER_INSTALL "")
elseif (APPLE)
  set(PLATFORM_HEADERS
    include/OSX/OgreOSXCocoaContext.h
    include/OSX/OgreOSXCocoaView.h
    include/OSX/OgreOSXCocoaWindow.h
    include/OSX/OgreOSXCocoaWindowDelegate.h
    include/OSX/OgreOSXGL3PlusSupport.h
    src/OSX/OgreGL3PlusUtil.h
  )
  set(PLATFORM_SOURCES
    src/OSX/OgreOSXCocoaContext.mm
    src/OSX/OgreOSXCocoaView.mm
    src/OSX/OgreOSXCocoaWindow.mm
    src/OSX/OgreOSXCocoaWindowDelegate.mm
    src/OSX/OgreOSXGL3PlusSupport.mm
  )
  include_directories(src/OSX include/OSX "${OGRE_SOURCE_DIR}/OgreMain/include/OSX")
  set(PLATFORM_LIBS ${Carbon_LIBRARY_FWK} ${Cocoa_LIBRARY_FWK})
  set(PLATFORM_HEADER_INSTALL "OSX")
elseif (UNIX)
  set(PLATFORM_HEADERS
    include/GLX/OgreGLXContext.h
    include/GLX/OgreGLXGLSupport.h
    include/GLX/OgreGLXUtils.h
    include/GLX/OgreGLXWindow.h
    src/GLX/OgreGL3PlusUtil.h
  )
  set(PLATFORM_SOURCES
    src/GLX/OgreGLXContext.cpp
    src/GLX/OgreGLXGLSupport.cpp
    src/GLX/OgreGLXWindow.cpp
  )
  include_directories(src/GLX include/GLX)
  set(PLATFORM_HEADER_INSTALL "GLX")
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
endif ()

set(HEADER_FILES ${HEADER_FILES} ${GLSL_HEADERS})

include_directories(
  BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include
  ${OPENGL_INCLUDE_DIR}
)

if (UNIX AND NOT APPLE)
  # Separate GLSupport and Window out to fix Unity builds
  ogre_add_library(RenderSystem_GL3Plus ${OGRE_LIB_TYPE} ${HEADER_FILES} ${GLSL_SOURCE} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES} ${SOURCE_FILES} SEPARATE "src/GLX/OgreGLXWindow.cpp" "src/GLX/OgreGLXGLSupport.cpp")
else()
  ogre_add_library(RenderSystem_GL3Plus ${OGRE_LIB_TYPE} ${HEADER_FILES} ${GLSL_SOURCE} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES} ${SOURCE_FILES})
endif()
target_link_libraries(RenderSystem_GL3Plus OgreMain ${OPENGL_LIBRARIES} ${PLATFORM_LIBS})

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_GL3Plus PROPERTIES
    COMPILE_DEFINITIONS OGRE_GLPLUGIN_EXPORTS
  )
endif ()
if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_GL3Plus ${OGRE_THREAD_LIBRARIES})
endif ()

ogre_config_framework(RenderSystem_GL3Plus)

ogre_config_plugin(RenderSystem_GL3Plus)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/GL3Plus)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GL3Plus/${PLATFORM_HEADER_INSTALL})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/OGRE/RenderSystems/GL3Plus)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/ DESTINATION include/OGRE/RenderSystems/GL3Plus)
