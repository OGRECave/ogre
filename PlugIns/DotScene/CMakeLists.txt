file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreDotScenePluginExports.h)

add_library(Plugin_DotScene ${OGRE_LIB_TYPE} ${HEADER_FILES} src/DotSceneLoader.cpp)
target_include_directories(Plugin_DotScene PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/Plugins/DotScene>)
target_link_libraries(Plugin_DotScene PUBLIC OgreMain PRIVATE pugixml)
if(OGRE_BUILD_COMPONENT_TERRAIN)
    target_link_libraries(Plugin_DotScene PUBLIC OgreTerrain)
endif()

ogre_config_framework(Plugin_DotScene)
ogre_config_plugin(Plugin_DotScene)
generate_export_header(Plugin_DotScene
    EXPORT_MACRO_NAME _OgreDotScenePluginExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreDotScenePluginExports.h)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/DotScene)