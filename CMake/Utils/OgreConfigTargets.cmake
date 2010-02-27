#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure settings and install targets

if (WIN32)
  set(OGRE_RELEASE_PATH "/Release")
  set(OGRE_RELWDBG_PATH "/RelWithDebInfo")
  set(OGRE_MINSIZE_PATH "/MinSizeRel")
  set(OGRE_DEBUG_PATH "/Debug")
  set(OGRE_LIB_RELEASE_PATH "/Release")
  set(OGRE_LIB_RELWDBG_PATH "/RelWithDebInfo")
  set(OGRE_LIB_MINSIZE_PATH "/MinSizeRel")
  set(OGRE_LIB_DEBUG_PATH "/Debug")
  set(OGRE_PLUGIN_PATH "/opt")
  set(OGRE_SAMPLE_PATH "/opt/samples")
elseif (UNIX)
  set(OGRE_RELEASE_PATH "")
  set(OGRE_RELWDBG_PATH "")
  set(OGRE_MINSIZE_PATH "")
  set(OGRE_DEBUG_PATH "/debug")
  if (NOT APPLE)
	set(OGRE_DEBUG_PATH "")
  endif ()
  set(OGRE_LIB_RELEASE_PATH "")
  set(OGRE_LIB_RELWDBG_PATH "")
  set(OGRE_LIB_MINSIZE_PATH "")
  set(OGRE_LIB_DEBUG_PATH "")
  if (APPLE)
    set(OGRE_PLUGIN_PATH "/")
  else()
    set(OGRE_PLUGIN_PATH "/OGRE")
  endif(APPLE)
  set(OGRE_SAMPLE_PATH "/OGRE/Samples")
endif ()

# create vcproj.user file for Visual Studio to set debug working directory
function(ogre_create_vcproj_userfile TARGETNAME)
  if (MSVC)
    configure_file(
	  ${OGRE_TEMPLATES_DIR}/VisualStudioUserFile.vcproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.user
	  @ONLY
	)
  endif ()
endfunction(ogre_create_vcproj_userfile)

# install targets according to current build type
function(ogre_install_target TARGETNAME SUFFIX EXPORT)
	# Skip all install targets in SDK
	if (OGRE_SDK_BUILD)
		return()
	endif()
	
	if(EXPORT)
	  install(TARGETS ${TARGETNAME} #EXPORT Ogre-exports
		BUNDLE DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release None ""
		RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release None ""
		LIBRARY DESTINATION "lib${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
		ARCHIVE DESTINATION "lib${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
		FRAMEWORK DESTINATION "lib${OGRE_RELEASE_PATH}/Release" CONFIGURATIONS Release None ""
      )
	  install(TARGETS ${TARGETNAME} #EXPORT Ogre-exports
		BUNDLE DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
		RUNTIME DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
		LIBRARY DESTINATION "lib${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
		ARCHIVE DESTINATION "lib${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
		FRAMEWORK DESTINATION "lib${OGRE_RELWDBG_PATH}/RelWithDebInfo" CONFIGURATIONS RelWithDebInfo
      )
	  install(TARGETS ${TARGETNAME} #EXPORT Ogre-exports
		BUNDLE DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
		RUNTIME DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
		LIBRARY DESTINATION "lib${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
		ARCHIVE DESTINATION "lib${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
		FRAMEWORK DESTINATION "lib${OGRE_MINSIZE_PATH}/MinSizeRel" CONFIGURATIONS MinSizeRel
      )
	  install(TARGETS ${TARGETNAME} #EXPORT Ogre-exports
		BUNDLE DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
		RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
		LIBRARY DESTINATION "lib${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
		ARCHIVE DESTINATION "lib${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
		FRAMEWORK DESTINATION "lib${OGRE_DEBUG_PATH}/Debug" CONFIGURATIONS Debug
	  )
	  #install(EXPORT Ogre-exports DESTINATION lib)
	else()
	  install(TARGETS ${TARGETNAME}
		BUNDLE DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release None ""
		RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release None ""
		LIBRARY DESTINATION "lib${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
		ARCHIVE DESTINATION "lib${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
		FRAMEWORK DESTINATION "lib${OGRE_RELEASE_PATH}/Release" CONFIGURATIONS Release None ""
      )
	  install(TARGETS ${TARGETNAME}
		BUNDLE DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
		RUNTIME DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
		LIBRARY DESTINATION "lib${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
		ARCHIVE DESTINATION "lib${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
		FRAMEWORK DESTINATION "lib${OGRE_RELWDBG_PATH}/RelWithDebInfo" CONFIGURATIONS RelWithDebInfo
      )
	  install(TARGETS ${TARGETNAME}
		BUNDLE DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
		RUNTIME DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
		LIBRARY DESTINATION "lib${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
		ARCHIVE DESTINATION "lib${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
		FRAMEWORK DESTINATION "lib${OGRE_MINSIZE_PATH}/MinSizeRel" CONFIGURATIONS MinSizeRel
      )
	  install(TARGETS ${TARGETNAME}
		BUNDLE DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
		RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
		LIBRARY DESTINATION "lib${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
		ARCHIVE DESTINATION "lib${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
		FRAMEWORK DESTINATION "lib${OGRE_DEBUG_PATH}/Debug" CONFIGURATIONS Debug
	  )
	endif()

