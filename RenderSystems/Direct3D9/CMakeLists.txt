#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Direct3D9 RenderSystem build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if (MINGW)
  list(APPEND HEADER_FILES ${OGRE_SOURCE_DIR}/OgreMain/include/WIN32/OgreMinGWSupport.h)
  list(APPEND SOURCE_FILES ${OGRE_SOURCE_DIR}/OgreMain/src/WIN32/OgreMinGWSupport.cpp)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${DirectX9_INCLUDE_DIR})
link_directories(${DirectX9_LIBRARY_DIR})
add_definitions(-D_USRDLL)

ogre_add_library(RenderSystem_Direct3D9 ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(RenderSystem_Direct3D9
  OgreMain
  ${DirectX9_LIBRARIES}
)

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_Direct3D9 PROPERTIES
    COMPILE_DEFINITIONS OGRED3DENGINEDLL_EXPORTS
  )
endif ()
if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_Direct3D9 ${OGRE_THREAD_LIBRARIES})
endif ()

ogre_config_plugin(RenderSystem_Direct3D9)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/Direct3D9)
