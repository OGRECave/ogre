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

if (OGRE_BUILD_PLATFORM_IPHONE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/VolumeTex.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/VolumeTex.mm COPYONLY)
  set(SOURCE_FILES 
    src/VolumeTex.mm
    src/ThingRenderable.cpp
    src/VolumeRenderable.cpp
  )
else ()
  set(SOURCE_FILES 
    src/VolumeTex.cpp
    src/ThingRenderable.cpp
    src/VolumeRenderable.cpp
  )
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_VolumeTex WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_VolumeTex ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_VolumeTex)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_VolumeTex ${SAMPLE_DEPENDENCIES})
endif ()
