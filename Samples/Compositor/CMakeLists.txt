#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Compositor demo build

set(HEADER_FILES 
  include/Compositor.h
  include/HelperLogics.h
)

set(SOURCE_FILES 
  src/Compositor.cpp
  src/HelperLogics.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_Compositor ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_Compositor ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(Sample_Compositor)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_Compositor
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

if (OGRE_STATIC)
  add_dependencies(SampleBrowser Sample_Compositor)
else ()
  add_dependencies(Sample_Compositor SampleBrowser)
endif (OGRE_STATIC)
