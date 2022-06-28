#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# Paging optional component
############################################################

if(ANDROID)
    include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
endif()

if(EMSCRIPTEN)
    add_definitions(-s USE_SDL=2)
endif()

# Find X11
if (UNIX AND NOT APPLE AND NOT ANDROID AND NOT EMSCRIPTEN)
  find_library(XAW_LIBRARY NAMES Xaw Xaw7 PATHS ${OGRE_DEP_SEARCH_PATH} ${DEP_LIB_SEARCH_DIR} ${X11_LIB_SEARCH_PATH})
  macro_log_feature(XAW_LIBRARY "Xaw" "X11 Athena widget set for ConfigDialog" "http://www.x.org")
  mark_as_advanced(XAW_LIBRARY)
endif ()

if (OGRE_STATIC OR OGRE_BITES_STATIC_PLUGINS)
  # Link to all enabled plugins
  if (OGRE_BUILD_PLUGIN_OCTREE)
    set(DEPENDENCIES ${DEPENDENCIES} Plugin_OctreeSceneManager)
  endif ()
  if (OGRE_BUILD_PLUGIN_BSP)
    set(DEPENDENCIES ${DEPENDENCIES} Plugin_BSPSceneManager)
  endif ()
  if (OGRE_BUILD_PLUGIN_CG)
    set(DEPENDENCIES ${DEPENDENCIES} Plugin_CgProgramManager)
  endif ()
  if (OGRE_BUILD_PLUGIN_GLSLANG)
    set(DEPENDENCIES ${DEPENDENCIES} Plugin_GLSLangProgramManager)
  endif ()
  if (OGRE_BUILD_PLUGIN_PFX)
    set(DEPENDENCIES ${DEPENDENCIES} Plugin_ParticleFX)
  endif ()
  if (OGRE_BUILD_PLUGIN_PCZ)
    set(DEPENDENCIES ${DEPENDENCIES} Plugin_PCZSceneManager)
  endif ()
  if (OGRE_BUILD_PLUGIN_DOT_SCENE)
    set(DEPENDENCIES ${DEPENDENCIES} Plugin_DotScene)
  endif ()
  if (OGRE_BUILD_PLUGIN_ASSIMP)
    set(DEPENDENCIES ${DEPENDENCIES} Codec_Assimp)
  endif ()
  if (OGRE_BUILD_PLUGIN_STBI)
    set(DEPENDENCIES ${DEPENDENCIES} Codec_STBI)
  endif ()
  if (OGRE_BUILD_PLUGIN_FREEIMAGE)
    set(DEPENDENCIES ${DEPENDENCIES} Codec_FreeImage)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_D3D9)
    include_directories(${PROJECT_SOURCE_DIR}/RenderSystems/Direct3D9/include ${DirectX9_INCLUDE_DIR})
    set(DEPENDENCIES ${DEPENDENCIES} RenderSystem_Direct3D9)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_D3D11)
    include_directories(${PROJECT_SOURCE_DIR}/RenderSystems/Direct3D11/include)
    set(DEPENDENCIES ${DEPENDENCIES} RenderSystem_Direct3D11)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_GL)
    set(DEPENDENCIES ${DEPENDENCIES} RenderSystem_GL)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
    set(DEPENDENCIES ${DEPENDENCIES} RenderSystem_GL3Plus)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_GLES2)
    set(DEPENDENCIES ${DEPENDENCIES} RenderSystem_GLES2)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_TINY)
    set(DEPENDENCIES ${DEPENDENCIES} RenderSystem_Tiny)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_VULKAN)
    set(DEPENDENCIES ${DEPENDENCIES} RenderSystem_Vulkan)
  endif ()
endif ()

# define header and source files for the library
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES
  ${PROJECT_BINARY_DIR}/include/OgreBitesPrerequisites.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/OgreBites.i)
