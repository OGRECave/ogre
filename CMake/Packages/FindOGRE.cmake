# - Try to find OGRE
# If you have multiple versions of Ogre installed, use the CMake or
# the environment variable OGRE_HOME to point to the path where the
# desired Ogre version can be found.
# By default this script will look for a dynamic Ogre build. If you
# need to link against static Ogre libraries, set the CMake variable
# OGRE_STATIC to TRUE.
#
# Once done, this will define
#
#  OGRE_FOUND - system has OGRE
#  OGRE_INCLUDE_DIRS - the OGRE include directories 
#  OGRE_LIBRARIES - link these to use the OGRE core
#
# Additionally this script searches for the following optional
# parts of the Ogre package:
#  Plugin_BSPSceneManager, Plugin_CgProgramManager,
#  Plugin_OctreeSceneManager, Plugin_OctreeZone,
#  Plugin_ParticleFX, Plugin_PCZSceneManager,
#  RenderSystem_GL, RenderSystem_Direct3D9, RenderSystem_Direct3D10,
#  CEGUIRenderer, Paging, Terrain
#
# For each of these components, the following variables are defined:
#
#  OGRE_${COMPONENT}_FOUND - ${COMPONENT} is available
#  OGRE_${COMPONENT}_INCLUDE_DIRS - additional include directories for ${COMPONENT}
#  OGRE_${COMPONENT}_LIBRARIES - link these to use ${COMPONENT} 
#
# Finally, the following variables are defined:
#
#  OGRE_PLUGIN_DIR_REL - The directory where the release versions of
#       the OGRE plugins are located
#  OGRE_PLUGIN_DIR_DBG - The directory where the debug versions of
#       the OGRE plugins are located
#  OGRE_MEDIA_DIR - The directory where the OGRE sample media is
#       located, if available

include(FindPkgMacros)
include(PreprocessorUtils)
findpkg_begin(OGRE)


# Get path, convert backslashes as ${ENV_${var}}
getenv_path(OGRE_HOME)
getenv_path(OGRE_SDK)
getenv_path(OGRE_SOURCE)
getenv_path(OGRE_BUILD)
getenv_path(PROGRAMFILES)


# Determine whether to search for a dynamic or static build
if (OGRE_STATIC)
  set(OGRE_LIB_SUFFIX "Static")
else ()
  set(OGRE_LIB_SUFFIX "")
endif ()


set(OGRE_LIBRARY_NAMES "OgreMain${OGRE_LIB_SUFFIX}")
get_debug_names(OGRE_LIBRARY_NAMES)

# construct search paths from environmental hints and
# OS specific guesses
if (WIN32)
  set(OGRE_PREFIX_GUESSES
    ${ENV_PROGRAMFILES}/OGRE
    C:/OgreSDK
  )
elseif (UNIX)
  set(OGRE_PREFIX_GUESSES
    /opt/ogre
    /opt/OGRE
    /usr/lib/ogre
    /usr/lib/OGRE
    /usr/local/lib/ogre
    /usr/local/lib/OGRE
    $ENV{HOME}/ogre
    $ENV{HOME}/OGRE
  )
endif ()
set(OGRE_PREFIX_PATH
  ${OGRE_HOME} ${ENV_OGRE_HOME} ${ENV_OGRE_SDK}
  ${OGRE_PREFIX_GUESSES}
)
create_search_paths(OGRE)
# If both OGRE_BUILD and OGRE_SOURCE are set, prepare to find Ogre in a build dir
set(OGRE_PREFIX_SOURCE ${OGRE_SOURCE} ${ENV_OGRE_SOURCE})
set(OGRE_PREFIX_BUILD ${OGRE_BUILD} ${ENV_OGRE_BUILD})
if (OGRE_PREFIX_SOURCE AND OGRE_PREFIX_BUILD)
  foreach(dir ${OGRE_PREFIX_SOURCE})
    set(OGRE_INC_SEARCH_PATH ${dir}/OgreMain/include ${dir}/Dependencies/include ${OGRE_INC_SEARCH_PATH})
    set(OGRE_LIB_SEARCH_PATH ${dir}/lib ${dir}/Dependencies/lib ${OGRE_LIB_SEARCH_PATH})
  endforeach(dir)
  foreach(dir ${OGRE_PREFIX_BUILD})
    set(OGRE_INC_SEARCH_PATH ${dir}/include ${OGRE_INC_SEARCH_PATH})
    set(OGRE_LIB_SEARCH_PATH ${dir}/lib ${OGRE_LIB_SEARCH_PATH})
  endforeach(dir)
