#----------------------------------------------------------
# Sample SSAO CMake file
# 

set(HEADER_FILES include/SSAO.h)
set(SOURCE_FILES src/SSAO.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_SSAO ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_SSAO ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(Sample_SSAO)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_SSAO
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

