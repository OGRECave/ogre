# - Try to find FreeImage
# Once done, this will define
#
#  FreeImage_FOUND - system has FreeImage
#  FreeImage_INCLUDE_DIRS - the FreeImage include directories 
#  FreeImage_LIBRARIES - link these to use FreeImage

include(FindPkgMacros)
findpkg_begin(FreeImage)

set(FreeImage_LIBRARY_NAMES freeimage)
get_debug_names(FreeImage_LIBRARY_NAMES)

use_pkgconfig(FreeImage_PKGC freeimage)

find_path(FreeImage_INCLUDE_DIR NAMES FreeImage.h PATHS ${FreeImage_PKGC_INCLUDE_DIRS})
find_library(FreeImage_LIBRARY_REL NAMES ${FreeImage_LIBRARY_NAMES} PATHS ${FreeImage_PKGC_LIBRARY_DIRS})
find_library(FreeImage_LIBRARY_DBG NAMES ${FreeImage_LIBRARY_NAMES_DBG} PATHS ${FreeImage_PKGC_LIBRARY_DIRS})
make_library_set(FreeImage_LIBRARY)

findpkg_finish(FreeImage)

