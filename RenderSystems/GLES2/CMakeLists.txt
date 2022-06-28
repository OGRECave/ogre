#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL ES 2.0 RenderSystem build

# GLSL-Optimizer
find_package(GLSLOptimizer QUIET)
macro_log_feature(GLSL_Optimizer_FOUND "GLSL Optimizer" "GLSL Optimizer" "http://github.com/aras-p/glsl-optimizer/")

# HLSL2GLSL
find_package(HLSL2GLSL QUIET)
macro_log_feature(HLSL2GLSL_FOUND "HLSL2GLSL" "HLSL2GLSL" "http://hlsl2glslfork.googlecode.com/")

file(GLOB HEADER_FILES "include/*.h")
list(APPEND HEADER_FILES
    ${PROJECT_BINARY_DIR}/include/OgreGLES2Exports.h
    ${PROJECT_BINARY_DIR}/include/OgreGLES2Config.h)
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

# Add system specific settings
if (APPLE)
  add_definitions(-DGLES_SILENCE_DEPRECATION)
  file(GLOB PLATFORM_HEADERS "include/EAGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EAGL/*.mm")

  include_directories(src/EAGL include/EAGL)
  set(PLATFORM_HEADER_INSTALL "APPLE")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/include)

add_library(RenderSystem_GLES2 ${OGRE_LIB_TYPE} ${HEADER_FILES} ${GLESW_HEADERS} ${KHR_HEADERS} ${PLATFORM_HEADERS} ${SOURCE_FILES} ${PLATFORM_SOURCES} ${GLSLES_FILES})
target_link_libraries(RenderSystem_GLES2 PUBLIC OgreMain ${CMAKE_DL_LIBS} PRIVATE OgreGLSupport)
target_include_directories(RenderSystem_GLES2 PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/RenderSystems/GLES2>)

if (OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER)
  target_link_libraries(RenderSystem_GLES2 ${GLSL_Optimizer_LIBRARIES})
endif ()

if (OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT)
  target_include_directories(RenderSystem_GLES2 PUBLIC ${HLSL2GLSL_INCLUDE_DIRS})
  target_link_libraries(RenderSystem_GLES2 ${HLSL2GLSL_LIBRARIES})
endif ()

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_GLES2 PROPERTIES
    COMPILE_DEFINITIONS OGRE_GLES2PLUGIN_EXPORTS
  )
endif ()

if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_GLES2 ${OGRE_THREAD_LIBRARIES})
endif ()

generate_export_header(RenderSystem_GLES2 
    EXPORT_MACRO_NAME _OgreGLES2Export
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreGLES2Exports.h)

ogre_config_plugin(RenderSystem_GLES2)

install(FILES ${HEADER_FILES} ${GLSUPPORT_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2)
install(FILES "include/GLES2/gl2ext.h" DESTINATION include/OGRE/RenderSystems/GLES2/GLES2)
install(FILES ${GLESW_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/${GLESW_HEADER_INSTALL})
install(FILES ${KHR_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/KHR)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES2/${PLATFORM_HEADER_INSTALL})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLES/include/ DESTINATION include/OGRE/RenderSystems/GLES2/GLSLES)