endfunction(ogre_install_target)

# setup common target settings
function(ogre_config_common TARGETNAME)
  set_target_properties(${TARGETNAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/bin
  )
  if(OGRE_BUILD_PLATFORM_IPHONE)
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${OGRE_SOURCE_DIR}/OgreMain/include/OgreStableHeaders.h")
  endif(OGRE_BUILD_PLATFORM_IPHONE)

  ogre_create_vcproj_userfile(${TARGETNAME})
endfunction(ogre_config_common)

# setup library build
function(ogre_config_lib LIBNAME EXPORT)
  ogre_config_common(${LIBNAME})
  if (OGRE_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME ${LIBNAME}Static)
  else (OGRE_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX)
      # add GCC visibility flags to shared library build
      set_target_properties(${LIBNAME} PROPERTIES COMPILE_FLAGS "${OGRE_GCC_VISIBILITY_FLAGS}")
	endif (CMAKE_COMPILER_IS_GNUCXX)
	if (MINGW)
	  # remove lib prefix from DLL outputs
	  set_target_properties(${LIBNAME} PROPERTIES PREFIX "")
	endif ()
  endif (OGRE_STATIC)
  ogre_install_target(${LIBNAME} "" ${EXPORT})
  
  if (OGRE_INSTALL_PDB)
    # install debug pdb files
    if (OGRE_STATIC)
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_LIB_DEBUG_PATH}/${LIBNAME}Static_d.pdb
	    DESTINATION lib${OGRE_LIB_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_LIB_RELWDBG_PATH}/${LIBNAME}Static.pdb
	    DESTINATION lib${OGRE_LIB_RELWDBG_PATH}
		CONFIGURATIONS RelWithDebInfo
	  )
	else ()
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${LIBNAME}_d.pdb
	    DESTINATION bin${OGRE_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_RELWDBG_PATH}/${LIBNAME}.pdb
	    DESTINATION bin${OGRE_RELWDBG_PATH}
		CONFIGURATIONS RelWithDebInfo
	  )
	endif ()
  endif ()
endfunction(ogre_config_lib)

function(ogre_config_component LIBNAME)
  ogre_config_lib(${LIBNAME} FALSE)
endfunction(ogre_config_component)