else()
  set(OGRE_PREFIX_SOURCE "NOTFOUND")
  set(OGRE_PREFIX_BUILD "NOTFOUND")
endif ()

# redo search if any of the environmental hints changed
set(OGRE_COMPONENTS Paging Terrain CEGUIRenderer
  Plugin_BSPSceneManager Plugin_CgProgramManager Plugin_OctreeSceneManager
  Plugin_OctreeZone Plugin_PCZSceneManager Plugin_ParticleFX
  RenderSystem_Direct3D10 RenderSystem_Direct3D9 RenderSystem_GL RenderSystem_GLES)
set(OGRE_RESET_VARS 
  OGRE_CONFIG_INCLUDE_DIR OGRE_INCLUDE_DIR 
  OGRE_LIBRARY_FWK OGRE_LIBRARY_REL OGRE_LIBRARY_DBG
  OGRE_PLUGIN_DIR_DBG OGRE_PLUGIN_DIR_REL OGRE_MEDIA_DIR)
foreach (comp ${OGRE_COMPONENTS})
  set(OGRE_RESET_VARS ${OGRE_RESET_VARS}
    OGRE_${comp}_INCLUDE_DIR OGRE_${comp}_LIBRARY_FWK
    OGRE_${comp}_LIBRARY_DBG OGRE_${comp}_LIBRARY_REL
  )
endforeach (comp)
set(OGRE_PREFIX_WATCH ${OGRE_PREFIX_PATH} ${OGRE_PREFIX_SOURCE} ${OGRE_PREFIX_BUILD})
clear_if_changed(OGRE_PREFIX_WATCH ${OGRE_RESET_VARS})

# try to locate Ogre via pkg-config
use_pkgconfig(OGRE_PKGC "OGRE${OGRE_LIB_SUFFIX}")
# try to find framework on OSX
findpkg_framework(OGRE)

# locate Ogre include files
find_path(OGRE_CONFIG_INCLUDE_DIR NAMES OgreConfig.h HINTS ${OGRE_INC_SEARCH_PATH} ${OGRE_FRAMEWORK_INCLUDES} ${OGRE_PKGC_INCLUDE_DIRS} PATH_SUFFIXES "OGRE")
find_path(OGRE_INCLUDE_DIR NAMES OgreRoot.h HINTS ${OGRE_CONFIG_INCLUDE_DIR} ${OGRE_INC_SEARCH_PATH} ${OGRE_FRAMEWORK_INCLUDES} ${OGRE_PKGC_INCLUDE_DIRS} PATH_SUFFIXES "OGRE")
set(OGRE_INCOMPATIBLE FALSE)

if (OGRE_INCLUDE_DIR AND OGRE_CONFIG_INCLUDE_DIR)
  # determine Ogre version
  file(READ ${OGRE_INCLUDE_DIR}/OgrePrerequisites.h OGRE_TEMP_VERSION_CONTENT)
  get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MAJOR OGRE_VERSION_MAJOR)
  get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MINOR OGRE_VERSION_MINOR)
  get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_PATCH OGRE_VERSION_PATCH)
  get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_NAME OGRE_VERSION_NAME)
  set(OGRE_VERSION "${OGRE_VERSION_MAJOR}.${OGRE_VERSION_MINOR}.${OGRE_VERSION_PATCH}")
  pkg_message(OGRE "Found Ogre ${OGRE_VERSION_NAME} (${OGRE_VERSION})")

  # determine configuration settings
  set(OGRE_CONFIG_HEADERS
    ${OGRE_CONFIG_INCLUDE_DIR}/buildsettings.h
    ${OGRE_CONFIG_INCLUDE_DIR}/config.h
    ${OGRE_CONFIG_INCLUDE_DIR}/OgreConfig.h
  )
  foreach(CFG_FILE ${OGRE_CONFIG_HEADERS})
    if (EXISTS ${CFG_FILE})
      set(OGRE_CONFIG_HEADER ${CFG_FILE})
      break()
    endif()
  endforeach()
  if (OGRE_CONFIG_HEADER)
    file(READ ${OGRE_CONFIG_HEADER} OGRE_TEMP_CONFIG_CONTENT)
    has_preprocessor_entry(OGRE_TEMP_CONFIG_CONTENT OGRE_STATIC_LIB OGRE_CONFIG_STATIC)
    get_preprocessor_entry(OGRE_TEMP_CONFIG_CONTENT OGRE_THREAD_SUPPORT OGRE_CONFIG_THREADS)
    get_preprocessor_entry(OGRE_TEMP_CONFIG_CONTENT OGRE_NO_FREEIMAGE OGRE_CONFIG_FREEIMAGE)
    if (OGRE_CONFIG_STATIC AND OGRE_STATIC)
    elseif (OGRE_CONFIG_STATIC OR OGRE_STATIC)
      pkg_message(OGRE "Build type (static, dynamic) does not match the requested one.")
      set(OGRE_INCOMPATIBLE TRUE)
    endif ()
  else ()
    pkg_message(OGRE "Could not determine Ogre build configuration.")
    set(OGRE_INCOMPATIBLE TRUE)
  endif ()
