# Configure SkyBox demo build

set(HEADER_FILES include/SkyBox.h)
set(SOURCE_FILES src/SkyBox.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_SkyBox ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_SkyBox ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(Sample_SkyBox)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_SkyBox
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

add_dependencies(SampleBrowser Sample_SkyBox)
