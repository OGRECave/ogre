#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${CMAKE_BINARY_DIR}/include/OgreEXRCodecExports.h)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(SYSTEM ${OPENEXR_INCLUDE_DIR}/OpenEXR)

add_library(Codec_EXR ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Codec_EXR OgreMain ${OPENEXR_LIBRARIES})
ogre_config_framework(Codec_EXR)
ogre_config_plugin(Codec_EXR)
generate_export_header(Codec_EXR 
    EXPORT_MACRO_NAME _OgreEXRPluginExport
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/OgreEXRCodecExports.h)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/EXRCodec)
