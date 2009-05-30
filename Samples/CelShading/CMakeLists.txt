#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure CelShading demo build

set(SOURCE_FILES src/CelShading.cpp)

add_executable(Demo_CelShading WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_CelShading ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_CelShading)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_CelShading ${SAMPLE_DEPENDENCIES})
endif ()
