# Configure Paging demo build

set(HEADER_FILES include/Paging.h)
set(SOURCE_FILES src/Paging.cpp)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${OGRE_SOURCE_DIR}/Components/Paging/include)

add_library(Sample_Paging ${OGRE_LIB_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_Paging ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_Paging_LIBRARIES})
ogre_config_sample_lib(Sample_Paging)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(Sample_Paging
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

