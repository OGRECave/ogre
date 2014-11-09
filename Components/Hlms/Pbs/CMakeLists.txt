#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Ogre HLMS Physically Based Shading for OpenGL 3.3+ and D3D11 build

PROJECT(OgreHlmsPbs)

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

ogre_add_library(OgreHlmsPbs ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreHlmsPbs PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreHlmsPbs OgreMain)

ogre_config_framework(OgreHlmsPbs)
ogre_config_component(OgreHlmsPbs)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Hlms/Pbs
)
