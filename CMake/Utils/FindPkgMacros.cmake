##################################################################
# Provides some common functionality for the FindPackage modules
##################################################################

# Begin processing of package
macro(findpkg_begin PREFIX)
  if (${PREFIX}_INCLUDE_DIR)
    set(${PREFIX}_FIND_QUIETLY TRUE)
  elseif (NOT ${PREFIX}_FIND_QUIETLY)
    message(STATUS "Looking for ${PREFIX}...")
  endif ()
endmacro(findpkg_begin)

# Try to get some hints from pkg-config, if available
macro(use_pkgconfig PREFIX PKGNAME)
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(${PREFIX} ${PKGNAME})
  endif ()
endmacro (use_pkgconfig)

# Try and look for a package config.cmake
function(findpkg_config PREFIX)
  if (${PREFIX}_CMAKE_INCLUDE_DIRS AND ${PREFIX}_CMAKE_LIBRARY_DIRS)
    return()
  endif()

  find_package(${PREFIX} QUIET NO_MODULE PATHS ${ARGN})
  mark_as_advanced(${PREFIX}_DIR)
  if (${PREFIX}_FOUND)
    set(${PREFIX}_CMAKE_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIRS} CACHE INTERNAL)
	foreach(i ${${PREFIX}_LIBRARIES)
	  string(TOUPPER ${i} li)
	  if (NOT li STREQUAL "OPTIMIZED" AND NOT li STREQUAL "DEBUG")
	    get_filename_component(p ${i} PATH)
	    set(${PREFIX}_CMAKE_LIBRARY_DIRS ${${PREFIX}_CMAKE_LIBRARY_DIRS} ${p} CACHE INTERNAL FORCE)
	  endif ()	  
	endforeach(i)
  endif ()
endfunction(findpkg_config)

# Couple a set of release AND debug libraries
macro(make_library_set PREFIX)
  if (${PREFIX}_REL AND ${PREFIX}_DBG)
    set(${PREFIX} optimized ${${PREFIX}_REL} debug ${${PREFIX}_DBG})
  elseif (${PREFIX}_REL)
    set(${PREFIX} ${${PREFIX}_REL})
  elseif (${PREFIX}_DBG)
    set(${PREFIX} ${${PREFIX}_DBG})
  endif ()
endmacro(make_library_set)

# Generate debug names from given release names
macro(get_debug_names PREFIX)
  foreach(i ${${PREFIX}})
    set(${PREFIX}_DBG ${${PREFIX}_DBG} ${i}d ${i}D ${i}_d ${i}_D)
  endforeach(i)
endmacro(get_debug_names)

# Add the parent dir from DIR to VAR
macro(add_parent_dir VAR DIR)
  if (${VAR})
    set(${VAR} ${${VAR}} "${${DIR}}/..")
  endif ()
endmacro(add_parent_dir)

# Do the final processing for the package find.
macro(findpkg_finish PREFIX)
  # skip if already processed during this run
  if (NOT ${PREFIX}_FOUND)
    if (${PREFIX}_INCLUDE_DIR AND ${PREFIX}_LIBRARY)
      set(${PREFIX}_FOUND TRUE)
      set(${PREFIX}_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIR})
      set(${PREFIX}_LIBRARIES ${${PREFIX}_LIBRARY})
      if (NOT ${PREFIX}_FIND_QUIETLY)
        message(STATUS "Found ${PREFIX}: ${${PREFIX}_LIBRARIES}")
      endif ()
    else ()
      if (NOT ${PREFIX}_FIND_QUIETLY)
        message(STATUS "Could not locate ${PREFIX}")
      endif ()
      if (${PREFIX}_FIND_REQUIRED)
        message(FATAL_ERROR "Required library ${PREFIX} not found! Install the library (including dev packages) and try again. If the library is already installed, set the missing variables manually in cmake.")
      endif ()
    endif ()

    mark_as_advanced(${PREFIX}_INCLUDE_DIR ${PREFIX}_LIBRARY ${PREFIX}_LIBRARY_REL ${PREFIX}_LIBRARY_DBG)
  endif ()
endmacro(findpkg_finish)
