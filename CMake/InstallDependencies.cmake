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
if (NOT APPLE AND NOT WIN32)
  return()
endif()

# TODO - most of this file assumes a common dependencies root folder
# This is not robust, we should instead source dependencies from their individual locations
get_filename_component(OGRE_DEP_DIR ${OIS_INCLUDE_DIR}/../../ ABSOLUTE)

option(OGRE_INSTALL_DEPENDENCIES "Install dependency libs needed for samples" TRUE)
option(OGRE_COPY_DEPENDENCIES "Copy dependency libs to the build directory" TRUE)

macro(install_debug INPUT)
  if (EXISTS ${OGRE_DEP_DIR}/bin/debug/${INPUT})
    if (IS_DIRECTORY ${OGRE_DEP_DIR}/bin/debug/${INPUT})
      install(DIRECTORY ${OGRE_DEP_DIR}/bin/debug/${INPUT} DESTINATION bin/debug CONFIGURATIONS Debug)
    else ()
      install(FILES ${OGRE_DEP_DIR}/bin/debug/${INPUT} DESTINATION bin/debug CONFIGURATIONS Debug)
    endif ()
  else()
    message(send_error "${OGRE_DEP_DIR}/bin/debug/${INPUT} did not exist, can't install!")
  endif ()
endmacro()

macro(install_release INPUT)
  if (EXISTS ${OGRE_DEP_DIR}/bin/release/${INPUT})
    if (IS_DIRECTORY ${OGRE_DEP_DIR}/bin/release/${INPUT})
      install(DIRECTORY ${OGRE_DEP_DIR}/bin/release/${INPUT} DESTINATION bin/release CONFIGURATIONS Release None "")
      install(DIRECTORY ${OGRE_DEP_DIR}/bin/release/${INPUT} DESTINATION bin/relwithdebinfo CONFIGURATIONS RelWithDebInfo)
      install(DIRECTORY ${OGRE_DEP_DIR}/bin/release/${INPUT} DESTINATION bin/minsizerel CONFIGURATIONS MinSizeRel)
    else ()
      install(FILES ${OGRE_DEP_DIR}/bin/release/${INPUT} DESTINATION bin/release CONFIGURATIONS Release None "")
      install(FILES ${OGRE_DEP_DIR}/bin/release/${INPUT} DESTINATION bin/relwithdebinfo CONFIGURATIONS RelWithDebInfo)
      install(FILES ${OGRE_DEP_DIR}/bin/release/${INPUT} DESTINATION bin/minsizerel CONFIGURATIONS MinSizeRel)
    endif ()
  else()
    message(send_error "${OGRE_DEP_DIR}/bin/release/${INPUT} did not exist, can't install!")
  endif ()
endmacro()

macro(copy_debug INPUT)
  if (EXISTS ${OGRE_DEP_DIR}/lib/debug/${INPUT})
    if (MINGW OR NMAKE)
      configure_file(${OGRE_DEP_DIR}/lib/debug/${INPUT} ${OGRE_BINARY_DIR}/lib/${INPUT} COPYONLY)
	else ()
      if (IS_DIRECTORY ${OGRE_DEP_DIR}/lib/debug/${INPUT})
        install(DIRECTORY ${OGRE_DEP_DIR}/lib/debug/${INPUT} DESTINATION lib/debug)
      else ()
        configure_file(${OGRE_DEP_DIR}/lib/debug/${INPUT} ${OGRE_BINARY_DIR}/lib/debug/${INPUT} COPYONLY)
      endif ()
	endif ()
  endif ()
endmacro()

