#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find Cocoa
# Once done, this will define
#
#  Cocoa_FOUND - system has Cocoa
#  Cocoa_INCLUDE_DIRS - the Cocoa include directories 
#  Cocoa_LIBRARIES - link these to use Cocoa

include(FindPkgMacros)
findpkg_begin(Cocoa)

# construct search paths
set(Cocoa_PREFIX_PATH ${Cocoa_HOME} $ENV{Cocoa_HOME}
  ${OGRE_HOME} $ENV{OGRE_HOME})
create_search_paths(Cocoa)
# redo search if prefix path changed
clear_if_changed(Cocoa_PREFIX_PATH
  Cocoa_LIBRARY_FWK
  Cocoa_LIBRARY_REL
  Cocoa_LIBRARY_DBG
  Cocoa_INCLUDE_DIR
)

set(Cocoa_LIBRARY_NAMES Cocoa)
get_debug_names(Cocoa_LIBRARY_NAMES)

use_pkgconfig(Cocoa_PKGC Cocoa)

findpkg_framework(Cocoa)

find_path(Cocoa_INCLUDE_DIR NAMES Cocoa.h HINTS ${Cocoa_INC_SEARCH_PATH} ${Cocoa_PKGC_INCLUDE_DIRS} PATH_SUFFIXES Cocoa)
find_library(Cocoa_LIBRARY_REL NAMES ${Cocoa_LIBRARY_NAMES} HINTS ${Cocoa_LIB_SEARCH_PATH} ${Cocoa_PKGC_LIBRARY_DIRS})
find_library(Cocoa_LIBRARY_DBG NAMES ${Cocoa_LIBRARY_NAMES_DBG} HINTS ${Cocoa_LIB_SEARCH_PATH} ${Cocoa_PKGC_LIBRARY_DIRS})
make_library_set(Cocoa_LIBRARY)

findpkg_finish(Cocoa)
add_parent_dir(Cocoa_INCLUDE_DIRS Cocoa_INCLUDE_DIR)

