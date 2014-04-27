#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Ogre HLMS User Interface (2D) for OpenGL ES 2.0 build

PROJECT(OgreHlmsGui2DMobile)

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

ogre_add_library(OgreHlmsGui2DMobile ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreHlmsGui2DMobile PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreHlmsGui2DMobile OgreMain)

ogre_config_framework(OgreHlmsGui2DMobile)
ogre_config_component(OgreHlmsGui2DMobile)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Hlms/UI2D_ES2
)
