#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreRsImageCodecExports.h)
set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreRsImageCodec.cpp")

if(MSVC)
    set(RS_LIBNAME "rsimage.lib")
else()
    set(RS_LIBNAME "librsimage.a")
endif()
set(RS_LIB_PATH "${CMAKE_BINARY_DIR}/rs/release/${RS_LIBNAME}")

add_custom_target(rsimage
    COMMAND cargo build --release --target-dir=${CMAKE_BINARY_DIR}/rs/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    BYPRODUCTS "${RS_LIB_PATH}")

add_library(Codec_RsImage ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCES})
add_dependencies(Codec_RsImage rsimage)
target_link_libraries(Codec_RsImage PUBLIC OgreMain PRIVATE ${RS_LIB_PATH} ${CMAKE_DL_LIBS})

if(MSVC)
    target_link_libraries(Codec_RsImage PRIVATE wsock32 ws2_32 bcrypt userenv)
endif()

target_include_directories(Codec_RsImage PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Plugins/RsImageCodec>)

ogre_config_framework(Codec_RsImage)
ogre_config_plugin(Codec_RsImage)
generate_export_header(Codec_RsImage
    EXPORT_MACRO_NAME _OgreRsImageCodecExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreRsImageCodecExports.h)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/RsImageCodec)
