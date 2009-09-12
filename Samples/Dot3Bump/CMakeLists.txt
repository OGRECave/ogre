#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Dot3Bump demo build

set(SOURCE_FILES src/Dot3Bump.cpp)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Dot3Bump.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

add_executable(Demo_Dot3Bump WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Dot3Bump ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Dot3Bump)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Dot3Bump ${SAMPLE_DEPENDENCIES})
endif ()
