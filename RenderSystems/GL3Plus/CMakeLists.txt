#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL 3+ RenderSystem build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

file(GLOB GLSL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/*.h")
file(GLOB GLSL_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/src/*.cpp")

# Add system specific settings
if (WIN32)
  file(GLOB PLATFORM_HEADERS "include/Win32/*.h" "src/Win32/*.h")
  file(GLOB PLATFORM_SOURCES "src/Win32/*.cpp")

  include_directories(src/win32 include/win32)
  source_group(GLSL FILES ${GLSL_SOURCE} ${GLSL_HEADERS})
  set(PLATFORM_HEADER_INSTALL "")
elseif (APPLE)
  file(GLOB PLATFORM_HEADERS "include/OSX/*.h" "src/OSX/*.h")
  file(GLOB PLATFORM_SOURCES "src/OSX/*.mm")

  include_directories(src/OSX include/OSX "${OGRE_SOURCE_DIR}/OgreMain/include/OSX")
  set(PLATFORM_LIBS ${Cocoa_LIBRARY_FWK})
  set(PLATFORM_HEADER_INSTALL "OSX")
elseif (UNIX)
  file(GLOB PLATFORM_HEADERS "include/GLX/*.h" "src/GLX/*.h")
  file(GLOB PLATFORM_SOURCES "src/GLX/*.cpp")

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
)

if(NOT APPLE)
  include_directories(
    ${OPENGL_INCLUDE_DIR}
  )
endif()

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

if (APPLE)
    set_target_properties(RenderSystem_GL3Plus PROPERTIES
        LINK_FLAGS "-framework Cocoa -framework OpenGL")
endif ()

ogre_config_framework(RenderSystem_GL3Plus)

ogre_config_plugin(RenderSystem_GL3Plus)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/GL3Plus)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GL3Plus/${PLATFORM_HEADER_INSTALL})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/OGRE/RenderSystems/GL3Plus)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/ DESTINATION include/OGRE/RenderSystems/GL3Plus)
