file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreGLSLangProgramManagerExports.h)

add_library(Plugin_GLSLangProgramManager ${OGRE_LIB_TYPE} ${HEADER_FILES} src/OgreGLSLang.cpp)
target_include_directories(Plugin_GLSLangProgramManager PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/Plugins/GLSLang>)
target_compile_features(Plugin_GLSLangProgramManager PRIVATE cxx_std_17)

if(ANDROID)
    target_include_directories(Plugin_GLSLangProgramManager SYSTEM PRIVATE
        ${ANDROID_NDK}/sources/third_party/shaderc/third_party/glslang
        ${ANDROID_NDK}/sources/third_party/shaderc/third_party/)
    target_link_libraries(Plugin_GLSLangProgramManager PUBLIC OgreMain
        ${ANDROID_NDK}/sources/third_party/shaderc/libs/c++_static/${ANDROID_ABI}/libshaderc.a)
elseif(DEFINED ENV{VULKAN_SDK})
    message(STATUS "Using VULKAN_SDK: $ENV{VULKAN_SDK}")
    target_include_directories(Plugin_GLSLangProgramManager SYSTEM PRIVATE $ENV{VULKAN_SDK}/include)
    target_link_directories(Plugin_GLSLangProgramManager PUBLIC $ENV{VULKAN_SDK}/lib)
    target_link_libraries(Plugin_GLSLangProgramManager PUBLIC OgreMain shaderc_combined)
else()
  set(GLSLANG_LIBS glslang SPIRV SPIRV-Tools-opt SPIRV-Tools)
  find_library(GLSLANG_OSDependent OSDependent)
  if(GLSLANG_OSDependent)
    set(GLSLANG_LIBS ${GLSLANG_LIBS} ${GLSLANG_OSDependent})
  endif()
  find_library(GLSLANG_HLSL HLSL)
  if(GLSLANG_HLSL)
    set(GLSLANG_LIBS ${GLSLANG_LIBS} ${GLSLANG_HLSL})
  endif()
  find_library(GLSLANG_OGLCompiler OGLCompiler)
  if(GLSLANG_OGLCompiler)
    set(GLSLANG_LIBS ${GLSLANG_LIBS} ${GLSLANG_OGLCompiler})
  endif()
  find_library(GLSLANG_MI MachineIndependent)
  if(GLSLANG_MI)
      set(GLSLANG_LIBS ${GLSLANG_MI} GenericCodeGen ${GLSLANG_LIBS})
  endif()
  target_link_libraries(Plugin_GLSLangProgramManager PUBLIC OgreMain ${GLSLANG_LIBS})
endif()
ogre_config_framework(Plugin_GLSLangProgramManager)
ogre_config_plugin(Plugin_GLSLangProgramManager)
generate_export_header(Plugin_GLSLangProgramManager
    EXPORT_MACRO_NAME _OgreGLSLangProgramManagerExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreGLSLangProgramManagerExports.h)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/GLSLang)
