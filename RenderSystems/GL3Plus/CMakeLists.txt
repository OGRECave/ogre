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

file(GLOB GLSL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/GLSL/*.h")
file(GLOB GLSL_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/*.cpp")

set(HEADER_FILES ${HEADER_FILES} ${GLSL_HEADERS})

include_directories(${OGRE_GLSUPPORT_INCLUDES})
include_directories(
  BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/GLSL
)

if(NOT APPLE)
  include_directories(
    ${OPENGL_INCLUDE_DIR}
  )
endif()

ogre_add_library_to_folder(RenderSystems RenderSystem_GL3Plus ${OGRE_LIB_TYPE} ${HEADER_FILES} ${GLSL_SOURCE} ${SOURCE_FILES})
target_link_libraries(RenderSystem_GL3Plus OgreMain ${OPENGL_LIBRARIES} ${GLSUPPORT_LIBS})

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
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/GLSL/ DESTINATION include/OGRE/RenderSystems/GL3Plus)