macro(copy_release INPUT)
  if (EXISTS ${OGRE_DEP_DIR}/lib/release/${INPUT})
    if (MINGW OR NMAKE)
      configure_file(${OGRE_DEP_DIR}/lib/release/${INPUT} ${OGRE_BINARY_DIR}/lib/${INPUT} COPYONLY)
	else ()
      if (IS_DIRECTORY ${OGRE_DEP_DIR}/lib/release/${INPUT})
        install(DIRECTORY ${OGRE_DEP_DIR}/lib/release/${INPUT} DESTINATION lib/release CONFIGURATIONS Release None "")
        install(DIRECTORY ${OGRE_DEP_DIR}/lib/release/${INPUT} DESTINATION lib/relwithdebinfo CONFIGURATIONS RelWithDebInfo)
        install(DIRECTORY ${OGRE_DEP_DIR}/lib/release/${INPUT} DESTINATION lib/minsizerel CONFIGURATIONS MinSizeRel)
      else ()
        configure_file(${OGRE_DEP_DIR}/lib/release/${INPUT} ${OGRE_BINARY_DIR}/lib/release/${INPUT} COPYONLY)
        configure_file(${OGRE_DEP_DIR}/lib/release/${INPUT} ${OGRE_BINARY_DIR}/lib/relwithdebinfo/${INPUT} COPYONLY)
        configure_file(${OGRE_DEP_DIR}/lib/release/${INPUT} ${OGRE_BINARY_DIR}/lib/minsizerel/${INPUT} COPYONLY)
      endif ()
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
	endif () # OGRE_STATIC
  
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
		if (EXISTS ${OIS_LIBRARY_DBG})
			install(FILES ${OIS_LIBRARY_DBG} DESTINATION lib/debug CONFIGURATIONS Debug)
		else()	
			install(FILES DESTINATION lib/debug CONFIGURATIONS Debug)
		endif ()
		if (EXISTS ${OIS_LIBRARY_REL})
			install(FILES ${OIS_LIBRARY_REL} DESTINATION lib/relwithdebinfo CONFIGURATIONS RelWithDebInfo)
			install(FILES ${OIS_LIBRARY_REL} DESTINATION lib/release CONFIGURATIONS Release)
			install(FILES ${OIS_LIBRARY_REL} DESTINATION lib/minsizerel CONFIGURATIONS MinSizeRel)
		else()
			install(FILES DESTINATION lib/relwithdebinfo CONFIGURATIONS RelWithDebInfo)
			install(FILES DESTINATION lib/release CONFIGURATIONS Release)
			install(FILES DESTINATION lib/minsizerel CONFIGURATIONS MinSizeRel)
		endif ()
	  endif () # MINGW
	endif () # WIN32
endif () # OGRE_INSTALL_DEPENDENCIES
    
  if(WIN32)
    # copy the dependency DLLs to the right places
    if(NOT OGRE_BUILD_PLATFORM_WINRT)
      install_debug(OIS_d.dll)
      install_release(OIS.dll)
    endif ()

    if (OGRE_BUILD_PLUGIN_CG)
      # if MinGW or NMake, the release/debug cg.dll's would conflict, so just pick one
      if (MINGW OR (CMAKE_GENERATOR STREQUAL "NMake Makefiles"))
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
          install_debug(cg.dll)
        else ()
          install_release(cg.dll)
        endif ()
      else ()
        install_debug(cg.dll)
        install_release(cg.dll)
      endif ()
    endif () # OGRE_BUILD_PLUGIN_CG

    # install GLES dlls
    if (OGRE_BUILD_RENDERSYSTEM_GLES)
      install_debug(libgles_cm.dll)
      install_release(libgles_cm.dll)
    endif ()

    # install GLES2 dlls
    if (OGRE_BUILD_RENDERSYSTEM_GLES2)
      install_debug(libGLESv2.dll)
      install_release(libEGL.dll)
    endif ()
  endif () # WIN32
  
  # If we're installing the sample source for an SDK, also install Boost headers & libraries
  if (OGRE_INSTALL_SAMPLES_SOURCE AND Boost_FOUND)
    # headers (try to exclude things we don't need)
    install(DIRECTORY "${Boost_INCLUDE_DIR}/boost" DESTINATION "boost"
      PATTERN "accumulators" EXCLUDE
      PATTERN "archive" EXCLUDE
      PATTERN "asio" EXCLUDE
      PATTERN "assign" EXCLUDE
      PATTERN "bimap" EXCLUDE
      PATTERN "circular_buffer" EXCLUDE
      PATTERN "compatibility" EXCLUDE
      PATTERN "concept_check" EXCLUDE
      PATTERN "container" EXCLUDE
      PATTERN "dynamic_bitset" EXCLUDE
      PATTERN "filesystem" EXCLUDE
      PATTERN "flyweight" EXCLUDE
      PATTERN "format" EXCLUDE
      PATTERN "fusion" EXCLUDE
      PATTERN "geometry" EXCLUDE
      PATTERN "gil" EXCLUDE
      PATTERN "graph" EXCLUDE
      PATTERN "interprocess" EXCLUDE
      PATTERN "intrusive" EXCLUDE
      PATTERN "iostreams" EXCLUDE
      PATTERN "lambda" EXCLUDE
      PATTERN "logic" EXCLUDE
      PATTERN "mpi" EXCLUDE
      PATTERN "multi_array" EXCLUDE
      PATTERN "multi_index" EXCLUDE
      PATTERN "numeric" EXCLUDE
      PATTERN "parameter" EXCLUDE
      PATTERN "pending" EXCLUDE
      PATTERN "phoenix" EXCLUDE
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
      PATTERN "test" EXCLUDE
      PATTERN "timer" EXCLUDE
      PATTERN "tr1" EXCLUDE
      PATTERN "units" EXCLUDE
      PATTERN "uuid" EXCLUDE
      PATTERN "variant" EXCLUDE
      PATTERN "wave" EXCLUDE
      PATTERN "xpressive" EXCLUDE
    )
    # License
    if (EXISTS "${Boost_INCLUDE_DIR}/boost/LICENSE_1_0.txt")
        install(FILES "${Boost_INCLUDE_DIR}/boost/LICENSE_1_0.txt" DESTINATION "boost")
    elseif (EXISTS "${Boost_INCLUDE_DIR}/LICENSE_1_0.txt")
        install(FILES "${Boost_INCLUDE_DIR}/LICENSE_1_0.txt" DESTINATION "boost")
    endif ()
    # libraries
    if (Boost_THREAD_FOUND)
      install(FILES ${Boost_THREAD_LIBRARY_DEBUG} DESTINATION "boost/lib" CONFIGURATIONS Debug)
      install(FILES ${Boost_THREAD_LIBRARY_RELEASE} DESTINATION "boost/lib" CONFIGURATIONS Release)
    endif()
    if (Boost_DATE_TIME_FOUND)
      install(FILES ${Boost_DATE_TIME_LIBRARY_DEBUG} DESTINATION "boost/lib" CONFIGURATIONS Debug)
      install(FILES ${Boost_DATE_TIME_LIBRARY_RELEASE} DESTINATION "boost/lib" CONFIGURATIONS Release)
    endif()
    if (Boost_SYSTEM_FOUND)
      install(FILES ${Boost_SYSTEM_LIBRARY_DEBUG} DESTINATION "boost/lib" CONFIGURATIONS Debug)
      install(FILES ${Boost_SYSTEM_LIBRARY_RELEASE} DESTINATION "boost/lib" CONFIGURATIONS Release)
    endif()
    if (Boost_CHRONO_FOUND)
      install(FILES ${Boost_CHRONO_LIBRARY_DEBUG} DESTINATION "boost/lib" CONFIGURATIONS Debug)
      install(FILES ${Boost_CHRONO_LIBRARY_RELEASE} DESTINATION "boost/lib" CONFIGURATIONS Release)
    endif()
  endif()
