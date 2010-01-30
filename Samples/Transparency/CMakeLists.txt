# Configure Transparency demo build

set(HEADER_FILES include/Transparency.h)
set(SOURCE_FILES src/Transparency.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_Transparency ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_Transparency ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(Sample_Transparency)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_Transparency
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

