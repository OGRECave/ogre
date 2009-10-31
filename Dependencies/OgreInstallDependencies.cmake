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

get_filename_component(OGRE_DEP_CUR_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

option(OGRE_INSTALL_DEPENDENCIES "Install dependencies needed for sample builds" FALSE)
# TODO: Move binary files to Dependencies/bin
set(OGRE_DEP_BIN_DIR ${OGRE_DEP_CUR_DIR}/../Samples/Common/bin)

if (OGRE_INSTALL_DEPENDENCIES)
  if (OGRE_STATIC)
    # for static builds, projects must link against all Ogre dependencies themselves, so copy full include and lib dir
    install(DIRECTORY ${OGRE_DEP_CUR_DIR}/include/ DESTINATION include)
    install(DIRECTORY ${OGRE_DEP_CUR_DIR}/lib/ DESTINATION lib)
    
  else ()
    # for non-static builds, we only need OIS for the samples
    install(DIRECTORY ${OGRE_DEP_CUR_DIR}/include/OIS   DESTINATION include)
    install(FILES
      ${OGRE_DEP_CUR_DIR}/lib/debug/OIS_d.lib
      DESTINATION lib/debug CONFIGURATIONS Debug
    )
    install(FILES
      ${OGRE_DEP_CUR_DIR}/lib/release/OIS.lib
      DESTINATION lib/release CONFIGURATIONS Release RelWithDebInfo MinSizeRel None ""
    )
  endif ()
    
  # copy the dependency DLLs to the right places
  install(FILES
    ${OGRE_DEP_BIN_DIR}/debug/OIS_d.dll
	  DESTINATION bin/debug CONFIGURATIONS Debug
  )
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/OIS.dll
	  DESTINATION bin/release CONFIGURATIONS Release None ""
  )  
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/OIS.dll
	  DESTINATION bin/relwithdebinfo CONFIGURATIONS RelWithDebInfo
  )  
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/OIS.dll
	  DESTINATION bin/minsizerel CONFIGURATIONS MinSizeRel
  )  
  if (OGRE_BUILD_PLUGIN_CG)
  install(FILES
    ${OGRE_DEP_BIN_DIR}/debug/cg.dll
	  DESTINATION bin/debug CONFIGURATIONS Debug
  )
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/cg.dll
	  DESTINATION bin/release CONFIGURATIONS Release None ""
  )  
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/cg.dll
	  DESTINATION bin/relwithdebinfo CONFIGURATIONS RelWithDebInfo
  )  
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/cg.dll
	  DESTINATION bin/minsizerel CONFIGURATIONS MinSizeRel
  )  
  endif ()
  
  # install GLES dlls
  if (OGRE_BUILD_RENDERSYSTEM_GLES)
	  install(FILES
		${OGRE_DEP_BIN_DIR}/debug/libgles_cm.dll
		  DESTINATION bin/debug CONFIGURATIONS Debug
	  )
	  install(FILES
		${OGRE_DEP_BIN_DIR}/release/libgles_cm.dll
		  DESTINATION bin/release CONFIGURATIONS Release None ""
	  )  
	  install(FILES
		${OGRE_DEP_BIN_DIR}/release/libgles_cm.dll
		  DESTINATION bin/relwithdebinfo CONFIGURATIONS RelWithDebInfo
	  )  
	  install(FILES
		${OGRE_DEP_BIN_DIR}/release/libgles_cm.dll
		  DESTINATION bin/minsizerel CONFIGURATIONS MinSizeRel
	  )  
  endif ()
endif ()

# copy the required DLLs to the build directory (configure_file is the only copy-like op I found in CMake)
configure_file(${OGRE_DEP_BIN_DIR}/debug/OIS_d.dll ${OGRE_BINARY_DIR}/bin/debug/OIS_d.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/OIS.dll ${OGRE_BINARY_DIR}/bin/release/OIS.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/OIS.dll ${OGRE_BINARY_DIR}/bin/relwithdebinfo/OIS.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/OIS.dll ${OGRE_BINARY_DIR}/bin/minsizerel/OIS.dll COPYONLY)

if (OGRE_BUILD_PLUGIN_CG)
	configure_file(${OGRE_DEP_BIN_DIR}/debug/cg.dll ${OGRE_BINARY_DIR}/bin/debug/cg.dll COPYONLY)
	configure_file(${OGRE_DEP_BIN_DIR}/release/cg.dll ${OGRE_BINARY_DIR}/bin/release/cg.dll COPYONLY)
	configure_file(${OGRE_DEP_BIN_DIR}/release/cg.dll ${OGRE_BINARY_DIR}/bin/relwithdebinfo/cg.dll COPYONLY)
	configure_file(${OGRE_DEP_BIN_DIR}/release/cg.dll ${OGRE_BINARY_DIR}/bin/minsizerel/cg.dll COPYONLY)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_GLES)
	configure_file(${OGRE_DEP_BIN_DIR}/debug/libgles_cm.dll ${OGRE_BINARY_DIR}/bin/debug/libgles_cm.dll COPYONLY)
	configure_file(${OGRE_DEP_BIN_DIR}/release/libgles_cm.dll ${OGRE_BINARY_DIR}/bin/release/libgles_cm.dll COPYONLY)
	configure_file(${OGRE_DEP_BIN_DIR}/release/libgles_cm.dll ${OGRE_BINARY_DIR}/bin/relwithdebinfo/libgles_cm.dll COPYONLY)
	configure_file(${OGRE_DEP_BIN_DIR}/release/libgles_cm.dll ${OGRE_BINARY_DIR}/bin/minsizerel/libgles_cm.dll COPYONLY)
endif ()

