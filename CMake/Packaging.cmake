#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

set(CPACK_PACKAGE_VERSION ${OGRE_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${OGRE_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${OGRE_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${OGRE_VERSION_PATCH})

set(CPACK_INSTALL_CMAKE_PROJECTS "${OGRE_BINARY_DIR}" "OGRE" "ALL" "/")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Source 3D Graphics Engine")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OGRE")
set(CPACK_PACKAGE_NAME "OGRE")
set(CPACK_PACKAGE_VENDOR "Torus Knot Software")

# CPack won't allow file without recognized extension to be used as
# license file.
configure_file("${OGRE_SOURCE_DIR}/COPYING" "${OGRE_BINARY_DIR}/COPYING.txt" COPYONLY)
set(CPACK_RESOURCE_FILE_LICENSE "${OGRE_BINARY_DIR}/COPYING.txt")

#set(CPACK_PACKAGE_ICON "${OGRE_SOURCE_DIR}\\\\ogrelogo.gif")

set(CPACK_PACKAGE_CONTACT "OGRE Team <webmaster@ogre3d.org>")

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "WriteRegStr \\\${WriteEnvStr_RegKey} \\\"OGRE_HOME\\\" $INSTDIR")

include(CPack)
