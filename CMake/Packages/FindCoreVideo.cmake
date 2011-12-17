#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find CoreVideo
# Once done, this will define
#
#  CoreVideo_FOUND - system has CoreVideo
#  CoreVideo_INCLUDE_DIRS - the CoreVideo include directories 
#  CoreVideo_LIBRARIES - link these to use CoreVideo

include(FindPkgMacros)
findpkg_begin(CoreVideo)

# construct search paths
set(CoreVideo_PREFIX_PATH ${CoreVideo_HOME} $ENV{CoreVideo_HOME}
  ${OGRE_HOME} $ENV{OGRE_HOME})
create_search_paths(CoreVideo)
# redo search if prefix path changed
clear_if_changed(CoreVideo_PREFIX_PATH
  CoreVideo_LIBRARY_FWK
  CoreVideo_LIBRARY_REL
  CoreVideo_LIBRARY_DBG
  CoreVideo_INCLUDE_DIR
)

set(CoreVideo_LIBRARY_NAMES CoreVideo)
get_debug_names(CoreVideo_LIBRARY_NAMES)

use_pkgconfig(CoreVideo_PKGC CoreVideo)

findpkg_framework(CoreVideo)

find_path(CoreVideo_INCLUDE_DIR NAMES CoreVideo.h HINTS ${CoreVideo_INC_SEARCH_PATH} ${CoreVideo_PKGC_INCLUDE_DIRS} PATH_SUFFIXES CoreVideo)
find_library(CoreVideo_LIBRARY_REL NAMES ${CoreVideo_LIBRARY_NAMES} HINTS ${CoreVideo_LIB_SEARCH_PATH} ${CoreVideo_PKGC_LIBRARY_DIRS})
find_library(CoreVideo_LIBRARY_DBG NAMES ${CoreVideo_LIBRARY_NAMES_DBG} HINTS ${CoreVideo_LIB_SEARCH_PATH} ${CoreVideo_PKGC_LIBRARY_DIRS})
make_library_set(CoreVideo_LIBRARY)

findpkg_finish(CoreVideo)
add_parent_dir(CoreVideo_INCLUDE_DIRS CoreVideo_INCLUDE_DIR)

