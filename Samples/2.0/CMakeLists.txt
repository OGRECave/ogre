# Configure Samples build

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)

if (OGRE_BUILD_SAMPLES2 AND NOT OIS_FOUND)
    message(STATUS "Could not find dependency: SDL2")
    message(STATUS "Skipping samples build")
    set(OGRE_BUILD_SAMPLES2 0)
else()
    include_directories(${SDL2_INCLUDE_DIR})
endif()

if( OGRE_BUILD_SAMPLES2 )
    add_subdirectory(Common)
    add_subdirectory(ApiUsage/V2Mesh)
endif()
