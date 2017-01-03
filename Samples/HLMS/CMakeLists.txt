# Configure HLMS demo build

set(HEADER_FILES include/HLMS.h)
set(SOURCE_FILES src/HLMS.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

ogre_add_library_to_folder(Samples Sample_HLMS ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_HLMS ${OGRE_LIBRARIES} ${SDL2_LIBRARY} ${SAMPLE_DEPENDENCIES})
ogre_config_sample_lib(Sample_HLMS)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_HLMS
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()
