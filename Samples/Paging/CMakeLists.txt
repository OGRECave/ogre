#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Grass demo build

set(SOURCE_FILES src/Paging.cpp)

include_directories(${CMAKE_SOURCE_DIR}/Components/Paging/include)

add_executable(Demo_Paging WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Paging ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_Paging_LIBRARIES})
ogre_config_sample(Demo_Paging)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Paging ${SAMPLE_DEPENDENCIES})
endif ()
