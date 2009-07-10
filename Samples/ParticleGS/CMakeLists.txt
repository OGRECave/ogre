#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure ParticleGS demo build

if (OGRE_BUILD_PLATFORM_IPHONE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/ParticleGS.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/ParticleGS.mm COPYONLY)
  set(SOURCE_FILES 
    src/ParticleGS.mm
    src/ProceduralManualObject.cpp
    src/ProceduralManualObject.h
    src/RandomTools.cpp
    src/RandomTools.h
  )
else ()
  set(SOURCE_FILES 
    src/ParticleGS.cpp
    src/ProceduralManualObject.cpp
    src/ProceduralManualObject.h
    src/RandomTools.cpp
    src/RandomTools.h
  )
endif ()

add_executable(Demo_ParticleGS WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_ParticleGS ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_ParticleGS)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_ParticleGS ${SAMPLE_DEPENDENCIES})
endif ()
