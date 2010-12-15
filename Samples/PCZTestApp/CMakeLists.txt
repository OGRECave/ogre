#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure PCZTestApp demo build

set(HEADER_FILES include/PCZTestApp.h include/RoomObject.h)
set(SOURCE_FILES src/PCZTestApp.cpp src/RoomObject.cpp)

if (OGRE_BUILD_PLATFORM_APPLE_IOS)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/PCZTestApp.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../PlugIns/OctreeZone/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../PlugIns/PCZSceneManager/include
)
add_executable(Demo_PCZTestApp WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_PCZTestApp ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OGRE_Plugin_PCZSceneManager_LIBRARIES} ${OGRE_Plugin_OctreeZone_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_exe(Demo_PCZTestApp)
# need to include plugins directory in RPATH for the demo to be able to find the PCZ libs
set_property(TARGET Demo_PCZTestApp APPEND PROPERTY
  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib/OGRE
)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_PCZTestApp ${SAMPLE_DEPENDENCIES})
endif ()
