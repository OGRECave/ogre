#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

#####################################################
# Install dependencies on Windows
#####################################################

if (NOT WIN32)
  return()
endif()

# determine if we have a common dependencies folder to install from.
# this is not a fool-proof test, but it should be good enough.
get_filename_component(OGRE_DEP1_DIR ${FREETYPE_INCLUDE_DIR}/../ ABSOLUTE)
get_filename_component(OGRE_DEP2_DIR ${ZLIB_INCLUDE_DIR}/../ ABSOLUTE)
if (OGRE_DEP1_DIR STREQUAL OGRE_DEP2_DIR)
  set(OGRE_DEP_DIR ${OGRE_DEP1_DIR})
else ()
  return()
endif ()

option(OGRE_INSTALL_DEPENDENCIES "Install dependency DLLs needed for samples" TRUE)
option(OGRE_COPY_DEPENDENCIES "Copy dependency DLLs to the build directory" TRUE)

macro(install_debug INPUT)
  if (EXISTS ${OGRE_DEP_DIR}/bin/debug/${INPUT})
    install(FILES ${OGRE_DEP_DIR}/bin/debug/${INPUT} DESTINATION bin/debug CONFIGURATIONS Debug)
  endif ()
endmacro()

macro(install_release INPUT)
  if (EXISTS ${OGRE_DEP_DIR}/bin/release/${INPUT})
    install(FILES ${OGRE_DEP_DIR}/bin/release/${INPUT} DESTINATION bin/release CONFIGURATIONS Release None "")
	install(FILES ${OGRE_DEP_DIR}/bin/release/${INPUT} DESTINATION bin/relwithdebinfo CONFIGURATIONS RelWithDebInfo)
	install(FILES ${OGRE_DEP_DIR}/bin/release/${INPUT} DESTINATION bin/minsizerel CONFIGURATIONS MinSizeRel)
  endif ()
endmacro()

macro(copy_debug INPUT)
  if (EXISTS ${OGRE_DEP_DIR}/bin/debug/${INPUT})
    configure_file(${OGRE_DEP_DIR}/bin/debug/${INPUT} ${OGRE_BINARY_DIR}/bin/debug/${INPUT} COPYONLY)
  endif ()
endmacro()

macro(copy_release INPUT)
  if (EXISTS ${OGRE_DEP_DIR}/bin/release/${INPUT})
    configure_file(${OGRE_DEP_DIR}/bin/release/${INPUT} ${OGRE_BINARY_DIR}/bin/release/${INPUT} COPYONLY)
    configure_file(${OGRE_DEP_DIR}/bin/release/${INPUT} ${OGRE_BINARY_DIR}/bin/relwithdebinfo/${INPUT} COPYONLY)
    configure_file(${OGRE_DEP_DIR}/bin/release/${INPUT} ${OGRE_BINARY_DIR}/bin/minsizerel/${INPUT} COPYONLY)
  endif ()
endmacro ()


if (OGRE_INSTALL_DEPENDENCIES)
  if (OGRE_STATIC)
    # for static builds, projects must link against all Ogre dependencies themselves, so copy full include and lib dir
    if (EXISTS ${OGRE_DEP_DIR}/include/)
	  install(DIRECTORY ${OGRE_DEP_DIR}/include/ DESTINATION include)
	endif ()
	if (EXISTS ${OGRE_DEP_DIR}/lib/)
      install(DIRECTORY ${OGRE_DEP_DIR}/lib/ DESTINATION lib)
	endif ()
    
  else ()
    # for non-static builds, we only need OIS for the samples
	if (EXISTS ${OGRE_DEP_DIR}/include/OIS/)
      install(DIRECTORY ${OGRE_DEP_DIR}/include/OIS   DESTINATION include)
	endif ()
	if (EXISTS ${OGRE_DEP_DIR}/lib/debug/OIS_d.lib)
      install(FILES
        ${OGRE_DEP_DIR}/lib/debug/OIS_d.lib
        DESTINATION lib/debug CONFIGURATIONS Debug
      )
	endif ()
	if (EXISTS ${OGRE_DEP_DIR}/lib/release/OIS.lib)
      install(FILES
        ${OGRE_DEP_DIR}/lib/release/OIS.lib
        DESTINATION lib/release CONFIGURATIONS Release RelWithDebInfo MinSizeRel None ""
      )
	endif ()
  endif ()
    
  # copy the dependency DLLs to the right places
  install_debug(OIS_d.dll)
  install_release(OIS.dll)
  install_debug(OIS_d.dll)
  install_release(OIS.dll)
  if (OGRE_BUILD_PLUGIN_CG)
    install_debug(cg.dll)
	install_release(cg.dll)
  endif ()
  
  # install GLES dlls
  if (OGRE_BUILD_RENDERSYSTEM_GLES)
    install_debug(libgles_cm.dll)
	install_release(libgles_cm.dll)
  endif ()
  
  # install GLES2 dlls
  if (OGRE_BUILD_RENDERSYSTEM_GLES2)
    install_debug(libEGL.dll)
    install_debug(libGLESv2.dll)
	install_release(libEGL.dll)
	install_release(libGLESv2.dll)
  endif ()
  
endif ()

if (OGRE_COPY_DEPENDENCIES)
  # copy the required DLLs to the build directory (configure_file is the only copy-like op I found in CMake)
  copy_debug(OIS_d.dll)
  copy_release(OIS.dll)

  if (OGRE_BUILD_PLUGIN_CG)
    copy_debug(cg.dll)
	copy_release(cg.dll)
  endif ()

  if (OGRE_BUILD_RENDERSYSTEM_GLES)
    copy_debug(libgles_cm.dll)
	copy_release(libgles_cm.dll)
  endif ()

  if (OGRE_BUILD_RENDERSYSTEM_GLES2)	
	copy_debug(libEGL.dll)
    copy_debug(libGLESv2.dll)
	copy_release(libEGL.dll)
	copy_release(libGLESv2.dll)
  endif ()

endif ()