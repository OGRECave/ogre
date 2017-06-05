#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------


PROJECT(OgrePlanarReflections)

file(
	GLOB HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)
file(
	GLOB SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_definitions( -DOgrePlanarReflections_EXPORTS )

ogre_add_library(OgrePlanarReflections ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgrePlanarReflections PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgrePlanarReflections OgreMain)

ogre_config_framework(OgrePlanarReflections)
ogre_config_component(OgrePlanarReflections)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/PlanarReflections
)
