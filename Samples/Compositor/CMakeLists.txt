#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Compositor demo build

set(HEADER_FILES 
  include/Compositor.h
  include/CompositorDemo_FrameListener.h
  include/ItemSelectorViewManager.h
  include/HelperLogics.h
)

set(SOURCE_FILES 
  src/Compositor.cpp
  src/CompositorDemo_FrameListener.cpp
  src/ItemSelectorViewManager.cpp
  src/HelperLogics.cpp
)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Compositor.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/CEGUIRenderer/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_Compositor WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Compositor ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${CEGUI_LIBRARIES} ${OGRE_CEGUIRenderer_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_Compositor)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_Compositor ${SAMPLE_DEPENDENCIES})
endif ()
