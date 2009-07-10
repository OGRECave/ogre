#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Water demo build

set(HEADER_FILES include/WaterMesh.h)
if (OGRE_BUILD_PLATFORM_IPHONE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Water.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/Water.mm COPYONLY)
  set(SOURCE_FILES src/Water.mm src/WaterMesh.cpp)
else ()
  set(SOURCE_FILES src/Water.cpp src/WaterMesh.cpp)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Water WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Water ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Water)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Water ${SAMPLE_DEPENDENCIES})
endif ()