endif ()

if (OGRE_COPY_DEPENDENCIES)

  if (WIN32)
    # copy the required DLLs to the build directory (configure_file is the only copy-like op I found in CMake)
	if( OGRE_BUILD_SAMPLES OR OGRE_BUILD_TESTS )
		if(NOT OGRE_BUILD_PLATFORM_WINRT)
		  file(COPY ${OIS_BINARY_DBG} DESTINATION ${OGRE_BINARY_DIR}/bin/debug)
			file(COPY ${OIS_BINARY_REL} DESTINATION ${OGRE_BINARY_DIR}/bin/release)
			file(COPY ${OIS_BINARY_REL} DESTINATION ${OGRE_BINARY_DIR}/bin/relwithdebinfo)
			file(COPY ${OIS_BINARY_REL} DESTINATION ${OGRE_BINARY_DIR}/bin/minsizerel)
		endif ()
	endif()

    if (OGRE_BUILD_PLUGIN_CG)
      # if MinGW or NMake, the release/debug cg.dll's would conflict, so just pick one
      if (MINGW OR (CMAKE_GENERATOR STREQUAL "NMake Makefiles"))
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
          file(COPY ${Cg_BINARY_DBG} DESTINATION ${OGRE_BINARY_DIR}/bin/debug)
        else ()
          file(COPY ${Cg_BINARY_REL} DESTINATION ${OGRE_BINARY_DIR}/bin/release)
    			file(COPY ${Cg_BINARY_REL} DESTINATION ${OGRE_BINARY_DIR}/bin/relwithdebinfo)
    			file(COPY ${Cg_BINARY_REL} DESTINATION ${OGRE_BINARY_DIR}/bin/minsizerel)
        endif ()
      else ()
        file(COPY ${Cg_BINARY_DBG} DESTINATION ${OGRE_BINARY_DIR}/bin/debug)
    		file(COPY ${Cg_BINARY_REL} DESTINATION ${OGRE_BINARY_DIR}/bin/release)
    		file(COPY ${Cg_BINARY_REL} DESTINATION ${OGRE_BINARY_DIR}/bin/relwithdebinfo)
    		file(COPY ${Cg_BINARY_REL} DESTINATION ${OGRE_BINARY_DIR}/bin/minsizerel)
      endif ()
    endif()
   
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

endif ()
