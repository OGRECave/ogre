# Configure Terrain Tessellation demo build

set(HEADER_FILES
	include/TerrainTessellation.h
	include/TerrainTessellationMaterialGenerator.h)
set(SOURCE_FILES
	src/TerrainTessellation.cpp
	src/TerrainTessellationMaterialGenerator.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
ogre_add_component_include_dir(Terrain)
ogre_add_component_include_dir(Paging)
ogre_add_library(Sample_TerrainTessellation ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_TerrainTessellation ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_Terrain_LIBRARIES} ${OGRE_Paging_LIBRARIES})
ogre_config_sample_lib(Sample_TerrainTessellation)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_TerrainTessellation
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()
