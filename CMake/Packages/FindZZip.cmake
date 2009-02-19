# - Try to find zziplib
# Once done, this will define
#
#  ZZip_FOUND - system has ZZip
#  ZZip_INCLUDE_DIRS - the ZZip include directories 
#  ZZip_LIBRARIES - link these to use ZZip

include(FindPkgMacros)
findpkg_begin(ZZip)

set(ZZip_LIBRARY_NAMES zzip zziplib)
get_debug_names(ZZip_LIBRARY_NAMES)

use_pkgconfig(ZZip_PKGC zziplib)

find_path(ZZip_INCLUDE_DIR NAMES zzip/zzip.h PATHS ${ZZip_PKGC_INCLUDE_DIRS})
find_library(ZZip_LIBRARY_REL NAMES ${ZZip_LIBRARY_NAMES} PATHS ${ZZip_PKGC_LIBRARY_DIRS})
find_library(ZZip_LIBRARY_DBG NAMES ${ZZip_LIBRARY_NAMES_DBG} PATHS ${ZZip_PKGC_LIBRARY_DIRS})
make_library_set(ZZip_LIBRARY)

findpkg_finish(ZZip)

