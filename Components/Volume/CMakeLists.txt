#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# Volume optional component
############################################################

# define header and source files for the library
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreVolumePrerequisites.h)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# setup target
add_library(OgreVolume ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreVolume PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreVolume PUBLIC OgreMain)
target_include_directories(OgreVolume PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Volume>)

generate_export_header(OgreVolume 
    EXPORT_MACRO_NAME _OgreVolumeExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreVolumePrerequisites.h)

# install 
ogre_config_framework(OgreVolume)
ogre_config_component(OgreVolume)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Volume
)
