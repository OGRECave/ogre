# Configure Grass demo build

set(SOURCE_FILES src/Paging.cpp)

include_directories(${CMAKE_SOURCE_DIR}/Components/Paging/include)

add_executable(Demo_Paging WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Demo_Paging ${OGRE_LIBRARIES} ${OIS_LIBRARIES} OgrePaging)
ogre_config_sample(Demo_Paging)

add_dependencies(Demo_Paging ${SAMPLE_DEPENDENCIES})
