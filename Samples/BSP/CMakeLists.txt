# Configure BSP demo build

set(SOURCE_FILES src/BSP.cpp)
set(HEADER_FILES include/BSP.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_BSP ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_BSP ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(Sample_BSP)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_BSP
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

add_dependencies(SampleBrowser Sample_BSP)


