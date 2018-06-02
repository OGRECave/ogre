cmake_minimum_required (VERSION 2.8)
project(OgreTutorialsSample)

## [discover_ogre]
# specify which version and components you need
find_package(OGRE 1.11 REQUIRED COMPONENTS Bites RTShaderSystem)

# copy resource.cfg next to our binaries where OGRE looks for it
file(COPY ${OGRE_CONFIG_DIR}/resources.cfg DESTINATION ${CMAKE_BINARY_DIR})
## [discover_ogre]

add_executable(0_Bootstrap Bootstrap.cpp)
target_link_libraries(0_Bootstrap ${OGRE_LIBRARIES})

add_executable(1_FirstScene BasicTutorial1.cpp)
target_link_libraries(1_FirstScene ${OGRE_LIBRARIES})

add_executable(2_CamerasAndShadows BasicTutorial2.cpp)
target_link_libraries(2_CamerasAndShadows ${OGRE_LIBRARIES})

