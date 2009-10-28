#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure PlayPen build

set(HEADER_FILES 
	include/AnimationBlender.h 
	include/StaticPluginLoader.h)
set(SOURCE_FILES 
    src/PlayPen.cpp
    src/AnimationBlender.cpp
    src/WindowEmbedding.cpp)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/PlayPen.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

if (APPLE)
  set(OGRE_LIBRARIES ${OGRE_LIBRARIES} IOKit)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(
	${CMAKE_SOURCE_DIR}/Samples/Common/include 
	${CMAKE_SOURCE_DIR}/Components/Terrain/include)
add_executable(PlayPen WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES} )
target_link_libraries(PlayPen ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_Terrain_LIBRARIES})
ogre_config_sample_exe(PlayPen)

add_dependencies(PlayPen ${TEST_DEPENDENCIES})

