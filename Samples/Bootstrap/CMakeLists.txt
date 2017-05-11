cmake_minimum_required (VERSION 2.8)
project(BootstrapOgre)

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

add_executable(BootstrapOgre main.cpp)
target_link_libraries(BootstrapOgre ${OGRE_LIBRARIES})