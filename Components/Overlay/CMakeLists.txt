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

PROJECT(OgreOverlay)

# define header and source files for the library
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${CMAKE_BINARY_DIR}/include/OgreOverlayPrerequisites.h)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# workaround Wundef in freetype
if(UNIX)
  set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/OgreFont.cpp" PROPERTIES COMPILE_FLAGS "-Wno-undef")
endif()

# include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/OgreMain/include)

# setup target
ogre_add_library_to_folder(Components OgreOverlay ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreOverlay PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreOverlay OgreMain ${FREETYPE_LIBRARIES})
if (OGRE_CONFIG_THREADS)
  target_link_libraries(OgreOverlay ${OGRE_THREAD_LIBRARIES})
endif ()

generate_export_header(OgreOverlay 
    EXPORT_MACRO_NAME _OgreOverlayExport
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/OgreOverlayPrerequisites.h)

# install 
ogre_config_framework(OgreOverlay)
ogre_config_component(OgreOverlay)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Overlay
)
