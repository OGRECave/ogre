#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL ES 1.x RenderSystem build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

include_directories(src/StateCacheManager)
if(OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT)
  list(APPEND HEADER_FILES
      src/StateCacheManager/OgreGLESStateCacheManagerImp.h
  )
  list(APPEND SOURCE_FILES
      src/StateCacheManager/OgreGLESStateCacheManagerImp.cpp
  )
else()
  list(APPEND HEADER_FILES
      src/StateCacheManager/OgreGLESNullStateCacheManagerImp.h
  )
  list(APPEND SOURCE_FILES
      src/StateCacheManager/OgreGLESNullStateCacheManagerImp.cpp
  )
endif()

# Add system specific settings
if (WIN32)
  file(GLOB PLATFORM_HEADERS "include/EGL/WIN32/*.h" "include/EGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EGL/WIN32/*.cpp" "src/EGL/*.cpp")

  include_directories(src/EGL/WIN32 include/EGL/WIN32)
  link_directories(${OPENGLES_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "WIN32")
# Add system specific settings
elseif (APPLE)
  file(GLOB PLATFORM_HEADERS "include/EAGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EAGL/*.mm")

  include_directories(src/EAGL include/EAGL)
  link_directories(${OPENGLES_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "APPLE")
elseif(ANDROID)
  file(GLOB PLATFORM_HEADERS "include/EGL/Android/*.h" "include/EGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EGL/Android/*.cpp" "src/EGL/*.cpp")

  include_directories(src/EGL/Android include/EGL/Android)
  link_directories(${OPENGLES_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "ANDROID")
elseif (UNIX)
  file(GLOB PLATFORM_HEADERS "include/EGL/X11/*.h" "include/EGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EGL/X11/*.cpp" "src/EGL/*.cpp")

  include_directories(src/EGL/X11 include/EGL/X11)
  set(PLATFORM_HEADER_INSTALL "X11")
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
endif ()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
  ${CMAKE_CURRENT_SOURCE_DIR}/include/EAGL
  ${OPENGLES_INCLUDE_DIR}
)

ogre_add_library(RenderSystem_GLES ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
target_link_libraries(RenderSystem_GLES OgreMain ${OPENGLES_LIBRARIES} ${PLATFORM_LIBS})

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_GLES PROPERTIES
    COMPILE_DEFINITIONS OGRE_GLESPLUGIN_EXPORTS
  )
endif ()

if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_GLES ${OGRE_THREAD_LIBRARIES})
endif ()

ogre_config_plugin(RenderSystem_GLES)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/GLES)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES/${PLATFORM_HEADER_INSTALL})
