# - Try to find Carbon
# Once done, this will define
#
#  Carbon_FOUND - system has Carbon
#  Carbon_INCLUDE_DIRS - the Carbon include directories 
#  Carbon_LIBRARIES - link these to use Carbon

include(FindPkgMacros)
findpkg_begin(Carbon)

# construct search paths
set(Carbon_PREFIX_PATH ${Carbon_HOME} $ENV{Carbon_HOME}
  ${OGRE_HOME} $ENV{OGRE_HOME})
create_search_paths(Carbon)
# redo search if prefix path changed
clear_if_changed(Carbon_PREFIX_PATH
  Carbon_LIBRARY_FWK
  Carbon_LIBRARY_REL
  Carbon_LIBRARY_DBG
  Carbon_INCLUDE_DIR
)

set(Carbon_LIBRARY_NAMES Carbon)
get_debug_names(Carbon_LIBRARY_NAMES)

use_pkgconfig(Carbon_PKGC Carbon)

findpkg_framework(Carbon)

find_path(Carbon_INCLUDE_DIR NAMES Carbon.h HINTS ${Carbon_INC_SEARCH_PATH} ${Carbon_PKGC_INCLUDE_DIRS} PATH_SUFFIXES Carbon)
find_library(Carbon_LIBRARY_REL NAMES ${Carbon_LIBRARY_NAMES} HINTS ${Carbon_LIB_SEARCH_PATH} ${Carbon_PKGC_LIBRARY_DIRS})
find_library(Carbon_LIBRARY_DBG NAMES ${Carbon_LIBRARY_NAMES_DBG} HINTS ${Carbon_LIB_SEARCH_PATH} ${Carbon_PKGC_LIBRARY_DIRS})
make_library_set(Carbon_LIBRARY)

findpkg_finish(Carbon)
add_parent_dir(Carbon_INCLUDE_DIRS Carbon_INCLUDE_DIR)

