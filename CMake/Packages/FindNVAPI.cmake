#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find NVAPI
# Once done, this will define
#
#  NVAPI_FOUND - system has NVAPI
#  NVAPI_INCLUDE_DIRS - the NVAPI include directories 
#  NVAPI_LIBRARIES - link these to use NVAPI

include(FindPkgMacros)
findpkg_begin(NVAPI)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(OGRE_SOURCE)
getenv_path(OGRE_HOME)

# construct search paths
set(NVAPI_PREFIX_PATH ${OGRE_SOURCE}/Dependencies ${ENV_OGRE_SOURCE}/Dependencies ${OGRE_HOME} ${ENV_OGRE_HOME})

create_search_paths(NVAPI)

# redo search if prefix path changed
clear_if_changed(NVAPI_PREFIX_PATH
  NVAPI_LIBRARY_FWK
  NVAPI_LIBRARY_REL
  NVAPI_LIBRARY_DBG
  NVAPI_INCLUDE_DIR
)

set(NVAPI_LIBRARY_NAMES NVAPI)
get_debug_names(NVAPI_LIBRARY_NAMES)

use_pkgconfig(NVAPI_PKGC NVAPI)

findpkg_framework(NVAPI)

find_path(NVAPI_INCLUDE_DIR NAMES nvapi.h HINTS ${NVAPI_FRAMEWORK_INCLUDES} ${NVAPI_INC_SEARCH_PATH} ${NVAPI_PKGC_INCLUDE_DIRS} PATH_SUFFIXES NVAPI)
find_library(NVAPI_LIBRARY_REL NAMES ${NVAPI_LIBRARY_NAMES} HINTS ${NVAPI_LIB_SEARCH_PATH} ${NVAPI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" release relwithdebinfo minsizerel)
find_library(NVAPI_LIBRARY_DBG NAMES ${NVAPI_LIBRARY_NAMES_DBG} HINTS ${NVAPI_LIB_SEARCH_PATH} ${NVAPI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" debug)
make_library_set(NVAPI_LIBRARY)

findpkg_finish(NVAPI)
add_parent_dir(NVAPI_INCLUDE_DIRS NVAPI_INCLUDE_DIR)