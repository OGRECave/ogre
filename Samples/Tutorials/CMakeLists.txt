cmake_minimum_required (VERSION 2.8)
project(OgreTutorialsSample)

## [discover_ogre]
# specify which version you need
find_package(OGRE 1.10 REQUIRED)

# the search paths
include_directories(${OGRE_INCLUDE_DIRS})
link_directories(${OGRE_LIBRARY_DIRS})

# copy essential config files next to our binary where OGRE autodiscovers them
file(COPY ${OGRE_CONFIG_DIR}/plugins.cfg ${OGRE_CONFIG_DIR}/resources.cfg
     DESTINATION ${CMAKE_BINARY_DIR})
## [discover_ogre]

add_executable(starter TutorialApplication.cpp)
target_link_libraries(starter ${OGRE_LIBRARIES})

add_executable(basic1 BasicTutorial1.cpp)
target_link_libraries(basic1 ${OGRE_LIBRARIES})

