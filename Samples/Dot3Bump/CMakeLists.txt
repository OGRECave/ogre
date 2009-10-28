# Configure Dot3Bump demo build

set(HEADER_FILES include/Dot3Bump.h)
set(SOURCE_FILES src/Dot3Bump.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_Dot3Bump ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_Dot3Bump ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(Sample_Dot3Bump)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_Dot3Bump
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

if (OGRE_STATIC)
  add_dependencies(SampleBrowser Sample_Dot3Bump)
else ()
  add_dependencies(Sample_Dot3Bump SampleBrowser)
endif (OGRE_STATIC)
