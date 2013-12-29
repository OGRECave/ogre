#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

set(HEADER_FILES
  include/PlayPenTestPlugin.h
  include/PlayPenTests.h)

set(SOURCE_FILES
  src/PlayPenTests.cpp
  src/PlayPenTestPlugin.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/Samples/Common/include)
include_directories(${OGRE_SOURCE_DIR}/Samples/Browser/include)
include_directories(${OGRE_SOURCE_DIR}/Tests/VisualTests/Common/include)

ogre_add_component_include_dir(Terrain)
ogre_add_component_include_dir(Paging)
ogre_add_component_include_dir(MeshLodGenerator)
ogre_add_component_include_dir(RTShaderSystem)
ogre_add_component_include_dir(Overlay)

add_library(PlayPenTests ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(PlayPenTests ${OGRE_LIBRARIES} ${OIS_LIBRARIES} OgreOverlay OgreRTShaderSystem)
ogre_config_sample_lib(PlayPenTests)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
  # Set the INSTALL_PATH so that Samples can be installed in the application package
  set_target_properties(PlayPenTests
    PROPERTIES BUILD_WITH_INSTALL_RPATH 1
    INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()
