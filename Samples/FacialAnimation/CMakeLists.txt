#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure FacialAnimation demo build

if (OGRE_BUILD_PLATFORM_IPHONE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/FacialAnimation.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/FacialAnimation.mm COPYONLY)
  set(SOURCE_FILES src/FacialAnimation.mm)
else ()
  set(SOURCE_FILES src/FacialAnimation.cpp)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_FacialAnimation WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_FacialAnimation ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUIRenderer_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_FacialAnimation)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_FacialAnimation ${SAMPLE_DEPENDENCIES})
endif ()
