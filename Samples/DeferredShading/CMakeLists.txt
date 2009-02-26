# Configure DeferredShading demo build

set(HEADER_FILES 
  include/AmbientLight.h
  include/DeferredShading.h
  include/GeomUtils.h
  include/LightMaterialGenerator.h
  include/MaterialGenerator.h
  include/MLight.h
)
set(SOURCE_FILES 
  src/AmbientLight.cpp
  src/DeferredShading.cpp
  src/DeferredShadingDemo.cpp
  src/GeomUtils.cpp
  src/LightMaterialGenerator.cpp
  src/MaterialGenerator.cpp
  src/MLight.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_DeferredShading ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_DeferredShading)
target_link_libraries(Demo_DeferredShading ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_DeferredShading)
