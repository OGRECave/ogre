#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Bezier demo build

set(HEADER_FILES include/Bezier.h)
set(SOURCE_FILES src/Bezier.cpp)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Bezier.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Bezier WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES} )

target_link_libraries(Demo_Bezier ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Bezier)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Bezier ${SAMPLE_DEPENDENCIES})
endif ()

