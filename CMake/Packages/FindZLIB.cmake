# - Try to find ZLIB
# Once done, this will define
#
#  ZLIB_FOUND - system has ZLIB
#  ZLIB_INCLUDE_DIRS - the ZLIB include directories 
#  ZLIB_LIBRARIES - link these to use ZLIB

include(FindPkgMacros)
findpkg_begin(ZLIB)

set(ZLIB_LIBRARY_NAMES z zlib zdll)
get_debug_names(ZLIB_LIBRARY_NAMES)

use_pkgconfig(ZLIB_PKGC zzip-zlib-config)

find_path(ZLIB_INCLUDE_DIR NAMES zlib.h PATHS ${ZLIB_PKGC_INCLUDE_DIRS})
find_library(ZLIB_LIBRARY_REL NAMES ${ZLIB_LIBRARY_NAMES} PATHS ${ZLIB_PKGC_LIBRARY_DIRS})
find_library(ZLIB_LIBRARY_DBG NAMES ${ZLIB_LIBRARY_NAMES_DBG} PATHS ${ZLIB_PKGC_LIBRARY_DIRS})
make_library_set(ZLIB_LIBRARY)

findpkg_finish(ZLIB)

