# Configure VolumeTex demo build

set(HEADER_FILES 
  include/VolumeRenderable.h
  include/Julia.h
  include/ThingRenderable.h
)
set(SOURCE_FILES 
  src/VolumeTex.cpp
  src/ThingRenderable.cpp
  src/VolumeRenderable.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(Demo_VolumeTex WIN32 ${HEADER_FILES} ${SOURCE_FILES})
ogre_config_build(Demo_VolumeTex)
target_link_libraries(Demo_VolumeTex ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_VolumeTex)
