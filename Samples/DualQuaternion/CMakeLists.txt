# Configure DualQuaternion demo build

set(HEADER_FILES include/DualQuaternion.h)
set(SOURCE_FILES src/DualQuaternion.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_DualQuaternion ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(Sample_DualQuaternion ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(Sample_DualQuaternion)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_DualQuaternion
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

