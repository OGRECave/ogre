#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

#####################################################
# Install dependencies 
#####################################################
if ((NOT APPLE) AND (NOT WIN32))
  return()
endif()

# TODO - most of this file assumes a common dependencies root folder
# This is not robust, we should instead source dependencies from their individual locations
get_filename_component(OGRE_DEP_DIR ${OIS_INCLUDE_DIR}/../../ ABSOLUTE)

option(OGRE_INSTALL_DEPENDENCIES "Install dependency libs needed for samples" TRUE)
option(OGRE_COPY_DEPENDENCIES "Copy dependency libs to the build directory" TRUE)

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
    if (MINGW OR NMAKE)
      configure_file(${OGRE_DEP_DIR}/bin/debug/${INPUT} ${OGRE_BINARY_DIR}/bin/${INPUT} COPYONLY)
	else ()
      configure_file(${OGRE_DEP_DIR}/bin/debug/${INPUT} ${OGRE_BINARY_DIR}/bin/debug/${INPUT} COPYONLY)
	endif ()
  endif ()
endmacro()

macro(copy_release INPUT)
  if (EXISTS ${OGRE_DEP_DIR}/bin/release/${INPUT})
    if (MINGW OR NMAKE)
      configure_file(${OGRE_DEP_DIR}/bin/release/${INPUT} ${OGRE_BINARY_DIR}/bin/${INPUT} COPYONLY)
	else ()
      configure_file(${OGRE_DEP_DIR}/bin/release/${INPUT} ${OGRE_BINARY_DIR}/bin/release/${INPUT} COPYONLY)
      configure_file(${OGRE_DEP_DIR}/bin/release/${INPUT} ${OGRE_BINARY_DIR}/bin/relwithdebinfo/${INPUT} COPYONLY)
      configure_file(${OGRE_DEP_DIR}/bin/release/${INPUT} ${OGRE_BINARY_DIR}/bin/minsizerel/${INPUT} COPYONLY)
	endif ()
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
	if(WIN32)
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
	  if (MINGW)
		install(FILES ${OIS_LIBRARY_DBG} DESTINATION lib/debug CONFIGURATIONS Debug)
		install(FILES ${OIS_LIBRARY_REL} DESTINATION lib/release CONFIGURATIONS Release)
	  endif ()
	elseif(APPLE)
	  if (EXISTS ${OGRE_DEP_DIR}/lib/debug/libOIS.a)
	        install(FILES
	          ${OGRE_DEP_DIR}/lib/debug/libOIS.a
	          DESTINATION lib/debug CONFIGURATIONS Debug
	        )
	  endif ()
	  if (EXISTS ${OGRE_DEP_DIR}/lib/release/libOIS.a)
	        install(FILES
	          ${OGRE_DEP_DIR}/lib/release/libOIS.a
	          DESTINATION lib/release CONFIGURATIONS Release RelWithDebInfo MinSizeRel None ""
	        )
	  endif ()
	endif ()
	  endif ()
    
  if(WIN32)
    # copy the dependency DLLs to the right places
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
  elseif(APPLE)
    # copy the dependency libs to the right places
    install_debug(libOIS.a)
    install_release(libOIS.a)

    if (OGRE_BUILD_PLUGIN_CG)
      install_debug(Cg.framework)
	  install_release(Cg.framework)
    endif ()
  endif ()
  
  # If we're installing the sample source for an SDK, also install Boost headers & libraries
  if (OGRE_INSTALL_SAMPLES_SOURCE)
	if (Boost_FOUND)
	  # headers (try to exclude things we don't need)
	  install(DIRECTORY "${Boost_INCLUDE_DIR}/boost" DESTINATION "boost_${Boost_LIB_VERSION}"
		PATTERN "accumulators" EXCLUDE
		PATTERN "algorithm" EXCLUDE
		PATTERN "archive" EXCLUDE
		PATTERN "asio" EXCLUDE
		PATTERN "assign" EXCLUDE
		PATTERN "bimap" EXCLUDE
		PATTERN "circular_buffer" EXCLUDE
		PATTERN "compatibility" EXCLUDE
		PATTERN "concept" EXCLUDE
		PATTERN "concept_check" EXCLUDE
		PATTERN "dynamic_bitset" EXCLUDE
		PATTERN "filesystem" EXCLUDE
		PATTERN "flyweight" EXCLUDE
		PATTERN "format" EXCLUDE
		PATTERN "functional" EXCLUDE
		PATTERN "fusion" EXCLUDE
		PATTERN "gil" EXCLUDE
		PATTERN "graph" EXCLUDE
		PATTERN "interprocess" EXCLUDE
		PATTERN "intrusive" EXCLUDE
		PATTERN "io" EXCLUDE
		PATTERN "iostreams" EXCLUDE
		PATTERN "lambda" EXCLUDE
		PATTERN "logic" EXCLUDE
		PATTERN "mpi" EXCLUDE
		PATTERN "multi_array" EXCLUDE
		PATTERN "multi_index" EXCLUDE
		PATTERN "numeric" EXCLUDE
		PATTERN "parameter" EXCLUDE
		PATTERN "pending" EXCLUDE
		PATTERN "pool" EXCLUDE
		PATTERN "program_options" EXCLUDE
		PATTERN "property_map" EXCLUDE
		PATTERN "property_tree" EXCLUDE
		PATTERN "proto" EXCLUDE
		PATTERN "ptr_container" EXCLUDE
		PATTERN "python" EXCLUDE
		PATTERN "random" EXCLUDE
		PATTERN "regex" EXCLUDE
		PATTERN "serialization" EXCLUDE
		PATTERN "signals" EXCLUDE
		PATTERN "signals2" EXCLUDE
		PATTERN "spirit" EXCLUDE
		PATTERN "statechart" EXCLUDE
		PATTERN "system" EXCLUDE
		PATTERN "test" EXCLUDE
		PATTERN "tr1" EXCLUDE
		PATTERN "typeof" EXCLUDE
		PATTERN "units" EXCLUDE
		PATTERN "unordered" EXCLUDE
		PATTERN "uuid" EXCLUDE
		PATTERN "variant" EXCLUDE
		PATTERN "wave" EXCLUDE
		PATTERN "xpressive" EXCLUDE
	  )
	  # License
	  install(FILES "${Boost_INCLUDE_DIR}/LICENSE_1_0.txt" DESTINATION "boost_${Boost_LIB_VERSION}")
	  # libraries
	  if (Boost_THREAD_FOUND)
	    install(FILES ${Boost_THREAD_LIBRARY_DEBUG} DESTINATION "boost_${Boost_LIB_VERSION}/lib" CONFIGURATIONS Debug)
	    install(FILES ${Boost_THREAD_LIBRARY_RELEASE} DESTINATION "boost_${Boost_LIB_VERSION}/lib" CONFIGURATIONS Release)
	  endif()
	  if (Boost_DATE_TIME_FOUND)
	    install(FILES ${Boost_DATE_TIME_LIBRARY_DEBUG} DESTINATION "boost_${Boost_LIB_VERSION}/lib" CONFIGURATIONS Debug)
	    install(FILES ${Boost_DATE_TIME_LIBRARY_RELEASE} DESTINATION "boost_${Boost_LIB_VERSION}/lib" CONFIGURATIONS Release)
	  endif()
		
	endif()
  endif()
  
  
  # install GLES2 dlls
  if (OGRE_BUILD_RENDERSYSTEM_GLES2)
    install_debug(libEGL.dll)
    install_debug(libGLESv2.dll)
	install_release(libEGL.dll)
	install_release(libGLESv2.dll)
  endif ()
  
endif ()

if (OGRE_COPY_DEPENDENCIES)

  if (WIN32)
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
  elseif(APPLE)
    # copy the required libs and frameworks to the build directory (configure_file is the only copy-like op I found in CMake)
    copy_debug(libOIS.a)
    copy_release(libOIS.a)

    if (OGRE_BUILD_PLUGIN_CG)
      copy_debug(Cg.framework)
      copy_release(Cg.framework)
    endif ()
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_GLES2)	
	copy_debug(libEGL.dll)
    copy_debug(libGLESv2.dll)
	copy_release(libEGL.dll)
	copy_release(libGLESv2.dll)
  endif ()

endif ()