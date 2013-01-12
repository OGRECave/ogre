# - Try to find iOS SDK
# Once done, this will define
#
#  iOSSDK_FOUND - system has the iOS SDK

include(FindPkgMacros)
findpkg_begin(iOS\ SDK)

# construct search paths
set(iOSSDK_PREFIX_PATH ${iOSSDK_HOME} $ENV{iOSSDK_HOME}
  ${OGRE_HOME} $ENV{OGRE_HOME})
create_search_paths(/Developer/Platforms /Applications/Xcode.app/Contents/Developer/Platforms)
# redo search if prefix path changed
clear_if_changed(iOSSDK_PREFIX_PATH
  iOSSDK_LIBRARY_FWK
  iOSSDK_LIBRARY_REL
  iOSSDK_LIBRARY_DBG
  iOSSDK_INCLUDE_DIR
)

set(iOSSDK_LIBRARY_NAMES UIKit)
get_debug_names(iOSSDK_LIBRARY_NAMES)

#use_pkgconfig(iOSSDK_PKGC iOSSDK)

#findpkg_framework(UIKit)

find_path(iOSSDK_INCLUDE_DIR iPhoneOS.platform /Developer/Platforms /Applications/Xcode.app/Contents/Developer/Platforms)

if (iOSSDK_INCLUDE_DIR)
  unset(iOSSDK_INCLUDE_DIR)
  set(iOSSDK_FOUND TRUE)
  message(STATUS "Found iOS SDK")
endif (iOSSDK_INCLUDE_DIR)
