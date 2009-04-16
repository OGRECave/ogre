# - Try to find CEGUI
# Once done, this will define
#
#  CEGUI_FOUND - system has CEGUI
#  CEGUI_INCLUDE_DIRS - the CEGUI include directories 
#  CEGUI_LIBRARIES - link these to use CEGUI

include(FindPkgMacros)
findpkg_begin(CEGUI)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(CEGUI_HOME)
getenv_path(OGRE_SOURCE)
getenv_path(OGRE_HOME)

# construct search paths
set(CEGUI_PREFIX_PATH ${CEGUI_HOME} ${ENV_CEGUI_HOME}
  ${OGRE_SOURCE}/Dependencies
  ${ENV_OGRE_SOURCE}/Dependencies
  ${OGRE_HOME} ${ENV_OGRE_HOME})
create_search_paths(CEGUI)
# redo search if prefix path changed
clear_if_changed(CEGUI_PREFIX_PATH
  CEGUI_LIBRARY_FWK
  CEGUI_LIBRARY_REL
  CEGUI_LIBRARY_DBG
  CEGUI_INCLUDE_DIR
)

set(CEGUI_LIBRARY_NAMES CEGUIBase CEGUI)
get_debug_names(CEGUI_LIBRARY_NAMES)

use_pkgconfig(CEGUI_PKGC CEGUI)

findpkg_framework(CEGUI)

find_path(CEGUI_INCLUDE_DIR NAMES CEGUI.h HINTS ${CEGUI_FRAMEWORK_INCLUDES} ${CEGUI_INC_SEARCH_PATH} ${CEGUI_PKGC_INCLUDE_DIRS} PATH_SUFFIXES CEGUI)
find_library(CEGUI_LIBRARY_REL NAMES ${CEGUI_LIBRARY_NAMES} HINTS ${CEGUI_LIB_SEARCH_PATH} ${CEGUI_PKGC_LIBRARY_DIRS})
find_library(CEGUI_LIBRARY_DBG NAMES ${CEGUI_LIBRARY_NAMES_DBG} HINTS ${CEGUI_LIB_SEARCH_PATH} ${CEGUI_PKGC_LIBRARY_DIRS})
make_library_set(CEGUI_LIBRARY)

findpkg_finish(CEGUI)
add_parent_dir(CEGUI_INCLUDE_DIRS CEGUI_INCLUDE_DIR)

