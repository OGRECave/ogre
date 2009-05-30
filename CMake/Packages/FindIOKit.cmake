#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find IOKit
# Once done, this will define
#
#  IOKit_FOUND - system has IOKit
#  IOKit_INCLUDE_DIRS - the IOKit include directories 
#  IOKit_LIBRARIES - link these to use IOKit

include(FindPkgMacros)
findpkg_begin(IOKit)

# construct search paths
set(IOKit_PREFIX_PATH ${IOKit_HOME} $ENV{IOKit_HOME}
  ${OGRE_HOME} $ENV{OGRE_HOME})
create_search_paths(IOKit)
# redo search if prefix path changed
clear_if_changed(IOKit_PREFIX_PATH
  IOKit_LIBRARY_FWK
  IOKit_LIBRARY_REL
  IOKit_LIBRARY_DBG
  IOKit_INCLUDE_DIR
)

set(IOKit_LIBRARY_NAMES IOKit)
get_debug_names(IOKit_LIBRARY_NAMES)

use_pkgconfig(IOKit_PKGC IOKit)

findpkg_framework(IOKit)

find_path(IOKit_INCLUDE_DIR NAMES IOKitLib.h HINTS ${IOKit_INC_SEARCH_PATH} ${IOKit_PKGC_INCLUDE_DIRS} PATH_SUFFIXES IOKit)
find_library(IOKit_LIBRARY_REL NAMES ${IOKit_LIBRARY_NAMES} HINTS ${IOKit_LIB_SEARCH_PATH} ${IOKit_PKGC_LIBRARY_DIRS})
find_library(IOKit_LIBRARY_DBG NAMES ${IOKit_LIBRARY_NAMES_DBG} HINTS ${IOKit_LIB_SEARCH_PATH} ${IOKit_PKGC_LIBRARY_DIRS})
make_library_set(IOKit_LIBRARY)
findpkg_finish(IOKit)
add_parent_dir(IOKit_INCLUDE_DIRS IOKit_INCLUDE_DIR)

