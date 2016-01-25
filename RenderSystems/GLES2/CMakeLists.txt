#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL ES 2.0 RenderSystem build

file(GLOB HEADER_FILES "include/*.h")
file(GLOB SOURCE_FILES "src/*.cpp")

file(GLOB GLSLES_FILES "src/GLSLES/include/*.h" "src/GLSLES/src/*.cpp")

list(REMOVE_ITEM GLSLES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/src/OgreGLSLESCgProgram.cpp")
list(REMOVE_ITEM GLSLES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/src/OgreGLSLESCgProgramFactory.cpp")

if(OGRE_CONFIG_ENABLE_GLES3_SUPPORT)
  set(GLESW_HEADER_INSTALL "GLES3")
  file(GLOB GLESW_HEADERS "include/GLES3/*.h")
  list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/gles3w.c")
  if (APPLE_IOS)
    set_source_files_properties(src/gles3w.c PROPERTIES COMPILE_FLAGS "-x objective-c")
  endif()
else()
  set(GLESW_HEADER_INSTALL "GLES2")
  file(GLOB GLESW_HEADERS "include/GLES2/*.h")
  list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/gles2w.c")
  if (APPLE_IOS)
    set_source_files_properties(src/gles2w.c PROPERTIES COMPILE_FLAGS "-x objective-c")
  endif()
endif()

file(GLOB KHR_HEADERS "include/KHR/*.h")

include_directories(src/StateCacheManager)
if(OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT)
  list(APPEND STATECACHE_HEADERS
      ${CMAKE_CURRENT_SOURCE_DIR}/src/StateCacheManager/OgreGLES2StateCacheManagerImp.h
      ${CMAKE_CURRENT_SOURCE_DIR}/src/StateCacheManager/OgreGLES2UniformCacheImp.h
  )
  list(APPEND SOURCE_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/src/StateCacheManager/OgreGLES2StateCacheManagerImp.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/src/StateCacheManager/OgreGLES2UniformCacheImp.cpp
  )
else()
  list(APPEND STATECACHE_HEADERS
      ${CMAKE_CURRENT_SOURCE_DIR}/src/StateCacheManager/OgreGLES2NullStateCacheManagerImp.h
      ${CMAKE_CURRENT_SOURCE_DIR}/src/StateCacheManager/OgreGLES2NullUniformCacheImp.h
  )
  list(APPEND SOURCE_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/src/StateCacheManager/OgreGLES2NullStateCacheManagerImp.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/src/StateCacheManager/OgreGLES2NullUniformCacheImp.cpp
  )
endif()

if (OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT)
  list(APPEND GLSLES_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/src/OgreGLSLESCgProgram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/src/OgreGLSLESCgProgramFactory.cpp
  )
endif (OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT)

if(NOT OPENGLES3_FOUND)
  set(OPENGLES3_INCLUDE_DIR "")
endif()

# Add system specific settings
if (OGRE_BUILD_PLATFORM_NACL)
  file(GLOB PLATFORM_HEADERS "include/NaCl/*.h")
  file(GLOB PLATFORM_SOURCES "src/NaCl/*.cpp")

  include_directories(src/NaCl include/NaCl)
  set(PLATFORM_HEADER_INSTALL "NaCl")
elseif (APPLE)
  file(GLOB PLATFORM_HEADERS "include/EAGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EAGL/*.mm")

  include_directories(src/EAGL include/EAGL)
  link_directories(${OPENGLES2_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "APPLE")
endif ()

include_directories(
  ${GLSUPPORT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/EAGL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/include
  ${OPENGLES3_INCLUDE_DIR} ${OPENGLES2_INCLUDE_DIR} ${EGL_INCLUDE_DIR}
)

ogre_add_library_to_folder(RenderSystems RenderSystem_GLES2 ${OGRE_LIB_TYPE} ${HEADER_FILES} ${STATECACHE_HEADERS} ${GLESW_HEADERS} ${KHR_HEADERS} ${PLATFORM_HEADERS} ${SOURCE_FILES} ${PLATFORM_SOURCES} SEPARATE ${GLSLES_FILES})
target_link_libraries(RenderSystem_GLES2 OgreMain ${OPENGLES3_LIBRARIES} ${OPENGLES2_LIBRARIES} ${GLSUPPORT_LIBS})

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_GLES2 PROPERTIES
    COMPILE_DEFINITIONS OGRE_GLES2PLUGIN_EXPORTS
  )
endif ()

if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_GLES2 ${OGRE_THREAD_LIBRARIES})
endif ()

ogre_config_plugin(RenderSystem_GLES2)

install(FILES ${HEADER_FILES} ${GLSUPPORT_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2)
install(FILES ${GLESW_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/${GLESW_HEADER_INSTALL})
install(FILES ${KHR_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/KHR)
install(FILES ${STATECACHE_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/StateCacheManager)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/include/ DESTINATION include/OGRE/RenderSystems/GLES2/GLSLES)
