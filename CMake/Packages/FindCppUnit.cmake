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

# Add other places CppUnit might be found
set(CPPUNIT_INC_SEARCH_PATH "$ENV{CPPUNIT_HOME}/include")
set(CPPUNIT_LIB_SEARCH_PATH "$ENV{CPPUNIT_HOME}/lib")

find_path(CppUnit_INCLUDE_DIR NAMES cppunit/Test.h PATHS ${CppUnit_PKGC_INCLUDE_DIRS} ${CPPUNIT_INC_SEARCH_PATH})
find_library(CppUnit_LIBRARY_REL NAMES ${CppUnit_LIBRARY_NAMES} PATHS ${CppUnit_PKGC_LIBRARY_DIRS} ${CPPUNIT_LIB_SEARCH_PATH})
find_library(CppUnit_LIBRARY_DBG NAMES ${CppUnit_LIBRARY_NAMES_DBG} PATHS ${CppUnit_PKGC_LIBRARY_DIRS} ${CPPUNIT_LIB_SEARCH_PATH})
make_library_set(CppUnit_LIBRARY)

findpkg_finish(CppUnit)

