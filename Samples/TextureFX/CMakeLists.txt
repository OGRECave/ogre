#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure TextureFX demo build

set(HEADER_FILES include/TextureFX.h)
set(SOURCE_FILES src/TextureFX.cpp)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/TextureFX.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_TextureFX WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_TextureFX ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_TextureFX)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_TextureFX ${SAMPLE_DEPENDENCIES})
endif ()
