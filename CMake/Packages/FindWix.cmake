#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find Wix
# You can help this by defining WIX_HOME in the environment / CMake
# Once done, this will define
#
#  Wix_FOUND - system has Wix
#  Wix_BINARY_DIR - location of the Wix binaries

include(FindPkgMacros)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(WIX_HOME)

# construct search paths
set(WIX_PREFIX_PATH ${WIX_HOME} ${ENV_WIX_HOME}
	"C:/Program Files/Windows Installer XML Toolset 3.0"
)
find_path(Wix_BINARY_DIR NAMES candle.exe HINTS ${WIX_PREFIX_PATH} PATH_SUFFIXES bin)

if(Wix_BINARY_DIR)
	set (Wix_FOUND TRUE)
endif()

mark_as_advanced(Wix_BINARY_DIR Wix_FOUND)