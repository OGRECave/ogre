#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Visual Test build

set(HEADER_FILES 
    include/VisualTest.h
    include/VisualTestPlugin.h
    include/PlayPenTests.h
    include/TestBatch.h
    include/tinyhtml.h
    include/TestResults.h
    include/ImageCompare.h)

set(SOURCE_FILES 
    src/PlayPenTests.cpp
    src/VisualTestPlugin.cpp)

set(HEADER_FILES_CONTEXT
    include/TestContext.h)

set(SOURCE_FILES_CONTEXT
    src/TestContext.cpp)

# we also need the file system stuff from the Sample Browser
if (WIN32)
  list(APPEND SOURCE_FILES_CONTEXT ../../Samples/Browser/src/FileSystemLayerImpl_WIN32.cpp)
elseif (OGRE_BUILD_PLATFORM_APPLE_IOS)
  list(APPEND SOURCE_FILES_CONTEXT ../../Samples/Browser/src/FileSystemLayerImpl_iPhone.mm)
elseif (APPLE)
  list(APPEND SOURCE_FILES_CONTEXT ../../Samples/Browser/src/FileSystemLayerImpl_OSX.cpp)
elseif (UNIX)
  list(APPEND SOURCE_FILES_CONTEXT ../../Samples/Browser/src/FileSystemLayerImpl_Unix.cpp)
else ()
  list(APPEND SOURCE_FILES_CONTEXT ../../Samples/Browser/src/FileSystemLayerImpl_Default.cpp)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/Samples/Common/include)
include_directories(${OGRE_SOURCE_DIR}/Samples/Browser/include)

ogre_add_component_include_dir(Terrain)
ogre_add_component_include_dir(Paging)
ogre_add_component_include_dir(RTShaderSystem)

ogre_add_executable(TestContext ${HEADER_FILES_CONTEXT} ${SOURCE_FILES_CONTEXT})
target_link_libraries(TestContext ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES} ${SAMPLE_LIBRARIES})
ogre_config_common(TestContext)

add_library(VisualTests ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} )
target_link_libraries(VisualTests ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(VisualTests)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(VisualTests
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()
