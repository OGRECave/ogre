#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Paging demo build

set(SOURCE_FILES src/Paging.cpp)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Paging.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

include_directories(${OGRE_SOURCE_DIR}/Components/Paging/include)

add_executable(Demo_Paging WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Paging ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_Paging_LIBRARIES})
ogre_config_sample_exe(Demo_Paging)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Paging ${SAMPLE_DEPENDENCIES})
endif ()
