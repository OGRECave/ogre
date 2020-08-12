file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreAssimpExports.h)

add_library(Codec_Assimp ${OGRE_LIB_TYPE} ${HEADER_FILES} src/AssimpLoader.cpp)
target_include_directories(Codec_Assimp PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/Plugins/Assimp>)
target_link_libraries(Codec_Assimp PUBLIC OgreMain PRIVATE fix::assimp)
if(OGRE_BUILD_DEPENDENCIES)
    # hack to get our CI build going, despite horribly broken assimp cmake
    target_link_libraries(Codec_Assimp PRIVATE ZLIB::ZLIB ${OGREDEPS_PATH}/lib/IrrXML.lib)
endif()

ogre_config_framework(Codec_Assimp)
ogre_config_plugin(Codec_Assimp)
generate_export_header(Codec_Assimp
    EXPORT_MACRO_NAME _OgreAssimpExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreAssimpExports.h)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/Assimp)