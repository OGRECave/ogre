# - Try to find OIS
# Once done, this will define
#
#  OIS_FOUND - system has OIS
#  OIS_INCLUDE_DIRS - the OIS include directories 
#  OIS_LIBRARIES - link these to use OIS

include(FindPkgMacros)
findpkg_begin(OIS)

set(OIS_LIBRARY_NAMES OIS)
get_debug_names(OIS_LIBRARY_NAMES)

use_pkgconfig(OIS_PKGC OIS)

# OIS might be installed alongside OGRE as a dependency
set(OIS_INC_SEARCH_PATH "$ENV{OGRE_HOME}/include")
set(OIS_LIB_SEARCH_PATH "$ENV{OGRE_HOME}/lib")

find_path(OIS_INCLUDE_DIR NAMES OIS.h PATHS ${OIS_PKGC_INCLUDE_DIRS} ${OIS_INC_SEARCH_PATH} PATH_SUFFIXES "OIS")
find_library(OIS_LIBRARY_REL NAMES ${OIS_LIBRARY_NAMES} PATHS ${OIS_PKGC_LIBRARY_DIRS} ${OIS_LIB_SEARCH_PATH})
find_library(OIS_LIBRARY_DBG NAMES ${OIS_LIBRARY_NAMES_DBG} PATHS ${OIS_PKGC_LIBRARY_DIRS} ${OIS_LIB_SEARCH_PATH})
make_library_set(OIS_LIBRARY)

findpkg_finish(OIS)
add_parent_dir(OIS_INCLUDE_DIRS OIS_INCLUDE_DIR)

