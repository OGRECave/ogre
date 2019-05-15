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
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/gl3w.c")

file(GLOB GLSL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/*.h")
file(GLOB GLSL_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/src/*.cpp")

add_definitions(${OGRE_VISIBILITY_FLAGS})

if(OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/OgreGL3PlusStateCacheManager.cpp
    PROPERTIES COMPILE_DEFINITIONS OGRE_ENABLE_STATE_CACHE)
endif()
add_library(RenderSystem_GL3Plus ${OGRE_LIB_TYPE} ${HEADER_FILES} ${GLSL_HEADERS} ${GLSL_SOURCE} ${SOURCE_FILES})
target_link_libraries(RenderSystem_GL3Plus OgreMain OgreGLSupport ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS})
target_include_directories(RenderSystem_GL3Plus PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/RenderSystems/GL3Plus>
    PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include>")

if (APPLE)
    set_target_properties(RenderSystem_GL3Plus PROPERTIES
        LINK_FLAGS "-framework Cocoa -framework OpenGL")
else()
    target_include_directories(RenderSystem_GL3Plus PUBLIC ${OPENGL_INCLUDE_DIR})
endif ()

ogre_config_framework(RenderSystem_GL3Plus)
ogre_config_plugin(RenderSystem_GL3Plus)

install(FILES ${HEADER_FILES} ${GLSUPPORT_HEADERS} DESTINATION include/OGRE/RenderSystems/GL3Plus)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/GL DESTINATION include/OGRE/RenderSystems/GL3Plus)
