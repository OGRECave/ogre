# - Try to find FreeType
# Once done, this will define
#
#  FREETYPE_FOUND - system has FreeType
#  FREETYPE_INCLUDE_DIRS - the FreeType include directories 
#  FREETYPE_LIBRARIES - link these to use FreeType

include(FindPkgMacros)
findpkg_begin(FREETYPE)

set(FREETYPE_LIBRARY_NAMES freetype freetype219 freetype235 freetype238)
get_debug_names(FREETYPE_LIBRARY_NAMES)

use_pkgconfig(FREETYPE_PKGC freetype2)

find_path(FREETYPE_INCLUDE_DIR NAMES freetype/freetype.h PATHS ${FREETYPE_PKGC_INCLUDE_DIRS} PATH_SUFFIXES freetype2)
find_library(FREETYPE_LIBRARY_REL NAMES ${FREETYPE_LIBRARY_NAMES} PATHS ${FREETYPE_PKGC_LIBRARY_DIRS})
find_library(FREETYPE_LIBRARY_DBG NAMES ${FREETYPE_LIBRARY_NAMES_DBG} PATHS ${FREETYPE_PKGC_LIBRARY_DIRS})
make_library_set(FREETYPE_LIBRARY)

findpkg_finish(FREETYPE)

