# Configure Isosurf demo build

set(SOURCE_FILES 
  src/Isosurf.cpp
  src/ProceduralTools.cpp
)

add_executable(Demo_Isosurf WIN32 ${SOURCE_FILES})
ogre_config_build(Demo_Isosurf)
target_link_libraries(Demo_Isosurf ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_install_sample(Demo_Isosurf)
