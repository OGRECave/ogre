#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

set(HEADER_FILES
    include/StencilShadowTest.h
    include/ParticleTest.h
    include/TextureBlitTest.h
    include/VTestPlugin.h)

set(SOURCE_FILES
    src/StencilShadowTest.cpp
    src/TextureEffectsTest.cpp
    src/TextureBlitTest.cpp
    src/CubeMappingTest.cpp
    src/TransparencyTest.cpp
    src/ParticleTest.cpp
    src/VTestPlugin.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/Samples/Common/include)
include_directories(${PROJECT_SOURCE_DIR}/Samples/Browser/include)
include_directories(${PROJECT_SOURCE_DIR}/Tests/VisualTests/Common/include)

add_library(VTests ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} )
target_link_libraries(VTests ${OGRE_LIBRARIES} OgreBites OgreOverlay OgreRTShaderSystem)
ogre_config_sample_lib(VTests)

if (APPLE AND NOT APPLE_IOS)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(VTests
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