else ()
  set(OGRE_INCOMPATIBLE FALSE)
endif ()

find_library(OGRE_LIBRARY_REL NAMES ${OGRE_LIBRARY_NAMES} HINTS ${OGRE_LIB_SEARCH_PATH} ${OGRE_PKGC_LIBRARY_DIRS} ${OGRE_FRAMEWORK_SEARCH_PATH} PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel")
find_library(OGRE_LIBRARY_DBG NAMES ${OGRE_LIBRARY_NAMES_DBG} HINTS ${OGRE_LIB_SEARCH_PATH} ${OGRE_PKGC_LIBRARY_DIRS} ${OGRE_FRAMEWORK_SEARCH_PATH} PATH_SUFFIXES "" "debug")
make_library_set(OGRE_LIBRARY)

if (OGRE_INCOMPATIBLE)
  set(OGRE_LIBRARY "NOTFOUND")
endif ()

set(OGRE_INCLUDE_DIR ${OGRE_CONFIG_INCLUDE_DIR} ${OGRE_INCLUDE_DIR})
list(REMOVE_DUPLICATES OGRE_INCLUDE_DIR)
findpkg_finish(OGRE)
add_parent_dir(OGRE_INCLUDE_DIRS OGRE_INCLUDE_DIR)

mark_as_advanced(OGRE_CONFIG_INCLUDE_DIR OGRE_MEDIA_DIR OGRE_PLUGIN_DIR_REL OGRE_PLUGIN_DIR_DBG)

if (NOT OGRE_FOUND)
  return()
endif ()


# look for required Ogre dependencies in case of static build and/or threading
if (OGRE_STATIC)
  set(OGRE_DEPS_FOUND TRUE)
  find_package(Cg QUIET)
  find_package(DirectX QUIET)
  find_package(FreeImage QUIET)
  find_package(Freetype QUIET)
  find_package(OpenGL QUIET)
  find_package(OpenGLES QUIET)
  find_package(ZLIB QUIET)
  find_package(ZZip QUIET)
  if (UNIX AND NOT APPLE)
    find_package(X11 QUIET)
    find_library(XAW_LIBRARY NAMES Xaw Xaw7 PATHS ${DEP_LIB_SEARCH_DIR} ${X11_LIB_SEARCH_PATH})
    if (NOT XAW_LIBRARY OR NOT X11_Xt_FOUND)
      set(X11_FOUND FALSE)
    endif ()
  endif ()
  if (APPLE)
    find_package(Cocoa QUIET)
    find_package(Carbon QUIET)
    if (NOT Cocoa_FOUND OR NOT Carbon_FOUND)
      set(OGRE_DEPS_FOUND FALSE)
    endif ()
  endif ()
    
  
  set(OGRE_LIBRARIES ${OGRE_LIBRARIES} ${ZLIB_LIBRARIES} ${ZZip_LIBRARIES}
    ${FreeImage_LIBRARIES} ${FREETYPE_LIBRARIES} 
    ${X11_LIBRARIES} ${X11_Xt_LIBRARIES} ${XAW_LIBRARY}
    ${Cocoa_LIBRARIES} ${Carbon_LIBRARIES})
  
  if (NOT ZLIB_FOUND OR NOT ZZip_FOUND)
    set(OGRE_DEPS_FOUND FALSE)
  endif ()
  if (NOT FreeImage_FOUND AND NOT OGRE_CONFIG_FREEIMAGE)
    set(OGRE_DEPS_FOUND FALSE)
  endif ()
  if (NOT FREETYPE_FOUND)
    set(OGRE_DEPS_FOUND FALSE)
  endif ()
  if (UNIX AND NOT X11_FOUND)
    set(OGRE_DEPS_FOUND FALSE)
  endif ()
  
  if (OGRE_CONFIG_THREADS)
    find_package(Boost COMPONENTS thread QUIET)
    if (NOT Boost_THREAD_FOUND)
      set(OGRE_DEPS_FOUND FALSE)
    endif ()
  endif ()
  
  if (NOT OGRE_DEPS_FOUND)
    pkg_message(OGRE "Could not find all required dependencies for the Ogre package.")
    set(OGRE_FOUND FALSE)
  endif ()
endif ()

if (NOT OGRE_FOUND)
  return()
endif ()


get_filename_component(OGRE_LIBRARY_DIR_REL "${OGRE_LIBRARY_REL}" PATH)
get_filename_component(OGRE_LIBRARY_DIR_DBG "${OGRE_LIBRARY_DBG}" PATH)
set(OGRE_LIBRARY_DIRS ${OGRE_LIBRARY_DIR_REL} ${OGRE_LIBRARY_DIR_DBG})


# look for Paging component
find_path(OGRE_Paging_INCLUDE_DIR NAMES OgrePage.h HINTS ${OGRE_INCLUDE_DIRS} ${OGRE_PREFIX_SOURCE} PATH_SUFFIXES Paging OGRE/Paging Components/Paging/include)
set(OGRE_Paging_LIBRARY_NAMES "OgrePaging${OGRE_LIB_SUFFIX}")
get_debug_names(OGRE_Paging_LIBRARY_NAMES)
find_library(OGRE_Paging_LIBRARY_REL NAMES ${OGRE_Paging_LIBRARY_NAMES} HINTS ${OGRE_LIBRARY_DIR_REL} PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel")
find_library(OGRE_Paging_LIBRARY_DBG NAMES ${OGRE_Paging_LIBRARY_NAMES_DBG} HINTS ${OGRE_LIBRARY_DIR_DBG} PATH_SUFFIXES "" "debug")
set(OGRE_Paging_LIBRARY_FWK ${OGRE_LIBRARY_FWK})
make_library_set(OGRE_Paging_LIBRARY)
if (OGRE_Paging_INCLUDE_DIR AND Ogre_Paging_LIBRARY)
  set(OGRE_Paging_FOUND TRUE)
  set(OGRE_Paging_INCLUDE_DIRS ${OGRE_Paging_INCLUDE_DIRS})
  set(OGRE_Paging_LIBRARIES ${OGRE_Paging_LIBRARIES})
endif ()
mark_as_advanced(OGRE_Paging_INCLUDE_DIR OGRE_Paging_LIBRARY_REL OGRE_Paging_LIBRARY_DBG OGRE_Paging_LIBRARY_FWK)

# look for Terrain component
find_path(OGRE_Terrain_INCLUDE_DIR NAMES OgreTerrain.h HINTS ${OGRE_INCLUDE_DIRS} ${OGRE_PREFIX_SOURCE} PATH_SUFFIXES Terrain OGRE/Terrain Components/Terrain/include)
set(OGRE_Terrain_LIBRARY_NAMES "OgreTerrain${OGRE_LIB_SUFFIX}")
get_debug_names(OGRE_Terrain_LIBRARY_NAMES)
find_library(OGRE_Terrain_LIBRARY_REL NAMES ${OGRE_Terrain_LIBRARY_NAMES} HINTS ${OGRE_LIBRARY_DIR_REL} PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel")
find_library(OGRE_Terrain_LIBRARY_DBG NAMES ${OGRE_Terrain_LIBRARY_NAMES_DBG} HINTS ${OGRE_LIBRARY_DIR_DBG} PATH_SUFFIXES "" "debug")
set(OGRE_Terrain_LIBRARY_FWK ${OGRE_LIBRARY_FWK})
make_library_set(OGRE_Terrain_LIBRARY)
if (OGRE_Terrain_INCLUDE_DIR AND Ogre_Terrain_LIBRARY)
  set(OGRE_Terrain_FOUND TRUE)
  set(OGRE_Terrain_INCLUDE_DIRS ${OGRE_Terrain_INCLUDE_DIRS})
  set(OGRE_Terrain_LIBRARIES ${OGRE_Terrain_LIBRARIES})
endif ()
mark_as_advanced(OGRE_Terrain_INCLUDE_DIR OGRE_Terrain_LIBRARY_REL OGRE_Terrain_LIBRARY_DBG OGRE_Terrain_LIBRARY_FWK)

# look for CEGUI Ogre Renderer
find_package(CEGUI QUIET)
if (CEGUI_FOUND)
  get_filename_component(CEGUI_LIBRARY_DIR_REL ${CEGUI_LIBRARY_REL} PATH)
  get_filename_component(CEGUI_LIBRARY_DIR_DBG ${CEGUI_LIBRARY_DBG} PATH)
  set(CEGUI_LIBRARY_DIRS ${CEGUI_LIBRARY_DIR_REL} ${CEGUI_LIBRARY_DIR_DBG})
  find_path(OGRE_CEGUIRenderer_INCLUDE_DIR NAMES OgreCEGUIRenderer.h 
    HINTS ${OGRE_INCLUDE_DIRS} ${OGRE_PREFIX_SOURCE} ${CEGUI_INCLUDE_DIRS} PATH_SUFFIXES CEGUIRenderer OGRE OGRE/CEGUIRenderer CEGUI Samples/Common/CEGUIRenderer/include)
  set(OGRE_CEGUIRenderer_LIBRARY_NAMES "CEGUIOgreRenderer${OGRE_LIB_SUFFIX}" "OgreCEGUIRenderer${OGRE_LIB_SUFFIX}" "OgreGUIRenderer${OGRE_LIB_SUFFIX}")
  get_debug_names(OGRE_CEGUIRenderer_LIBRARY_NAMES)
  set(OGRE_CEGUIRenderer_LIBRARY_FWK ${OGRE_LIBRARY_FWK})
  find_library(OGRE_CEGUIRenderer_LIBRARY_REL NAMES ${OGRE_CEGUIRenderer_LIBRARY_NAMES} HINTS ${OGRE_LIBRARY_DIRS} ${CEGUI_LIBRARY_DIRS}
    ${OGRE_FRAMEWORK_SEARCH_PATH} PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel") 
  find_library(OGRE_CEGUIRenderer_LIBRARY_DBG NAMES ${OGRE_CEGUIRenderer_LIBRARY_NAMES_DBG} HINTS ${OGRE_LIBRARY_DIRS} ${CEGUI_LIBRARY_DIRS}
    ${OGRE_FRAMEWORK_SEARCH_PATH} PATH_SUFFIXES "" "debug") 
  make_library_set(OGRE_CEGUIRenderer_LIBRARY)
  
  if (OGRE_CEGUIRenderer_INCLUDE_DIR AND OGRE_CEGUIRenderer_LIBRARY)
    set(OGRE_CEGUIRenderer_FOUND TRUE)
    set(OGRE_CEGUIRenderer_INCLUDE_DIRS ${OGRE_CEGUIRenderer_INCLUDE_DIR} ${CEGUI_INCLUDE_DIRS})
    set(OGRE_CEGUIRenderer_LIBRARIES ${OGRE_CEGUIRenderer_LIBRARY} ${CEGUI_LIBRARIES})
  endif()

  mark_as_advanced(OGRE_CEGUIRenderer_INCLUDE_DIR OGRE_CEGUIRenderer_LIBRARY_REL OGRE_CEGUIRenderer_LIBRARY_DBG OGRE_CEGUIRenderer_LIBRARY_FWK)
endif ()


#########################################################
# Find Ogre plugins
#########################################################

macro(ogre_find_plugin PLUGIN HEADER)
  # On Unix, the plugins might have no prefix
  if (CMAKE_FIND_LIBRARY_PREFIXES)
    set(TMP_CMAKE_LIB_PREFIX ${CMAKE_FIND_LIBRARY_PREFIXES})
    set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} "")
  endif()
  
  # strip RenderSystem_ or Plugin_ prefix from plugin name
  string(REPLACE "RenderSystem_" "" PLUGIN_TEMP ${PLUGIN})
  string(REPLACE "Plugin_" "" PLUGIN_NAME ${PLUGIN_TEMP})
  
  # header files for plugins are not usually needed, but find them anyway if they are present
  set(OGRE_PLUGIN_PATH_SUFFIXES
    PlugIns PlugIns/${PLUGIN_NAME} Plugins Plugins/${PLUGIN_NAME} ${PLUGIN} 
    RenderSystems RenderSystems/${PLUGIN_NAME} ${ARGN})
  find_path(OGRE_${PLUGIN}_INCLUDE_DIR NAMES ${HEADER} 
    HINTS ${OGRE_INCLUDE_DIRS} ${OGRE_PREFIX_SOURCE}  
    PATH_SUFFIXES ${OGRE_PLUGIN_PATH_SUFFIXES})
  # find link libraries for plugins
  set(OGRE_${PLUGIN}_LIBRARY_NAMES "${PLUGIN}${OGRE_LIB_SUFFIX}")
  get_debug_names(OGRE_${PLUGIN}_LIBRARY_NAMES)
  set(OGRE_${PLUGIN}_LIBRARY_FWK ${OGRE_LIBRARY_FWK})
  find_library(OGRE_${PLUGIN}_LIBRARY_REL NAMES ${OGRE_${PLUGIN}_LIBRARY_NAMES}
    HINTS ${OGRE_LIBRARY_DIRS} PATH_SUFFIXES "" OGRE opt release release/opt relwithdebinfo relwithdebinfo/opt minsizerel minsizerel/opt)
  find_library(OGRE_${PLUGIN}_LIBRARY_DBG NAMES ${OGRE_${PLUGIN}_LIBRARY_NAMES_DBG}
    HINTS ${OGRE_LIBRARY_DIRS} PATH_SUFFIXES "" OGRE opt debug debug/opt)
  make_library_set(OGRE_${PLUGIN}_LIBRARY)

  if (OGRE_${PLUGIN}_LIBRARY OR OGRE_${PLUGIN}_INCLUDE_DIR)
    set(OGRE_${PLUGIN}_FOUND TRUE)
    if (OGRE_${PLUGIN}_INCLUDE_DIR)
      set(OGRE_${PLUGIN}_INCLUDE_DIRS ${OGRE_${PLUGIN}_INCLUDE_DIR})
    endif()
    set(OGRE_${PLUGIN}_LIBRARIES ${OGRE_${PLUGIN}_LIBRARY})
  endif ()

  mark_as_advanced(OGRE_${PLUGIN}_INCLUDE_DIR OGRE_${PLUGIN}_LIBRARY_REL OGRE_${PLUGIN}_LIBRARY_DBG OGRE_${PLUGIN}_LIBRARY_FWK)

  # look for plugin dirs
  if (OGRE_${PLUGIN}_FOUND)
    if (NOT OGRE_PLUGIN_DIR_REL OR NOT OGRE_PLUGIN_DIR_DBG)
      if (WIN32)
        set(OGRE_PLUGIN_SEARCH_PATH_REL 
          ${OGRE_LIBRARY_DIR_REL}/..
          ${OGRE_LIBRARY_DIR_REL}/../..
        )
        set(OGRE_PLUGIN_SEARCH_PATH_DBG
          ${OGRE_LIBRARY_DIR_DBG}/..
          ${OGRE_LIBRARY_DIR_DBG}/../..
        )
        find_path(OGRE_PLUGIN_DIR_REL NAMES "${PLUGIN}.dll" HINTS ${OGRE_PLUGIN_SEARCH_PATH_REL}
          PATH_SUFFIXES "" bin bin/release bin/relwithdebinfo bin/minsizerel)
        find_path(OGRE_PLUGIN_DIR_DBG NAMES "${PLUGIN}_d.dll" HINTS ${OGRE_PLUGIN_SEARCH_PATH_DBG}
          PATH_SUFFIXES "" bin bin/debug)
      elseif (UNIX)
        get_filename_component(OGRE_PLUGIN_DIR_TMP ${OGRE_${PLUGIN}_LIBRARY_REL} PATH)
        set(OGRE_PLUGIN_DIR_REL ${OGRE_PLUGIN_DIR_TMP} CACHE STRING "Ogre plugin dir (release)")
        get_filename_component(OGRE_PLUGIN_DIR_TMP ${OGRE_${PLUGIN}_LIBRARY_DBG} PATH)
        set(OGRE_PLUGIN_DIR_DBG ${OGRE_PLUGIN_DIR_TMP} CACHE STRING "Ogre plugin dir (debug)")
      endif ()
    endif ()
  endif ()

  if (TMP_CMAKE_LIB_PREFIX)
    set(CMAKE_FIND_LIBRARY_PREFIXES ${TMP_CMAKE_LIB_PREFIX})
  endif ()
endmacro(ogre_find_plugin)

ogre_find_plugin(Plugin_PCZSceneManager OgrePCZSceneManager.h PCZ PlugIns/PCZSceneManager/include)
ogre_find_plugin(Plugin_OctreeZone OgreOctreeZone.h PCZ PlugIns/OctreeZone/include)
ogre_find_plugin(Plugin_BSPSceneManager OgreBspSceneManager.h PlugIns/BSPSceneManager/include)
ogre_find_plugin(Plugin_CgProgramManager OgreCgProgram.h PlugIns/CgProgramManager/include)
ogre_find_plugin(Plugin_OctreeSceneManager OgreOctreeSceneManager.h PlugIns/OctreeSceneManager/include)
ogre_find_plugin(Plugin_ParticleFX OgreParticleFXPrerequisites.h PlugIns/ParticleFX/include)
ogre_find_plugin(RenderSystem_GL OgreGLRenderSystem.h RenderSystems/GL/include)
ogre_find_plugin(RenderSystem_GLES OgreGLESRenderSystem.h RenderSystems/GLES/include)
ogre_find_plugin(RenderSystem_Direct3D9 OgreD3D9RenderSystem.h RenderSystems/Direct3D9/include)
ogre_find_plugin(RenderSystem_Direct3D10 OgreD3D10RenderSystem.h RenderSystems/Direct3D10/include)

if (OGRE_STATIC)
  # check if dependencies for plugins are met
  if (NOT DirectX_FOUND)
    set(OGRE_RenderSystem_Direct3D9_FOUND FALSE)
  endif ()
  if (NOT DirectX_D3D10_FOUND)
    set(OGRE_RenderSystem_Direct3D10_FOUND FALSE)
  endif ()
  if (NOT OPENGL_FOUND)
    set(OGRE_RenderSystem_GL_FOUND FALSE)
  endif ()
  if (NOT OPENGLES_FOUND)
    set(OGRE_RenderSystem_GLES_FOUND FALSE)
  endif ()
  if (NOT Cg_FOUND)
    set(OGRE_Plugin_CgProgramManager_FOUND FALSE)
  endif ()
  
  set(OGRE_RenderSystem_Direct3D9_LIBRARIES ${OGRE_RenderSystem_Direct3D9_LIBRARIES}
    ${DirectX_LIBRARIES}
  )
  set(OGRE_RenderSystem_Direct3D10_LIBRARIES ${OGRE_RenderSystem_Direct3D10_LIBRARIES}
    ${DirectX_D3D10_LIBRARIES}
  )
  set(OGRE_RenderSystem_GL_LIBRARIES ${OGRE_RenderSystem_GL_LIBRARIES}
    ${OPENGL_LIBRARIES}
  )
  set(OGRE_RenderSystem_GLES_LIBRARIES ${OGRE_RenderSystem_GLES_LIBRARIES}
    ${OPENGLES_LIBRARIES}
  )
  set(OGRE_Plugin_CgProgramManager_LIBRARIES ${OGRE_Plugin_CgProgramManager_LIBRARIES}
    ${Cg_LIBRARIES}
  )
endif ()

# look for the media directory
set(OGRE_MEDIA_SEARCH_PATH
  ${OGRE_LIBRARY_DIR_REL}/..
  ${OGRE_LIBRARY_DIR_DBG}/..
  ${OGRE_LIBRARY_DIR_REL}/../..
  ${OGRE_LIBRARY_DIR_DBG}/../..
  ${OGRE_PREFIX_SOURCE}
)
set(OGRE_MEDIA_SEARCH_SUFFIX
  Media
  media
  share/OGRE/media
  Samples/Media
)
clear_if_changed(OGRE_PREFIX_WATCH OGRE_MEDIA_DIR)
find_path(OGRE_MEDIA_DIR NAMES packs/OgreCore.zip HINTS ${OGRE_MEDIA_SEARCH_PATH}
  PATHS ${OGRE_PREFIX_PATH} PATH_SUFFIXES ${OGRE_MEDIA_SEARCH_SUFFIX})

