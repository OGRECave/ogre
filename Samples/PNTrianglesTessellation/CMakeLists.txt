# Configure Parametric PN Triangles Tessellation demo build

set(HEADER_FILES include/PNTrianglesTessellation.h)
set(SOURCE_FILES src/PNTrianglesTessellation.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
ogre_add_library(Sample_PNTriangles ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_PNTriangles ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(Sample_PNTriangles)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_PNTriangles
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()