# setup plugin build
function(ogre_config_plugin PLUGINNAME)
  ogre_config_common(${PLUGINNAME})
  if (OGRE_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${PLUGINNAME} PROPERTIES OUTPUT_NAME ${PLUGINNAME}Static)

    if(OGRE_BUILD_PLATFORM_IPHONE)
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${OGRE_SOURCE_DIR}/OgreMain/include/OgreStableHeaders.h")
    endif(OGRE_BUILD_PLATFORM_IPHONE)
  else (OGRE_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX)
      # add GCC visibility flags to shared library build
      set_target_properties(${PLUGINNAME} PROPERTIES COMPILE_FLAGS "${OGRE_GCC_VISIBILITY_FLAGS}")
      # disable "lib" prefix on Unix
      set_target_properties(${PLUGINNAME} PROPERTIES PREFIX "")
	endif (CMAKE_COMPILER_IS_GNUCXX)	
  endif (OGRE_STATIC)
  # export only if static
  ogre_install_target(${PLUGINNAME} ${OGRE_PLUGIN_PATH} ${OGRE_STATIC})

  if (OGRE_INSTALL_PDB)
    # install debug pdb files
    if (OGRE_STATIC)
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_LIB_DEBUG_PATH}/${PLUGINNAME}Static_d.pdb
	    DESTINATION lib${OGRE_LIB_DEBUG_PATH}/opt
		CONFIGURATIONS Debug
	  )
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_LIB_RELWDBG_PATH}/${PLUGINNAME}Static.pdb
	    DESTINATION lib${OGRE_LIB_RELWDBG_PATH}/opt
		CONFIGURATIONS RelWithDebInfo
	  )
	else ()
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${PLUGINNAME}_d.pdb
	    DESTINATION bin${OGRE_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_RELWDBG_PATH}/${PLUGINNAME}.pdb
	    DESTINATION bin${OGRE_RELWDBG_PATH}
		CONFIGURATIONS RelWithDebInfo
	  )
	endif ()
  endif ()
endfunction(ogre_config_plugin)

# setup Ogre sample build
function(ogre_config_sample_common SAMPLENAME)
  # The PRODUCT_NAME target setting cannot contain underscores.  Just remove them
  # Known bug in Xcode CFBundleIdentifier processing rdar://6187020
  # Can cause an instant App Store rejection. Also, code signing will fail. 
  #if (OGRE_BUILD_PLATFORM_IPHONE)
#    string (REPLACE "_" "" SAMPLENAME ${SAMPLENAME})
  #endif()
  ogre_config_common(${SAMPLENAME})

  # set install RPATH for Unix systems
  if (UNIX AND OGRE_FULL_RPATH)
    set_property(TARGET ${SAMPLENAME} APPEND PROPERTY
      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set_property(TARGET ${SAMPLENAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif ()
  
  if (APPLE)
    # On OS X, create .app bundle
    set_property(TARGET ${SAMPLENAME} PROPERTY MACOSX_BUNDLE TRUE)
    if (NOT OGRE_BUILD_PLATFORM_IPHONE)
      # Add the path where the Ogre framework was found
      if(${OGRE_FRAMEWORK_PATH})
        set_target_properties(${SAMPLENAME} PROPERTIES
          COMPILE_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
          LINK_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
        )
      endif()
    endif(NOT OGRE_BUILD_PLATFORM_IPHONE)
  endif (APPLE)
  if (CMAKE_COMPILER_IS_GNUCXX)
    # add GCC visibility flags to shared library build
    set_target_properties(${SAMPLENAME} PROPERTIES COMPILE_FLAGS "${OGRE_GCC_VISIBILITY_FLAGS}")
    # disable "lib" prefix on Unix
    set_target_properties(${SAMPLENAME} PROPERTIES PREFIX "")
  endif (CMAKE_COMPILER_IS_GNUCXX)	
  if (OGRE_INSTALL_SAMPLES)
	ogre_install_target(${SAMPLENAME} ${OGRE_SAMPLE_PATH} FALSE)
  endif()
  
endfunction(ogre_config_sample_common)

function(ogre_config_sample_exe SAMPLENAME)
  ogre_config_sample_common(${SAMPLENAME})
  if (OGRE_INSTALL_PDB AND OGRE_INSTALL_SAMPLES)
	  # install debug pdb files - no _d on exe
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${SAMPLENAME}.pdb
		  DESTINATION bin${OGRE_DEBUG_PATH}
		  CONFIGURATIONS Debug
		  )
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_RELWDBG_PATH}/${SAMPLENAME}.pdb
		  DESTINATION bin${OGRE_RELWDBG_PATH}
		  CONFIGURATIONS RelWithDebInfo
		  )
  endif ()

  if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE AND OGRE_SDK_BUILD)
    # Add the path where the Ogre framework was found
    if(NOT ${OGRE_FRAMEWORK_PATH} STREQUAL "")
      set_target_properties(${SAMPLENAME} PROPERTIES
        COMPILE_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
        LINK_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
      )
    endif()
  endif(APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE AND OGRE_SDK_BUILD)
