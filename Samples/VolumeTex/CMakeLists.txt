#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure VolumeTex demo build

set(HEADER_FILES 
  include/VolumeRenderable.h
  include/Julia.h
  include/ThingRenderable.h
)

set(SOURCE_FILES 
  src/VolumeTex.cpp
  src/ThingRenderable.cpp
  src/VolumeRenderable.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_VolumeTex ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_VolumeTex ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(Sample_VolumeTex)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_VolumeTex
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

add_dependencies(SampleBrowser Sample_VolumeTex)