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
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/glew.c")

file(GLOB NVPARSE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/nvparse/*.cpp")
file(GLOB NVPARSE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/nvparse/*.h")

if(UNIX)
  # easier than porting to new flex  
  foreach(NVSRC ${NVPARSE_SOURCE})
    set_source_files_properties(${NVSRC} 
      PROPERTIES COMPILE_FLAGS "-Wno-unused-function -Wno-shadow -Wno-cast-qual -Wno-missing-declarations -Wno-undef")
  endforeach()
endif()

# Remove test app file
list(REMOVE_ITEM NVPARSE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/nvparse/ps1.0__test_main.cpp")

file(GLOB ATIFS_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/atifs/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/atifs/src/*.cpp")
file(GLOB ATIFS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/atifs/*.h")

file(GLOB GLSL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/*.h")
file(GLOB GLSL_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/src/*.cpp")

source_group(NVparse FILES ${NVPARSE_SOURCE} ${NVPARSE_HEADERS})
source_group(ATIFS FILES ${ATIFS_SOURCE} ${ATIFS_HEADERS})
source_group(GLSL FILES ${GLSL_SOURCE} ${GLSL_HEADERS})

if(OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/OgreGLStateCacheManager.cpp 
    PROPERTIES COMPILE_DEFINITIONS OGRE_ENABLE_STATE_CACHE)
endif()

list(APPEND HEADER_FILES
  ${GLSL_HEADERS}
  ${NVPARSE_HEADERS}
  ${ATIFS_HEADERS}
)

include_directories(${GLSUPPORT_INCLUDE_DIR})
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

add_definitions(-DGLEW_STATIC -DYY_NEVER_INTERACTIVE)
#Note that in the next row SOURCE_FILES are added last. This is to prevent compilation problems of unity build found on Windows Visual Studio. 
#In this situation any file added after the "glew.cpp" file, which belongs to the SOURCE_FILES package, does not compile
ogre_add_library_to_folder(RenderSystems RenderSystem_GL ${OGRE_LIB_TYPE} ${HEADER_FILES} ${GLSL_SOURCE} ${ATIFS_SOURCE} ${NVPARSE_SOURCE} ${SOURCE_FILES})
target_link_libraries(RenderSystem_GL OgreMain ${GLSUPPORT_LIBS} ${OPENGL_LIBRARIES})

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
        LINK_FLAGS "-framework Cocoa -framework Carbon -framework OpenGL -framework AGL")
endif ()

ogre_config_framework(RenderSystem_GL)

ogre_config_plugin(RenderSystem_GL)
install(FILES ${HEADER_FILES} ${GLSUPPORT_HEADERS} DESTINATION include/OGRE/RenderSystems/GL)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/GL DESTINATION include/OGRE/RenderSystems/GL)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/ DESTINATION include/OGRE/RenderSystems/GL)
