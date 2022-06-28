#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL Support build

# Add platform specific settings

if(EGL_FOUND)
  cmake_dependent_option(OGRE_GLSUPPORT_USE_EGL "use EGL for GL Context Creation instead of GLX/ WGL" TRUE "NOT WIN32" FALSE)
endif()

if(ANDROID)
  file(GLOB PLATFORM_HEADERS "include/EGL/Android/*.h" "include/EGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EGL/Android/*.cpp" "src/EGL/*.cpp")

  set(NATIVE_INCLUDES 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL/Android)
elseif (EMSCRIPTEN)
  file(GLOB PLATFORM_HEADERS "include/EGL/Emscripten/*.h" "include/EGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EGL/Emscripten/*.cpp" "src/EGL/*.cpp")

  set(NATIVE_INCLUDES 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL/Emscripten)
elseif (WIN32)
  if(OGRE_GLSUPPORT_USE_EGL)
      file(GLOB PLATFORM_HEADERS "include/EGL/WIN32/*.h" "include/EGL/*.h")
      file(GLOB PLATFORM_SOURCES "src/EGL/WIN32/*.cpp" "src/EGL/*.cpp")
      
      set(NATIVE_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
        ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL/WIN32)
  else()
    file(GLOB PLATFORM_HEADERS "include/win32/*.h")
    file(GLOB PLATFORM_SOURCES "src/win32/*.cpp")

    set(NATIVE_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/win32)
        
    set(PLATFORM_LIBS ${OPENGL_gl_LIBRARY})
  endif()
elseif(APPLE_IOS)
  # only works with GLES2 - no shared components here.
elseif (APPLE)
  file(GLOB PLATFORM_HEADERS "include/OSX/*.h")
  file(GLOB PLATFORM_SOURCES "src/OSX/*.mm")

  # ignore usage of deprecated OSX API for now
  add_definitions(-Wno-deprecated-declarations)

  set_source_files_properties("src/OSX/OgreOSXGL3PlusSupport.mm" 
    PROPERTIES COMPILE_FLAGS "-Wno-cast-qual")

  set(NATIVE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/OSX 
    ${PROJECT_SOURCE_DIR}/OgreMain/include/OSX)
  set(PLATFORM_LIBS "-framework AppKit" ${OPENGL_gl_LIBRARY})
elseif (UNIX)
  if(OGRE_GLSUPPORT_USE_EGL)
    file(GLOB PLATFORM_HEADERS "include/EGL/X11/*.h" "include/EGL/*.h")
    file(GLOB PLATFORM_SOURCES "src/EGL/X11/*.cpp" "src/EGL/*.cpp" "src/X11/*.cpp")
    
    set(NATIVE_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
        ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL/X11)
    set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB} ${EGL_LIBRARIES})
  else()
    file(GLOB PLATFORM_HEADERS "include/GLX/*.h")
    file(GLOB PLATFORM_SOURCES "src/GLX/*.cpp" "src/X11/*.cpp")

    set(NATIVE_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/GLX
        ${OPENGL_INCLUDE_DIR})
    
    set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB} ${OPENGL_glx_LIBRARY})
  endif()
  list(APPEND NATIVE_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/src/X11/")
endif ()

file(GLOB GLSUPPORT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
set(GLSUPPORT_HEADERS ${GLSUPPORT_HEADERS} ${PROJECT_BINARY_DIR}/include/OgreGLSupportPrerequisites.h PARENT_SCOPE)

file(GLOB GLSL_SOURCES src/GLSL/*.cpp)

if(OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/OgreGLUniformCache.cpp 
    PROPERTIES COMPILE_DEFINITIONS OGRE_ENABLE_STATE_CACHE)
endif()

add_library(OgreGLSupport STATIC 
    ${GLSUPPORT_HEADERS} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES} ${GLSL_SOURCES} 
    src/OgreGLRenderSystemCommon.cpp 
    src/OgreGLTextureCommon.cpp
    src/OgreGLDepthBufferCommon.cpp
    src/OgreGLRenderTexture.cpp
    src/OgreGLHardwarePixelBufferCommon.cpp
    src/OgreGLUniformCache.cpp
    src/OgreGLVertexArrayObject.cpp
    src/OgreGLWindow.cpp
)
target_link_libraries(OgreGLSupport PUBLIC OgreMain PRIVATE ${PLATFORM_LIBS})
target_include_directories(OgreGLSupport PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/GLSL>"
    PRIVATE "$<BUILD_INTERFACE:${NATIVE_INCLUDES}>")

set_property(TARGET OgreGLSupport PROPERTY POSITION_INDEPENDENT_CODE ON)
generate_export_header(OgreGLSupport 
    EXPORT_MACRO_NAME _OgreGLExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreGLSupportPrerequisites.h)

if(OGRE_STATIC)
  ogre_config_lib(OgreGLSupport TRUE)
endif()
