cmake_minimum_required (VERSION 2.8)
project(OgreTutorialsSample)

# required for Ogre 1.11+
set(CMAKE_CXX_STANDARD 11)

## [discover_ogre]
# specify which version and components you need
find_package(OGRE 1.11 REQUIRED COMPONENTS Bites RTShaderSystem)

# copy resource.cfg next to our binaries where OGRE looks for it
file(COPY ${OGRE_CONFIG_DIR}/resources.cfg DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# add the source files as usual
add_executable(0_Bootstrap Bootstrap.cpp)

# this also sets the includes and pulls third party dependencies
target_link_libraries(0_Bootstrap OgreBites OgreRTShaderSystem)
## [discover_ogre]

add_executable(1_FirstScene BasicTutorial1.cpp)

target_link_libraries(1_FirstScene ${OGRE_LIBRARIES})

add_executable(2_CamerasAndShadows BasicTutorial2.cpp)

target_link_libraries(2_CamerasAndShadows ${OGRE_LIBRARIES})
