#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure EnvMapping demo build

set(HEADER_FILES include/EnvMapping.h)
set(SOURCE_FILES src/EnvMapping.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_EnvMapping WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_EnvMapping ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_EnvMapping)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_EnvMapping ${SAMPLE_DEPENDENCIES})
endif ()
