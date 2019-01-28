#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# HLMS optional component
############################################################

# define header and source files for the library
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreHlmsPrerequisites.h)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Add needed definitions
add_definitions(-DOGRE_HLMS_EXPORTS)

# setup target
add_library(OgreHLMS ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreHLMS PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreHLMS PUBLIC OgreMain)
target_include_directories(OgreHLMS PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/HLMS>)

generate_export_header(OgreHLMS
    EXPORT_MACRO_NAME _OgreHlmsExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreHlmsPrerequisites.h)

# install
ogre_config_framework(OgreHLMS)
ogre_config_component(OgreHLMS)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/HLMS
)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/Media/HLMS"
  DESTINATION "${OGRE_MEDIA_PATH}/"
)