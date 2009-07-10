# - Try to find iPhone SDK
# Once done, this will define
#
#  iPhoneSDK_FOUND - system has the iPhone SDK

include(FindPkgMacros)
findpkg_begin(iPhone\ SDK)

# construct search paths
set(iPhoneSDK_PREFIX_PATH ${iPhoneSDK_HOME} $ENV{iPhoneSDK_HOME}
  ${OGRE_HOME} $ENV{OGRE_HOME})
create_search_paths(/Developer/Platforms)
# redo search if prefix path changed
clear_if_changed(iPhoneSDK_PREFIX_PATH
  iPhoneSDK_LIBRARY_FWK
  iPhoneSDK_LIBRARY_REL
  iPhoneSDK_LIBRARY_DBG
  iPhoneSDK_INCLUDE_DIR
)

set(iPhoneSDK_LIBRARY_NAMES UIKit)
get_debug_names(iPhoneSDK_LIBRARY_NAMES)

#use_pkgconfig(iPhoneSDK_PKGC iPhoneSDK)

#findpkg_framework(UIKit)

find_path(iPhoneSDK_INCLUDE_DIR iPhoneOS.platform /Developer/Platforms)

if (iPhoneSDK_INCLUDE_DIR)
  unset(iPhoneSDK_INCLUDE_DIR)
  set(iPhoneSDK_FOUND TRUE)
  message(STATUS "Found iPhone SDK")
endif (iPhoneSDK_INCLUDE_DIR)
