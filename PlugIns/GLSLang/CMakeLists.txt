file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreGLSLangProgramManagerExports.h)

add_library(Plugin_GLSLangProgramManager ${OGRE_LIB_TYPE} ${HEADER_FILES} src/OgreGLSLang.cpp)
target_include_directories(Plugin_GLSLangProgramManager PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/Plugins/GLSLang>)
target_link_libraries(Plugin_GLSLangProgramManager PUBLIC OgreMain glslang HLSL OSDependent OGLCompiler SPIRV SPIRV-Tools)

ogre_config_framework(Plugin_GLSLangProgramManager)
ogre_config_plugin(Plugin_GLSLangProgramManager)
generate_export_header(Plugin_GLSLangProgramManager
    EXPORT_MACRO_NAME _OgreGLSLangProgramManagerExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreGLSLangProgramManagerExports.h)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/GLSLang)