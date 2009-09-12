#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure CameraTrack demo build

set(SOURCE_FILES src/CameraTrack.cpp)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/CameraTrack.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

add_executable(Demo_CameraTrack WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_CameraTrack ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_CameraTrack)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_CameraTrack ${SAMPLE_DEPENDENCIES})
endif ()
