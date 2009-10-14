#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure DeferredShading demo build

set(HEADER_FILES 
  include/AmbientLight.h
  include/DeferredLightCP.h
  include/DeferredShading.h
  include/DLight.h
  include/GBufferMaterialGenerator.h
  include/GBufferSchemeHandler.h
  include/GeomUtils.h
  include/LightMaterialGenerator.h
  include/ListenerFactoryLogic.h
  include/MaterialGenerator.h
  include/NullSchemeHandler.h
  include/SharedData.h
  include/SSAOLogic.h
)

set(SOURCE_FILES 
  src/AmbientLight.cpp
  src/DeferredLightCP.cpp
  src/DeferredShading.cpp
  src/DeferredShadingDemo.cpp
  src/DLight.cpp
  src/GBufferMaterialGenerator.cpp
  src/GBufferSchemeHandler.cpp
  src/GeomUtils.cpp
  src/LightMaterialGenerator.cpp
  src/MaterialGenerator.cpp
  src/SSAOLogic.cpp
)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/DeferredShadingDemo.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_DeferredShading ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_DeferredShading ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Demo_DeferredShading)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(Demo_DeferredShading ${SAMPLE_DEPENDENCIES})
endif ()
