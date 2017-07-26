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
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/glesw.c")

file(GLOB GLSLES_FILES "src/GLSLES/include/*.h" "src/GLSLES/src/*.cpp")

list(REMOVE_ITEM GLSLES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/src/OgreGLSLESCgProgram.cpp")
list(REMOVE_ITEM GLSLES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/src/OgreGLSLESCgProgramFactory.cpp")

set(GLESW_HEADER_INSTALL "GLES3")
file(GLOB GLESW_HEADERS "include/GLES3/*.h")
file(GLOB KHR_HEADERS "include/KHR/*.h")

if(OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/OgreGLES2StateCacheManager.cpp 
    PROPERTIES COMPILE_DEFINITIONS OGRE_ENABLE_STATE_CACHE)
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

ogre_add_library_to_folder(RenderSystems RenderSystem_GLES2 ${OGRE_LIB_TYPE} ${HEADER_FILES} ${GLESW_HEADERS} ${KHR_HEADERS} ${PLATFORM_HEADERS} ${SOURCE_FILES} ${PLATFORM_SOURCES} SEPARATE ${GLSLES_FILES})
target_link_libraries(RenderSystem_GLES2 OgreMain ${GLSUPPORT_LIBS} ${OPENGLES3_LIBRARIES} ${OPENGLES2_LIBRARIES})

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
install(FILES "include/GLES2/gl2ext.h" DESTINATION include/OGRE/RenderSystems/GLES2/GLES2)
install(FILES ${GLESW_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/${GLESW_HEADER_INSTALL})
install(FILES ${KHR_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/KHR)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/${PLATFORM_HEADER_INSTALL})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/include/ DESTINATION include/OGRE/RenderSystems/GLES2/GLSLES)
