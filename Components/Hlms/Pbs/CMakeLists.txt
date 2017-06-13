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

file(
	GLOB HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Cubemaps/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/InstantRadiosity/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Common/include/*.h"
)
file(
	GLOB SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Cubemaps/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/InstantRadiosity/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Common/src/*.cpp"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/include)

if( OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS )
	ogre_add_component_include_dir( PlanarReflections )
endif()

ogre_add_library(OgreHlmsPbs ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreHlmsPbs PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreHlmsPbs OgreMain)

if( OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS )
	target_link_libraries( OgreHlmsPbs OgrePlanarReflections )
endif()

ogre_config_framework(OgreHlmsPbs)
ogre_config_component(OgreHlmsPbs)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Hlms/Pbs
)
