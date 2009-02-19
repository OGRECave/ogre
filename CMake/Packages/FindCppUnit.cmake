# - Try to find CppUnit
# Once done, this will define
#
#  CppUnit_FOUND - system has CppUnit
#  CppUnit_INCLUDE_DIRS - the CppUnit include directories 
#  CppUnit_LIBRARIES - link these to use CppUnit

include(FindPkgMacros)
findpkg_begin(CppUnit)

set(CppUnit_LIBRARY_NAMES cppunit)
get_debug_names(CppUnit_LIBRARY_NAMES)

use_pkgconfig(CppUnit_PKGC cppunit)

find_path(CppUnit_INCLUDE_DIR NAMES cppunit/Test.h PATHS ${CppUnit_PKGC_INCLUDE_DIRS})
find_library(CppUnit_LIBRARY_REL NAMES ${CppUnit_LIBRARY_NAMES} PATHS ${CppUnit_PKGC_LIBRARY_DIRS})
find_library(CppUnit_LIBRARY_DBG NAMES ${CppUnit_LIBRARY_NAMES_DBG} PATHS ${CppUnit_PKGC_LIBRARY_DIRS})
make_library_set(CppUnit_LIBRARY)

findpkg_finish(CppUnit)
add_parent_dir(CppUnit_INCLUDE_DIRS CppUnit_INCLUDE_DIR)

