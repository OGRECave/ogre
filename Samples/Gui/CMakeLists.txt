#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Gui demo build

set(SOURCE_FILES src/Gui.cpp)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Gui.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
add_executable(Demo_Gui WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Gui ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUIRenderer_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Gui)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Gui ${SAMPLE_DEPENDENCIES})
endif ()