endfunction(ogre_config_sample_exe)

function(ogre_config_sample_lib SAMPLENAME)
  ogre_config_sample_common(${SAMPLENAME})
  if (OGRE_INSTALL_PDB AND OGRE_INSTALL_SAMPLES)
	  # install debug pdb files - with a _d on lib
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${SAMPLENAME}_d.pdb
		  DESTINATION bin${OGRE_DEBUG_PATH}
		  CONFIGURATIONS Debug
		  )
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_RELWDBG_PATH}/${SAMPLENAME}.pdb
		  DESTINATION bin${OGRE_RELWDBG_PATH}
		  CONFIGURATIONS RelWithDebInfo
		  )
  endif ()

  if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE AND OGRE_SDK_BUILD)
    # Add the path where the Ogre framework was found
    if(NOT ${OGRE_FRAMEWORK_PATH} STREQUAL "")
      set_target_properties(${SAMPLENAME} PROPERTIES
        COMPILE_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
        LINK_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
      )
    endif()
  endif(APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE AND OGRE_SDK_BUILD)

  # Add sample to the list of link targets
  # Global property so that we can build this up across entire sample tree
  # since vars are local to containing scope of directories / functions
  get_property(OGRE_SAMPLES_LIST GLOBAL PROPERTY "OGRE_SAMPLES_LIST")
  set_property (GLOBAL PROPERTY "OGRE_SAMPLES_LIST" ${OGRE_SAMPLES_LIST} ${SAMPLENAME})

endfunction(ogre_config_sample_lib)


# setup Ogre tool build
function(ogre_config_tool TOOLNAME)
  ogre_config_common(${TOOLNAME})

  #set _d debug postfix
  if (NOT APPLE)
	set_property(TARGET ${TOOLNAME} APPEND PROPERTY DEBUG_POSTFIX "_d")
  endif ()

  # set install RPATH for Unix systems
  if (UNIX AND OGRE_FULL_RPATH)
    set_property(TARGET ${TOOLNAME} APPEND PROPERTY
      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set_property(TARGET ${TOOLNAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif ()

  if (OGRE_INSTALL_TOOLS)
    ogre_install_target(${TOOLNAME} "" FALSE)
    if (OGRE_INSTALL_PDB)
      # install debug pdb files
      install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${TOOLNAME}.pdb
        DESTINATION bin${OGRE_DEBUG_PATH}
        CONFIGURATIONS Debug
        )
      install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_RELWDBG_PATH}/${TOOLNAME}.pdb
        DESTINATION bin${OGRE_RELWDBG_PATH}
        CONFIGURATIONS RelWithDebInfo
        )
    endif ()
  endif ()	

endfunction(ogre_config_tool)

# Get component include dir (different when referencing SDK)
function(ogre_add_component_include_dir COMPONENTNAME)
	if (OGRE_SDK_BUILD)
		include_directories("${OGRE_INCLUDE_DIR}/${COMPONENTNAME}")
	else()
		include_directories("${OGRE_SOURCE_DIR}/Components/${COMPONENTNAME}/include")	
	endif()
endfunction(ogre_add_component_include_dir)
