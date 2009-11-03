#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find ThreadingBuildingBlocks libraries
# Once done, this will define
#
#  TBB_FOUND - system has TBB
#  TBB_INCLUDE_DIRS - the TBB include directories 
#  TBB_LIBRARIES - link these to use TBB

include(FindPkgMacros)
findpkg_begin(TBB)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(TBB_HOME)
getenv_path(TBB_ROOT)
getenv_path(TBB_BASE)

# construct search paths
set(TBB_PREFIX_PATH 
  ${TBB_HOME} ${ENV_TBB_HOME} 
  ${TBB_ROOT} ${ENV_TBB_ROOT}
  ${TBB_BASE} ${ENV_TBB_BASE}
)
# redo search if prefix path changed
clear_if_changed(TBB_PREFIX_PATH
  TBB_LIBRARY_FWK
  TBB_LIBRARY_REL
  TBB_LIBRARY_DBG
  TBB_INCLUDE_DIR
)

create_search_paths(TBB)
set(TBB_INC_SEARCH_PATH ${TBB_INC_SEARCH_PATH} ${TBB_PREFIX_PATH})

# For Windows, let's assume that the user might be using the precompiled
# TBB packages from the main website. These use a rather awkward directory
# structure (at least for automatically finding the right files) depending
# on platform and compiler, but we'll do our best to accomodate it.
# Not adding the same effort for the precompiled linux builds, though. Those
# have different versions for CC compiler versions and linux kernels which
# will never adequately match the user's setup, so there is no feasible way
# to detect the "best" version to use. The user will have to manually
# select the right files. (Chances are the distributions are shipping their
# custom version of tbb, anyway, so the problem is probably nonexistant.)
if (WIN32)
  set(COMPILER_PREFIX "vc7.1")
  if (MSVC80)
    set(COMPILER_PREFIX "vc8")
  endif ()
  if (MSVC90)
    set(COMPILER_PREFIX "vc9")
  endif ()
  
  # for each prefix path, add ia32/64\${COMPILER_PREFIX}\lib to the lib search path
  foreach (dir ${TBB_PREFIX_PATH})
    if (CMAKE_CL_64)
      list(APPEND TBB_LIB_SEARCH_PATH ${dir}/ia64/${COMPILER_PREFIX}/lib)
      list(APPEND TBB_LIB_SEARCH_PATH ${dir}/intel64/${COMPILER_PREFIX}/lib)
    else ()
      list(APPEND TBB_LIB_SEARCH_PATH ${dir}/ia32/${COMPILER_PREFIX}/lib)
    endif ()
  endforeach ()
endif ()



set(TBB_LIBRARY_NAMES tbb)
get_debug_names(TBB_LIBRARY_NAMES)

# use_pkgconfig(TBB_PKGC TBB)

findpkg_framework(TBB)

find_path(TBB_INCLUDE_DIR NAMES tbb/tbb.h HINTS ${TBB_INC_SEARCH_PATH} ${TBB_PKGC_INCLUDE_DIRS})
find_library(TBB_LIBRARY_REL NAMES ${TBB_LIBRARY_NAMES} HINTS ${TBB_LIB_SEARCH_PATH} ${TBB_PKGC_LIBRARY_DIRS})
find_library(TBB_LIBRARY_DBG NAMES ${TBB_LIBRARY_NAMES_DBG} HINTS ${TBB_LIB_SEARCH_PATH} ${TBB_PKGC_LIBRARY_DIRS})
make_library_set(TBB_LIBRARY)

findpkg_finish(TBB)

if (NOT TBB_FOUND)
  return()
endif ()


# Look for TBB's malloc package
findpkg_begin(TBB_MALLOC)
set(TBB_MALLOC_LIBRARY_NAMES tbbmalloc)
get_debug_names(TBB_MALLOC_LIBRARY_NAMES)
find_path(TBB_MALLOC_INCLUDE_DIR NAMES tbb/tbb.h HINTS ${TBB_INCLUDE_DIR} ${TBB_INC_SEARCH_PATH} ${TBB_PKGC_INCLUDE_DIRS} )
find_library(TBB_MALLOC_LIBRARY_REL NAMES ${TBB_MALLOC_LIBRARY_NAMES} HINTS ${TBB_LIB_SEARCH_PATH} ${TBB_PKGC_LIBRARY_DIRS} )
find_library(TBB_MALLOC_LIBRARY_DBG NAMES ${TBB_MALLOC_LIBRARY_NAMES_DBG} HINTS ${TBB_LIB_SEARCH_PATH} ${TBB_PKGC_LIBRARY_DIRS} )
make_library_set(TBB_MALLOC_LIBRARY)
findpkg_finish(TBB_MALLOC)

# Look for TBB's malloc proxy package
findpkg_begin(TBB_MALLOC_PROXY)
set(TBB_MALLOC_PROXY_LIBRARY_NAMES tbbmalloc_proxy)
get_debug_names(TBB_MALLOC_PROXY_LIBRARY_NAMES)
find_path(TBB_MALLOC_PROXY_INCLUDE_DIR NAMES tbb/tbbmalloc_proxy.h HINTS ${TBB_INCLUDE_DIR} ${TBB_INC_SEARCH_PATH} ${TBB_PKGC_INCLUDE_DIRS})
find_library(TBB_MALLOC_PROXY_LIBRARY_REL NAMES ${TBB_MALLOC_PROXY_LIBRARY_NAMES} HINTS ${TBB_LIB_SEARCH_PATH} ${TBB_PKGC_LIBRARY_DIRS})
find_library(TBB_MALLOC_PROXY_LIBRARY_DBG NAMES ${TBB_MALLOC_PROXY_LIBRARY_NAMES_DBG} HINTS ${TBB_LIB_SEARCH_PATH} ${TBB_PKGC_LIBRARY_DIRS})
make_library_set(TBB_MALLOC_PROXY_LIBRARY)
findpkg_finish(TBB_MALLOC_PROXY)
