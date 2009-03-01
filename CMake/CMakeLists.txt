#############################################################
# Install useful CMake modules.
# These are necessary to compile the samples from the install
# directory, but can also be used for custom projects.
#############################################################

if(WIN32)
	set(OGRE_CMAKE_DIR "CMake")
else(WIN32)
	set(OGRE_CMAKE_DIR "lib/cmake")
endif(WIN32)

set(INST_FILES
  Packages/FindCEGUI.cmake
  Packages/FindOIS.cmake
  Packages/FindOGRE.cmake
  Utils/FindPkgMacros.cmake
  Utils/OgreInstall.cmake
  Utils/OgreConfigBuild.cmake
  Utils/OgreGetVersion.cmake
  Templates/VisualStudioUserFile.vcproj.user.in
) 
install(FILES ${INST_FILES} DESTINATION ${OGRE_CMAKE_DIR})
