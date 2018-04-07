#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

#############################################################
# Install useful CMake modules.
# These are necessary to compile the samples from the install
# directory, but can also be used for custom projects.
#############################################################

if(WIN32 OR APPLE)
	set(OGRE_CMAKE_DIR "CMake")
else()
	set(OGRE_CMAKE_DIR "${OGRE_LIB_DIRECTORY}/OGRE/cmake")
endif()

set(INST_FILES Utils/FindPkgMacros.cmake) 

if (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "poco")
  set(INST_FILES ${INST_FILES} Packages/FindPOCO.cmake)
endif ()

if (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "tbb")
  set(INST_FILES ${INST_FILES} Packages/FindTBB.cmake)
endif ()

set(STATIC_INST_FILES
  Packages/FindCg.cmake
  Packages/FindDirectX.cmake
  Packages/FindDirectX11.cmake
  Packages/FindFreeImage.cmake
  Packages/FindOpenGLES2.cmake
  Packages/FindZZip.cmake
  Packages/FindSoftimage.cmake
  Packages/FindGLSLOptimizer.cmake
  Packages/FindHLSL2GLSL.cmake
)
if (WIN32)
  set(INST_FILES ${INST_FILES}
    Templates/VisualStudioUserFile.vcproj.user.in
    Templates/VisualStudioUserFile.vcxproj.user.in
  )
endif ()

if (APPLE)
  set(INST_FILES ${INST_FILES}
    Packages/FindCg.cmake
  )
  set(STATIC_INST_FILES ${STATIC_INST_FILES}
    Packages/FindGLSLOptimizer.cmake
    Packages/FindHLSL2GLSL.cmake
  )
endif ()

install(FILES ${INST_FILES} DESTINATION ${OGRE_CMAKE_DIR})
if (OGRE_STATIC)
  install(FILES ${STATIC_INST_FILES} DESTINATION ${OGRE_CMAKE_DIR})
endif ()