set(SOURCES 
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreAdvancedRenderControls.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreApplicationContextBase.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreBitesConfigDialog.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreCameraMan.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreSGTechniqueResolverListener.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreStaticPluginLoader.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreTrays.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreWindowEventUtilities.cpp")

if(SDL2_FOUND OR EMSCRIPTEN)
  list(APPEND SOURCES  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreApplicationContextSDL.cpp")
elseif(ANDROID)
  list(APPEND SOURCES  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreApplicationContextAndroid.cpp")
endif()

if(ANDROID OR EMSCRIPTEN OR APPLE_IOS OR WINDOWS_STORE OR WINDOWS_PHONE)
  # no config dialog available
elseif(WIN32)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
  list(APPEND SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreWIN32ConfigDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/resource.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/misc/OgreWin32Resources.rc")
  # Need to include resource files so that icons are linked
  set(RESOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/resource.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/misc/OgreWin32Resources.rc"
  )
  source_group(Resources FILES ${RESOURCE_FILES})
elseif(APPLE)
  list(APPEND SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreOSXConfigDialog.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/misc/ogrelogo.png")
  set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/OgreOSXConfigDialog.mm"
    PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations"
  )
  set(RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/ogrelogo.png")
  source_group(Resources FILES ${RESOURCE_FILES})
  set(DEPENDENCIES ${DEPENDENCIES} "-framework Cocoa")
elseif(UNIX)
  list(APPEND DEPENDENCIES ${X11_X11_LIB})
  set(NATIVE_INCLUDES ${X11_Xlib_INCLUDE_PATH})
  if(XAW_LIBRARY)
    set_source_files_properties(src/OgreBitesConfigDialog.cpp PROPERTIES COMPILE_DEFINITIONS HAVE_XAW)
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreGLXConfigDialog.cpp")
    list(APPEND DEPENDENCIES ${X11_Xt_LIB} ${XAW_LIBRARY})
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/GLX_backdrop.png" DESTINATION "share/OGRE/")
  endif()
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
  list(APPEND SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreImGuiInputListener.cpp")
endif()

# setup target
add_library(OgreBites ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCES})
set_target_properties(OgreBites PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreBites PUBLIC OgreMain OgreOverlay PRIVATE ${DEPENDENCIES})
if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
  target_link_libraries(OgreBites PUBLIC OgreRTShaderSystem)
endif ()
target_include_directories(OgreBites PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${NATIVE_INCLUDES}>"
  $<INSTALL_INTERFACE:include/OGRE/Bites>)

if (OGRE_STATIC OR OGRE_BITES_STATIC_PLUGINS)
  target_compile_definitions(OgreBites PRIVATE OGRE_BITES_STATIC_PLUGINS)
endif()

if(OGRE_STATIC AND APPLE AND OGRE_BUILD_PLUGIN_CG)
  # workaround so the Cg framework is found in the above condition
  target_include_directories(OgreBites PUBLIC ${Cg_INCLUDE_DIRS})
endif()

if(SDL2_FOUND)
  target_link_libraries(OgreBites PRIVATE SDL2::SDL2)
elseif(NOT EMSCRIPTEN)
  message(WARNING "SDL2 not found - no input handling and reduced window creation capabilites")
endif()

generate_export_header(OgreBites 
    EXPORT_MACRO_NAME _OgreBitesExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreBitesPrerequisites.h)

if(Qt6_FOUND OR Qt5_FOUND)
  if(Qt6_FOUND)
    qt6_wrap_cpp(MOC_SRC "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreApplicationContextQt.h")
  else()
    qt5_wrap_cpp(MOC_SRC "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreApplicationContextQt.h")
  endif()

  add_library(OgreBitesQt ${OGRE_COMP_LIB_TYPE} ${MOC_SRC} "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreApplicationContextQt.cpp")
  set_target_properties(OgreBitesQt PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
  target_link_libraries(OgreBitesQt PUBLIC Qt${QT_VERSION_MAJOR}::Gui OgreBites)
  ogre_config_component(OgreBitesQt)
endif()

# install 
ogre_config_framework(OgreBites)
ogre_config_component(OgreBites)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Bites
)
install(FILES "${PROJECT_SOURCE_DIR}/Media/packs/SdkTrays.zip"
  DESTINATION "${OGRE_MEDIA_PATH}/packs/"
)
