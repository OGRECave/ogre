#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# Overlay optional component
############################################################

# define header and source files for the library
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES 
  ${PROJECT_BINARY_DIR}/include/OgreOverlayPrerequisites.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/OgreOverlay.i)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# setup target
add_library(OgreOverlay ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreOverlay PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreOverlay PUBLIC OgreMain PRIVATE "${FREETYPE_LIBRARIES}" ZLIB::ZLIB)
target_include_directories(OgreOverlay PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Overlay>
  PRIVATE ${FREETYPE_INCLUDE_DIRS})

generate_export_header(OgreOverlay 
    EXPORT_MACRO_NAME _OgreOverlayExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreOverlayPrerequisites.h)

# install 
ogre_config_framework(OgreOverlay)
ogre_config_component(OgreOverlay)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Overlay
)
install(FILES "${PROJECT_SOURCE_DIR}/Media/packs/profiler.zip"
  DESTINATION "${OGRE_MEDIA_PATH}/packs/"
)