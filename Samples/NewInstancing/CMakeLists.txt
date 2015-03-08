# Configure NewInstancing demo build

set(HEADER_FILES 
  include/NewInstancing.h
  include/MersenneTwister.h)
set(SOURCE_FILES src/NewInstancing.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
ogre_add_library_to_folder(Samples Sample_NewInstancing ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(Sample_NewInstancing ${OGRE_LIBRARIES} ${SDL2_LIBRARY})
ogre_config_sample_lib(Sample_NewInstancing)

if (APPLE AND NOT APPLE_IOS)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_NewInstancing
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

