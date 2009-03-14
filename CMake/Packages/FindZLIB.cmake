# - Try to find ZLIB
# Once done, this will define
#
#  ZLIB_FOUND - system has ZLIB
#  ZLIB_INCLUDE_DIRS - the ZLIB include directories 
#  ZLIB_LIBRARIES - link these to use ZLIB

include(FindPkgMacros)
findpkg_begin(ZLIB)

# construct search paths
set(ZLIB_PREFIX_PATH ${ZLIB_HOME} $ENV{ZLIB_HOME})
create_search_paths(ZLIB)
# redo search if prefix path changed
clear_if_changed(ZLIB_PREFIX_PATH
  ZLIB_LIBRARY_FWK
  ZLIB_LIBRARY_REL
  ZLIB_LIBRARY_DBG
  ZLIB_INCLUDE_DIR
)

set(ZLIB_LIBRARY_NAMES z zlib zdll)
get_debug_names(ZLIB_LIBRARY_NAMES)

use_pkgconfig(ZLIB_PKGC zzip-zlib-config)

findpkg_framework(ZLIB)

find_path(ZLIB_INCLUDE_DIR NAMES zlib.h HINTS ${ZLIB_INC_SEARCH_PATH} ${ZLIB_PKGC_INCLUDE_DIRS})
find_library(ZLIB_LIBRARY_REL NAMES ${ZLIB_LIBRARY_NAMES} HINTS ${ZLIB_LIB_SEARCH_PATH} ${ZLIB_PKGC_LIBRARY_DIRS})
find_library(ZLIB_LIBRARY_DBG NAMES ${ZLIB_LIBRARY_NAMES_DBG} HINTS ${ZLIB_LIB_SEARCH_PATH} ${ZLIB_PKGC_LIBRARY_DIRS})
make_library_set(ZLIB_LIBRARY)

findpkg_finish(ZLIB)

