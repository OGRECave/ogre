#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Fresnel demo build

set(SOURCE_FILES src/Fresnel.cpp)

add_executable(Demo_Fresnel WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Fresnel ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Fresnel)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Fresnel ${SAMPLE_DEPENDENCIES})
endif ()
