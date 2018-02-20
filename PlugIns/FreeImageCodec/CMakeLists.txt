#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${CMAKE_BINARY_DIR}/include/OgreFreeImageCodecExports.h)
set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreFreeImageCodec.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions(-DFREEIMAGE_LIB)
add_library(Codec_FreeImage ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCES})
target_link_libraries(Codec_FreeImage OgreMain "${FreeImage_LIBRARIES}")

if (MINGW)
  # may need winsock htons functions for FreeImage
  target_link_libraries(Codec_FreeImage ws2_32)
endif ()

ogre_config_framework(Codec_FreeImage)
ogre_config_plugin(Codec_FreeImage)
generate_export_header(Codec_FreeImage 
    EXPORT_MACRO_NAME _OgreFreeImageCodecExport
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/OgreFreeImageCodecExports.h)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/FreeImageCodec)
