#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Ogre HLMS Physically Based Shading for OpenGL ES 2.0 build

PROJECT(OgreHlmsPbsEs2)

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

ogre_add_library(OgreHlmsPbsEs2 ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreHlmsPbsEs2 PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreHlmsPbsEs2 OgreMain)

ogre_config_framework(OgreHlmsPbsEs2)
ogre_config_component(OgreHlmsPbsEs2)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Hlms/PBS_ES2
)
