#######################################################################
# Find all necessary and optional OGRE dependencies
#######################################################################

# OGRE_DEPENDENCIES_DIR can be used to specify a single base
# folder where the required dependencies may be found.
set(OGRE_DEPENDENCIES_DIR "${OGRE_SOURCE_DIR}/Dependencies" CACHE PATH "Path to OGRE dependencies")
set(DEP_PREFIX_SEARCH_DIR "${OGRE_DEPENDENCIES_DIR}")

# Set hardcoded path guesses for various platforms
if (WIN32 OR APPLE)
  set(DEP_INCLUDE_SEARCH_DIR "${OGRE_DEPENDENCIES_DIR}/include")
  set(DEP_LIB_SEARCH_DIR "${OGRE_DEPENDENCIES_DIR}/lib/Release")
  set(DEP_LIBD_SEARCH_DIR "${OGRE_DEPENDENCIES_DIR}/lib/Debug")
endif ()


if (UNIX)
  # Important - OS X registers as *both* UNIX and APPLE, so append
  set(DEP_INCLUDE_SEARCH_DIR ${DEP_INCLUDE_SEARCH_DIR} "/usr/local/include" ${OGRE_DEPENDENCIES_DIR}/include)
  set(DEP_LIB_SEARCH_DIR ${DEP_LIB_SEARCH_DIR} "/usr/local/lib" ${OGRE_DEPENDENCIES_DIR}/lib)
  set(DEP_LIBD_SEARCH_DIR ${DEP_LIBD_SEARCH_DIR} "/usr/local/lib" ${OGRE_DEPENDENCIES_DIR}/lib)
endif ()

# give guesses as hints to the find_package calls
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${DEP_INCLUDE_SEARCH_DIR})
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${DEP_LIB_SEARCH_DIR} ${DEP_LIBD_SEARCH_DIR})
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${DEP_PREFIX_SEARCH_DIR})
set(CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} ${OGRE_DEPENDENCIES_DIR})


#######################################################################
# Core dependencies
#######################################################################

# Find zlib
find_package(ZLIB)
macro_log_feature(ZLIB_FOUND "zlib" "Simple data compression library" "http://www.zlib.net" TRUE "" "")

# Find zziplib
find_package(ZZip)
macro_log_feature(ZZip_FOUND "zziplib" "Extract data from zip archives" "http://zziplib.sourceforge.net" TRUE "" "")

# Find FreeImage
find_package(FreeImage)
macro_log_feature(FreeImage_FOUND "freeimage" "Support for commonly used graphics image formats" "http://freeimage.sourceforge.net" FALSE "" "")

# Find FreeType
find_package(Freetype)
macro_log_feature(FREETYPE_FOUND "freetype" "Portable font engine" "http://www.freetype.org" TRUE "" "")

# Find X11
if (UNIX)
	find_package(X11)
	macro_log_feature(X11_FOUND "X11" "X Window system" "http://www.x.org" TRUE "" "")
	macro_log_feature(X11_Xt_FOUND "Xt" "X Toolkit" "http://www.x.org" TRUE "" "")
	find_library(XAW_LIBRARY NAMES Xaw Xaw7 PATHS ${DEP_LIB_SEARCH_DIR} ${X11_LIB_SEARCH_PATH})
	macro_log_feature(XAW_LIBRARY "Xaw" "X11 Athena widget set" "http://www.x.org" TRUE "" "")
  mark_as_advanced(XAW_LIBRARY)
endif ()


#######################################################################
# RenderSystem dependencies
#######################################################################

# Find OpenGL
find_package(OpenGL)
macro_log_feature(OPENGL_FOUND "opengl" "Support for the OpenGL render system" "" FALSE "" "")

# Find DirectX
if(WIN32)
	find_package(DirectX)
	macro_log_feature(DirectX_FOUND "DirectX" "Support for the DirectX render system" "http://msdn.microsoft.com/en-us/directx/" FALSE "" "")
endif()


#######################################################################
# Additional features
#######################################################################

# Find Cg
find_package(Cg)
macro_log_feature(Cg_FOUND "cg" "C for graphics shader language" "http://developer.nvidia.com/object/cg_toolkit.html" FALSE "" "")

# Find Boost
if (OGRE_STATIC)
  set(Boost_USE_STATIC_LIBS TRUE)
endif()
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.38.0" "1.38")
find_package(Boost COMPONENTS thread QUIET)
macro_log_feature(Boost_FOUND "boost-thread" "Used for threading support" "http://boost.org" FALSE "" "")


#######################################################################
# Samples dependencies
#######################################################################

# Find CEGUI
find_package(CEGUI)
macro_log_feature(CEGUI_FOUND "CEGUI" "GUI system used for some of the samples" "http://www.cegui.org.uk" FALSE "" "")

# Find OIS
find_package(OIS)
macro_log_feature(OIS_FOUND "OIS" "Input library needed for the samples" "http://sourceforge.net/projects/wgois" FALSE "" "")


#######################################################################
# Tools
#######################################################################

find_package(Doxygen)
macro_log_feature(DOXYGEN_FOUND "Doxygen" "Tool for building API documentation" "http://doxygen.org" FALSE "" "")

#######################################################################
# Tests
#######################################################################

find_package(CppUnit)
macro_log_feature(CppUnit_FOUND "CppUnit" "Library for performing unit tests" "http://cppunit.sourceforge.net" FALSE "" "")

# Display results, terminate if anything required is missing
MACRO_DISPLAY_FEATURE_LOG()


# Add library and include paths from the dependencies
include_directories(
  ${ZLIB_INCLUDE_DIRS}
  ${ZZip_INCLUDE_DIRS}
  ${FreeImage_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${CEGUI_INCLUDE_DIRS}
  ${OIS_INCLLUDE_DIRS}
  ${Cg_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS}
  ${X11_INCLUDE_DIR}
  ${DirectX_INCLUDE_DIRS}
  ${CppUnit_INCLUDE_DIRS}
)
link_directories(
  ${OPENGL_LIBRARY_DIRS}
  ${Cg_LIBRARY_DIRS}
  ${BOOST_LIBRARY_DIRS}
  ${X11_LIBRARY_DIRS}
  ${DirectX_LIBRARY_DIRS}
  ${CppUnit_LIBRARY_DIRS}
)
