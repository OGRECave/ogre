#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL RenderSystem build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

file(GLOB NVPARSE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/nvparse/*.cpp")
file(GLOB NVPARSE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/nvparse/*.h")

# Remove test app file
list(REMOVE_ITEM NVPARSE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/nvparse/ps1.0__test_main.cpp")

file(GLOB ATIFS_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/atifs/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/atifs/src/*.cpp")
file(GLOB ATIFS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/atifs/*.h")

file(GLOB GLSL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/*.h")
file(GLOB GLSL_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/src/*.cpp")

source_group(NVparse FILES ${NVPARSE_SOURCE} ${NVPARSE_HEADERS})
source_group(ATIFS FILES ${ATIFS_SOURCE} ${ATIFS_HEADERS})
source_group(GLSL FILES ${GLSL_SOURCE} ${GLSL_HEADERS})

include_directories(src/StateCacheManager)
if(OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT)
  list(APPEND HEADER_FILES
      src/StateCacheManager/OgreGLStateCacheManagerImp.h
      src/StateCacheManager/OgreGLUniformCacheImp.h
  )
  list(APPEND SOURCE_FILES
      src/StateCacheManager/OgreGLStateCacheManagerImp.cpp
      src/StateCacheManager/OgreGLUniformCacheImp.cpp
  )
else()
  list(APPEND HEADER_FILES
      src/StateCacheManager/OgreGLNullStateCacheManagerImp.h
      src/StateCacheManager/OgreGLNullUniformCacheImp.h
  )
  list(APPEND SOURCE_FILES
      src/StateCacheManager/OgreGLNullStateCacheManagerImp.cpp
      src/StateCacheManager/OgreGLNullUniformCacheImp.cpp
  )
endif()

# Add system specific settings
if (WIN32)
  file(GLOB PLATFORM_HEADERS "include/Win32/*.h" "src/Win32/*.h")
  file(GLOB PLATFORM_SOURCES "src/Win32/*.cpp")

  include_directories(src/nvparse/winheaders include/Win32 src/Win32)
  set(PLATFORM_HEADER_INSTALL "")
elseif (APPLE)
  file(GLOB PLATFORM_HEADERS "include/OSX/*.h" "src/OSX/*.h")
  file(GLOB PLATFORM_SOURCES "src/OSX/*.cpp" "src/OSX/*.mm")

  include_directories(src/OSX include/OSX "${OGRE_SOURCE_DIR}/OgreMain/include/OSX")
  set(PLATFORM_HEADER_INSTALL "OSX")
elseif (UNIX)
  file(GLOB PLATFORM_HEADERS "include/GLX/*.h" "src/GLX/*.h")
  file(GLOB PLATFORM_SOURCES "src/GLX/*.cpp")

  include_directories(src/GLX include/GLX)
  set(PLATFORM_HEADER_INSTALL "GLX")
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
endif ()

list(APPEND HEADER_FILES
  ${GLSL_HEADERS}
  ${NVPARSE_HEADERS}
  ${ATIFS_HEADERS}
)

include_directories(
  BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/atifs/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/nvparse
  ${OGRE_SOURCE_DIR}/OgreMain/include/Threading
)

if(NOT APPLE)
  include_directories(
    ${OPENGL_INCLUDE_DIR}
  )
endif()

#Note that in the next row SOURCE_FILES are added last. This is to prevent compilation problems of unity build found on Windows Visual Studio. 
#In this situation any file added after the "glew.cpp" file, which belongs to the SOURCE_FILES package, does not compile
ogre_add_library(RenderSystem_GL ${OGRE_LIB_TYPE} ${HEADER_FILES} ${GLSL_SOURCE} ${ATIFS_SOURCE} ${NVPARSE_SOURCE} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES} ${SOURCE_FILES})
target_link_libraries(RenderSystem_GL OgreMain ${OPENGL_LIBRARIES} ${PLATFORM_LIBS})

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_GL PROPERTIES
    COMPILE_DEFINITIONS OGRE_GLPLUGIN_EXPORTS
  )
endif ()
if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_GL ${OGRE_THREAD_LIBRARIES})
endif ()

if (APPLE)
    set_target_properties(RenderSystem_GL PROPERTIES
        LINK_FLAGS "-framework Cocoa -framework Carbon -framework OpenGL")
endif ()

ogre_config_framework(RenderSystem_GL)

ogre_config_plugin(RenderSystem_GL)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/GL)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GL/${PLATFORM_HEADER_INSTALL})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/GL DESTINATION include/OGRE/RenderSystems/GL)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/ DESTINATION include/OGRE/RenderSystems/GL)
