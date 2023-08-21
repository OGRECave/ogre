#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find Softimage SDK
# Once done, this will define
#
#  Softimage_FOUND - system has Softimage SDK
#  Softimage_INCLUDE_DIRS - the Softimage include directories 
#  Softimage_LIBRARIES - link these to use OIS

include(FindPkgMacros)
findpkg_begin(Softimage)

# construct search paths
set(Softimage_PREFIX_PATH
  "C:/Softimage/XSI_6.0/XSISDK"
  "C:/Softimage/XSI_6.01/XSISDK"
  "C:/Softimage/XSI_6.02/XSISDK"
  "C:/Softimage/XSI_6.5/XSISDK"
  "C:/Softimage/XSI_7.0/XSISDK"
  "C:/Softimage/XSI_7.01/XSISDK"
  "C:/Softimage/XSI_7.5/XSISDK"
  "C:/Softimage/Softimage_2010/XSISDK"
  "C:/Softimage/Softimage_2010_SP1/XSISDK"
  "$ENV{ProgramFiles}/Autodesk/Softimage 2011/XSISDK"
  "$ENV{ProgramFiles}/Autodesk/Softimage 2011 SP1/XSISDK"
  "$ENV{ProgramFiles}/Autodesk/Softimage 2012/XSISDK"
  "$ENV{ProgramFiles}/Autodesk/Softimage 2012 SP1/XSISDK"
  "$ENV{ProgramFiles}/Autodesk/Softimage 2013/XSISDK"
  "C:/Softimage/XSI_6.0_x64/XSISDK"
  "C:/Softimage/XSI_6.01_x64/XSISDK"
  "C:/Softimage/XSI_6.02_x64/XSISDK"
  "C:/Softimage/XSI_6.5_x64/XSISDK"
  "C:/Softimage/XSI_7.0_x64/XSISDK"
  "C:/Softimage/XSI_7.01_x64/XSISDK"
  "C:/Softimage/XSI_7.5_x64/XSISDK"
  "C:/Softimage/Softimage_2010_x64/XSISDK"
  "C:/Softimage/Softimage_2010_SP1_x64/XSISDK"
  "$ENV{ProgramW6432}/Autodesk/Softimage 2011/XSISDK"
  "$ENV{ProgramW6432}/Autodesk/Softimage 2011 SP1/XSISDK"
  "$ENV{ProgramW6432}/Autodesk/Softimage 2012/XSISDK"
  "$ENV{ProgramW6432}/Autodesk/Softimage 2012 SP1/XSISDK"
  "$ENV{ProgramW6432}/Autodesk/Softimage 2013/XSISDK"
)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(Softimage_LIBPATH_SUFFIX "nt-x86-64")
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(Softimage_LIBPATH_SUFFIX "nt-x86")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

create_search_paths(Softimage)
# redo search if prefix path changed
clear_if_changed(Softimage_PREFIX_PATH
  Softimage_LIBRARY
  Softimage_SICPPSDK_LIBRARY
  Softimage_INCLUDE_DIR
)

set(Softimage_LIBRARY_NAMES sicoresdk)
set(Softimage_SICPPSDK_LIBRARY_NAMES sicppsdk)

find_path(Softimage_INCLUDE_DIR NAMES xsisdk.h HINTS ${Softimage_INC_SEARCH_PATH})

find_library(Softimage_LIBRARY NAMES ${Softimage_LIBRARY_NAMES} HINTS ${Softimage_LIB_SEARCH_PATH} PATH_SUFFIXES ${Softimage_LIBPATH_SUFFIX})
find_library(Softimage_SICPPSDK_LIBRARY NAMES ${Softimage_SICPPSDK_LIBRARY_NAMES} HINTS ${Softimage_LIB_SEARCH_PATH} PATH_SUFFIXES ${Softimage_LIBPATH_SUFFIX})

findpkg_finish(Softimage)

set(Softimage_LIBRARIES ${Softimage_LIBRARIES}
  ${Softimage_SICPPSDK_LIBRARY}
)

mark_as_advanced(Softimage_SICPPSDK_LIBRARY)
