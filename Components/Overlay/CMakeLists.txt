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
  set(IMGUI_DIR "${PROJECT_BINARY_DIR}/imgui-1.91.2" CACHE PATH "")
  if(NOT EXISTS ${IMGUI_DIR})
    message(STATUS "Downloading imgui")
    file(DOWNLOAD
        https://github.com/ocornut/imgui/archive/v1.91.2.tar.gz
        ${PROJECT_BINARY_DIR}/imgui.tar.gz)
    execute_process(COMMAND ${CMAKE_COMMAND}
        -E tar xf imgui.tar.gz WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
  endif()
  list(APPEND SOURCE_FILES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp)
  if(FREETYPE_FOUND)
    set(IMGUI_ENABLE_FREETYPE TRUE)
    list(APPEND SOURCE_FILES ${IMGUI_DIR}/misc/freetype/imgui_freetype.cpp)
  endif()
else()
    list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreImGuiOverlay.cpp")
endif()

# setup target
add_library(OgreOverlay ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreOverlay PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreOverlay PUBLIC OgreMain)
target_include_directories(OgreOverlay PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Overlay>)

if(FREETYPE_FOUND)
  target_compile_definitions(OgreOverlay PRIVATE HAVE_FREETYPE)
  target_link_libraries(OgreOverlay PRIVATE ${FREETYPE_LIBRARIES})
  target_include_directories(OgreOverlay PRIVATE ${FREETYPE_INCLUDE_DIRS})
  if(ZLIB_FOUND)
    target_link_libraries(OgreOverlay PRIVATE ZLIB::ZLIB)
  endif()
elseif(UNIX)
  set_source_files_properties(src/OgreFont.cpp PROPERTIES
    COMPILE_FLAGS "-Wno-cast-qual -Wno-unused-function")
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
  target_include_directories(OgreOverlay PUBLIC
    PUBLIC "$<BUILD_INTERFACE:${IMGUI_DIR}>"
    PRIVATE "$<BUILD_INTERFACE:${IMGUI_DIR}/misc/freetype>")

  if(UNIX)
    set_source_files_properties(${IMGUI_DIR}/misc/freetype/imgui_freetype.cpp PROPERTIES
      COMPILE_FLAGS "-Wno-missing-declarations")

    set_source_files_properties(${IMGUI_DIR}/imgui_widgets.cpp PROPERTIES
      COMPILE_FLAGS "-Wno-cast-qual")
    set_source_files_properties(${IMGUI_DIR}/imgui_draw.cpp PROPERTIES
      COMPILE_FLAGS "-Wno-cast-qual")
    set_source_files_properties(${IMGUI_DIR}/imgui.cpp PROPERTIES
      COMPILE_FLAGS "-Wno-cast-qual -Wno-unused-variable")
    set_source_files_properties(${IMGUI_DIR}/imgui_demo.cpp PROPERTIES
      COMPILE_FLAGS "-Wno-cast-qual")
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set_property(SOURCE ${IMGUI_DIR}/misc/freetype/imgui_freetype.cpp
        APPEND PROPERTY COMPILE_OPTIONS "-Wno-unknown-warning-option")
    endif()
  endif()

  if(MINGW)
    target_link_libraries(OgreOverlay PRIVATE imm32)
  endif()

  configure_file(${OGRE_TEMPLATES_DIR}/imconfig.h ${IMGUI_DIR}/imconfig.h)

  list(APPEND HEADER_FILES
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imconfig.h)
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
