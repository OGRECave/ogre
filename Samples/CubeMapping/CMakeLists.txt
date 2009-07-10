#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure CubeMapping demo build

set(HEADER_FILES include/CubeMapping.h)
if (OGRE_BUILD_PLATFORM_IPHONE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/CubeMapping.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/CubeMapping.mm COPYONLY)
  set(SOURCE_FILES src/CubeMapping.mm)
else ()
  set(SOURCE_FILES src/CubeMapping.cpp)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_CubeMapping WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_CubeMapping ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_CubeMapping)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_CubeMapping ${SAMPLE_DEPENDENCIES})
endif ()
