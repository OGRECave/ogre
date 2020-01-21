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

if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
  list(APPEND SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/misc/freetype/imgui_freetype.cpp)
else()
    list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreImGuiOverlay.cpp")
endif()

# setup target
add_library(OgreOverlay ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreOverlay PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreOverlay PUBLIC OgreMain PRIVATE ${FREETYPE_LIBRARIES} ZLIB::ZLIB)
target_include_directories(OgreOverlay PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Overlay>
  PRIVATE ${FREETYPE_INCLUDE_DIRS})

if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
  target_include_directories(OgreOverlay PUBLIC
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/imgui>"
    PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/misc/freetype>")

  if(UNIX)
    set_source_files_properties(src/imgui/misc/freetype/imgui_freetype.cpp PROPERTIES
      COMPILE_FLAGS "-Wno-missing-declarations")

    set_source_files_properties(src/imgui/imgui_widgets.cpp PROPERTIES
      COMPILE_FLAGS "-Wno-cast-qual")
  endif()

  file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/imconfig.h
    "#pragma once
    #define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    #include <OgreOverlayPrerequisites.h>
    #define IMGUI_API _OgreOverlayExport")

  list(APPEND HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/imconfig.h)
endif()

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