#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find FreeType
# Once done, this will define
#
#  FREETYPE_FOUND - system has FreeType
#  FREETYPE_INCLUDE_DIRS - the FreeType include directories 
#  FREETYPE_LIBRARIES - link these to use FreeType

include(FindPkgMacros)
findpkg_begin(FREETYPE)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(FREETYPE_HOME)

# construct search paths
set(FREETYPE_PREFIX_PATH ${FREETYPE_HOME} ${ENV_FREETYPE_HOME})
create_search_paths(FREETYPE)
# redo search if prefix path changed
clear_if_changed(FREETYPE_PREFIX_PATH
  FREETYPE_LIBRARY_FWK
  FREETYPE_LIBRARY_REL
  FREETYPE_LIBRARY_DBG
  FREETYPE_INCLUDE_DIR
)

set(FREETYPE_LIBRARY_NAMES freetype freetype219 freetype235 freetype238)
get_debug_names(FREETYPE_LIBRARY_NAMES)

use_pkgconfig(FREETYPE_PKGC freetype2)

findpkg_framework(FREETYPE)

find_path(FREETYPE_INCLUDE_DIR NAMES freetype/freetype.h HINTS ${FREETYPE_INC_SEARCH_PATH} ${FREETYPE_PKGC_INCLUDE_DIRS} PATH_SUFFIXES freetype2)
find_library(FREETYPE_LIBRARY_REL NAMES ${FREETYPE_LIBRARY_NAMES} HINTS ${FREETYPE_LIB_SEARCH_PATH} ${FREETYPE_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" release relwithdebinfo minsizerel)
find_library(FREETYPE_LIBRARY_DBG NAMES ${FREETYPE_LIBRARY_NAMES_DBG} HINTS ${FREETYPE_LIB_SEARCH_PATH} ${FREETYPE_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" debug)
make_library_set(FREETYPE_LIBRARY)

findpkg_finish(FREETYPE)

