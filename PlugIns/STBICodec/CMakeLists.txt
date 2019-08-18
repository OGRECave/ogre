#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreSTBICodecExports.h)
set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreSTBICodec.cpp")

if(UNIX)
  set_source_files_properties(src/OgreSTBICodec.cpp PROPERTIES  
    COMPILE_FLAGS "-Wno-cast-qual -Wno-unused-function -Wno-missing-declarations -Wno-type-limits")
endif()

add_library(Codec_STBI ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCES})
target_link_libraries(Codec_STBI PUBLIC OgreMain)

target_include_directories(Codec_STBI PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Plugins/STBICodec>)

if(CMAKE_COMPILER_IS_GNUCXX)
  # workaround gcc5 bug on Ubuntu 16.04 regarding __builtin_cpu_supports
  target_link_libraries(Codec_STBI PRIVATE gcc)
endif()
if(OGRE_CONFIG_ENABLE_ZIP)
  target_link_libraries(Codec_STBI PRIVATE ZLIB::ZLIB)
endif()

ogre_config_framework(Codec_STBI)
ogre_config_plugin(Codec_STBI)
generate_export_header(Codec_STBI 
    EXPORT_MACRO_NAME _OgreSTBICodecExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreSTBICodecExports.h)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/STBICodec)
