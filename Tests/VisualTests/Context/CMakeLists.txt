#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

set(HEADER_FILES
    include/TestContext.h
	../Common/include/ImageValidator.h
	../Common/include/TestBatch.h
	../Common/include/TestResultWriter.h
	../Common/include/SimpleResultWriter.h
	../Common/include/HTMLWriter.h
	../Common/include/VisualTest.h
	../Common/include/TinyHTML.h
	)

set(SOURCE_FILES
    src/TestContext.cpp)

# we also need the file system stuff from the Sample Browser
if (WIN32)
  list(APPEND SOURCE_FILES ${OGRE_SOURCE_DIR}/Samples/Browser/src/FileSystemLayerImpl_WIN32.cpp)
elseif (OGRE_BUILD_PLATFORM_APPLE_IOS)
  list(APPEND SOURCE_FILES ${OGRE_SOURCE_DIR}/Samples/Browser/src/FileSystemLayerImpl_iOS.mm)
elseif (APPLE)
  list(APPEND SOURCE_FILES ${OGRE_SOURCE_DIR}/Samples/Browser/src/FileSystemLayerImpl_OSX.cpp)
elseif (UNIX)
  list(APPEND SOURCE_FILES ${OGRE_SOURCE_DIR}/Samples/Browser/src/FileSystemLayerImpl_Unix.cpp)
else ()
  list(APPEND SOURCE_FILES ${OGRE_SOURCE_DIR}/Samples/Browser/src/FileSystemLayerImpl_Default.cpp)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/Samples/Common/include)
include_directories(${OGRE_SOURCE_DIR}/Samples/Browser/include)
include_directories(${OGRE_SOURCE_DIR}/Tests/VisualTests/Common/include)

ogre_add_component_include_dir(Terrain)
ogre_add_component_include_dir(Paging)
ogre_add_component_include_dir(RTShaderSystem)

ogre_add_executable(TestContext ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(TestContext ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES} ${SAMPLE_LIBRARIES})
ogre_config_common(TestContext)
