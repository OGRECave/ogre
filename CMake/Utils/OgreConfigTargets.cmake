#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure settings and install targets
if(APPLE)
  macro(set_xcode_property targ xc_prop_name xc_prop_val)
    set_property( TARGET ${targ} PROPERTY XCODE_ATTRIBUTE_${xc_prop_name} ${xc_prop_val} )
  endmacro(set_xcode_property)

  if(NOT OGRE_BUILD_PLATFORM_ANDROID AND NOT APPLE_IOS)
    set(PLATFORM_NAME "macosx")
  elseif(APPLE_IOS)
    set(PLATFORM_NAME "$(PLATFORM_NAME)")
  endif()
endif()

# Default build output paths
if (NOT OGRE_ARCHIVE_OUTPUT)
  if(APPLE AND NOT OGRE_BUILD_PLATFORM_ANDROID)
    set(OGRE_ARCHIVE_OUTPUT ${PROJECT_BINARY_DIR}/lib/${PLATFORM_NAME})
  else()
    set(OGRE_ARCHIVE_OUTPUT ${PROJECT_BINARY_DIR}/lib)
  endif()
endif ()
if (NOT OGRE_LIBRARY_OUTPUT)
  if(APPLE AND NOT OGRE_BUILD_PLATFORM_ANDROID)
    set(OGRE_LIBRARY_OUTPUT ${PROJECT_BINARY_DIR}/lib/${PLATFORM_NAME})
  else()
    set(OGRE_LIBRARY_OUTPUT ${PROJECT_BINARY_DIR}/lib)
  endif()
endif ()
if (NOT OGRE_RUNTIME_OUTPUT)
  set(OGRE_RUNTIME_OUTPUT ${PROJECT_BINARY_DIR}/bin)
endif ()

if (WIN32)
  set(OGRE_RELEASE_PATH "")
  set(OGRE_RELWDBG_PATH "")
  set(OGRE_MINSIZE_PATH "")
  set(OGRE_DEBUG_PATH "")
  set(OGRE_LIB_RELEASE_PATH "")
  set(OGRE_LIB_RELWDBG_PATH "")
  set(OGRE_LIB_MINSIZE_PATH "")
  set(OGRE_LIB_DEBUG_PATH "")
  set(OGRE_PLUGIN_PATH "/OGRE")
  set(OGRE_SAMPLE_PATH "/OGRE/Samples")
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
  if(APPLE AND NOT APPLE_IOS)
    set(OGRE_RELEASE_PATH "/${PLATFORM_NAME}")
  endif()
  if(APPLE AND APPLE_IOS)
    set(OGRE_LIB_RELEASE_PATH "/Release")
  endif(APPLE AND APPLE_IOS)
  if (OGRE_BUILD_LIBS_AS_FRAMEWORKS)
    set(OGRE_PLUGIN_PATH "/")
  else()
    set(OGRE_PLUGIN_PATH "/OGRE")
  endif()
  set(OGRE_SAMPLE_PATH "/OGRE/Samples")
endif ()

# create vcproj.user file for Visual Studio to set debug working directory
function(ogre_create_vcproj_userfile TARGETNAME)
  if (MSVC AND NOT WINDOWS_STORE AND NOT WINDOWS_PHONE)
    configure_file(
	  ${OGRE_TEMPLATES_DIR}/VisualStudioUserFile.vcproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.user
	  @ONLY
	)
    configure_file(
	  ${OGRE_TEMPLATES_DIR}/VisualStudioUserFile.vcxproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
	  @ONLY
	)
  endif ()
endfunction(ogre_create_vcproj_userfile)

# install targets according to current build type
function(ogre_install_target TARGETNAME SUFFIX EXPORT)
	if(EXPORT)
	  install(TARGETS ${TARGETNAME} EXPORT OgreTargetsRelease
		CONFIGURATIONS Release None ""
		BUNDLE DESTINATION "bin${OGRE_RELEASE_PATH}"
		RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}"
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELEASE_PATH}${SUFFIX}"
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELEASE_PATH}${SUFFIX}"
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_RELEASE_PATH}/Release")
	  install(TARGETS ${TARGETNAME} EXPORT OgreTargetsRelWithDebInfo
		CONFIGURATIONS RelWithDebInfo
		BUNDLE DESTINATION "bin${OGRE_RELWDBG_PATH}"
		RUNTIME DESTINATION "bin${OGRE_RELWDBG_PATH}"
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}${SUFFIX}"
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}${SUFFIX}"
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_RELWDBG_PATH}/RelWithDebInfo")
	  install(TARGETS ${TARGETNAME} EXPORT OgreTargetsMinSizeRel
		CONFIGURATIONS MinSizeRel
		BUNDLE DESTINATION "bin${OGRE_MINSIZE_PATH}"
		RUNTIME DESTINATION "bin${OGRE_MINSIZE_PATH}"
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_MINSIZE_PATH}${SUFFIX}"
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_MINSIZE_PATH}${SUFFIX}"
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_MINSIZE_PATH}/MinSizeRel")
	  install(TARGETS ${TARGETNAME} EXPORT OgreTargetsDebug
		CONFIGURATIONS Debug
		BUNDLE DESTINATION "bin${OGRE_DEBUG_PATH}"
		RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}"
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}${SUFFIX}"
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}${SUFFIX}"
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_DEBUG_PATH}/Debug")
	else()
	  install(TARGETS ${TARGETNAME}
		CONFIGURATIONS Release None ""
		BUNDLE DESTINATION "bin${OGRE_RELEASE_PATH}" 
		RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}"
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELEASE_PATH}${SUFFIX}"
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELEASE_PATH}${SUFFIX}"
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_RELEASE_PATH}/Release")
	  install(TARGETS ${TARGETNAME}
		CONFIGURATIONS RelWithDebInfo
		BUNDLE DESTINATION "bin${OGRE_RELWDBG_PATH}"
		RUNTIME DESTINATION "bin${OGRE_RELWDBG_PATH}"
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}${SUFFIX}"
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}${SUFFIX}"
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_RELWDBG_PATH}/RelWithDebInfo")
	  install(TARGETS ${TARGETNAME}
		CONFIGURATIONS MinSizeRel
		BUNDLE DESTINATION "bin${OGRE_MINSIZE_PATH}"
		RUNTIME DESTINATION "bin${OGRE_MINSIZE_PATH}"
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_MINSIZE_PATH}${SUFFIX}"
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_MINSIZE_PATH}${SUFFIX}"
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_MINSIZE_PATH}/MinSizeRel")
	  install(TARGETS ${TARGETNAME}
		CONFIGURATIONS Debug
		BUNDLE DESTINATION "bin${OGRE_DEBUG_PATH}"
		RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}"
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}${SUFFIX}"
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}${SUFFIX}"
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_DEBUG_PATH}/Debug")
	endif()

endfunction(ogre_install_target)

# setup common target settings
function(ogre_config_common TARGETNAME)
  set_target_properties(${TARGETNAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OGRE_ARCHIVE_OUTPUT}
    LIBRARY_OUTPUT_DIRECTORY ${OGRE_LIBRARY_OUTPUT}
    RUNTIME_OUTPUT_DIRECTORY ${OGRE_RUNTIME_OUTPUT}
  )
  if(APPLE_IOS)
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf$<$<NOT:$<CONFIG:Debug>>:-with-dsym>")
  endif(APPLE_IOS)

  if(NOT OGRE_STATIC AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")
  endif()

  ogre_create_vcproj_userfile(${TARGETNAME})
endfunction(ogre_config_common)

# checks whether the target LIBNAME produces a pdb file
function(ogre_produces_pdb VARNAME LIBNAME)
  get_target_property(TYPE ${LIBNAME} TYPE)
  if (TYPE STREQUAL "SHARED_LIBRARY" OR TYPE STREQUAL "MODULE_LIBRARY" OR TYPE STREQUAL "EXECUTABLE")
    set(${VARNAME} ON PARENT_SCOPE)
  else ()
    set(${VARNAME} OFF PARENT_SCOPE)
  endif ()
endfunction(ogre_produces_pdb)

# setup library build
function(ogre_config_lib LIBNAME EXPORT)
  ogre_config_common(${LIBNAME})
  if (OGRE_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME ${LIBNAME}Static)
  else (OGRE_STATIC)
	if (MINGW)
	  # remove lib prefix from DLL outputs
	  set_target_properties(${LIBNAME} PROPERTIES PREFIX "")
	endif ()
  endif (OGRE_STATIC)
  ogre_install_target(${LIBNAME} "" ${EXPORT})
  
  if (OGRE_INSTALL_PDB)
    # install debug pdb files
    if (OGRE_STATIC)
	  install(FILES ${PROJECT_BINARY_DIR}/lib${OGRE_LIB_DEBUG_PATH}/${LIBNAME}Static_d.pdb
	    DESTINATION ${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES ${PROJECT_BINARY_DIR}/lib${OGRE_LIB_RELWDBG_PATH}/${LIBNAME}Static.pdb
	    DESTINATION ${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}
		CONFIGURATIONS RelWithDebInfo
	  )
	else ()
    ogre_produces_pdb(PRODUCES_PDB ${LIBNAME})
    if (PRODUCES_PDB)
	  install(FILES $<TARGET_PDB_FILE:${LIBNAME}>
	    DESTINATION bin${OGRE_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES $<TARGET_PDB_FILE:${LIBNAME}>
	    DESTINATION bin${OGRE_RELWDBG_PATH}
		CONFIGURATIONS RelWithDebInfo
	  )
    endif ()
	endif ()
  endif ()
endfunction(ogre_config_lib)

function(ogre_config_component LIBNAME)
  ogre_config_lib(${LIBNAME} TRUE)
  if (OGRE_PROJECT_FOLDERS)
    set_property(TARGET ${LIBNAME} PROPERTY FOLDER Components)
  endif ()
endfunction(ogre_config_component)

function(ogre_config_framework LIBNAME)
  if (OGRE_BUILD_LIBS_AS_FRAMEWORKS)
      set_target_properties(${LIBNAME} PROPERTIES FRAMEWORK TRUE)

      # Set the INSTALL_PATH so that frameworks can be installed in the application package
      set_target_properties(${LIBNAME}
         PROPERTIES BUILD_WITH_INSTALL_RPATH 1
         INSTALL_NAME_DIR "@rpath"
      )
      set_target_properties(${LIBNAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES};${PLATFORM_HEADERS};" )
      set_target_properties(${LIBNAME} PROPERTIES RESOURCE "${RESOURCE_FILES}")
      set_source_files_properties("${RESOURCE_FILES}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

      set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME ${LIBNAME})
  endif()
endfunction(ogre_config_framework)

# setup plugin build
function(ogre_config_plugin PLUGINNAME)
  ogre_config_common(${PLUGINNAME})

  if (OGRE_PROJECT_FOLDERS)
    set_property(TARGET ${LIBNAME} PROPERTY FOLDER Plugins)
  endif ()

  set_target_properties(${PLUGINNAME} PROPERTIES VERSION ${OGRE_SOVERSION})
  if (OGRE_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${PLUGINNAME} PROPERTIES OUTPUT_NAME ${PLUGINNAME}Static)

    if(APPLE_IOS)
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
    endif(APPLE_IOS)
  else (OGRE_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      # disable "lib" prefix on Unix
      set_target_properties(${PLUGINNAME} PROPERTIES PREFIX "")
    endif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  endif (OGRE_STATIC)
  # export only if static
  ogre_install_target(${PLUGINNAME} ${OGRE_PLUGIN_PATH} ${OGRE_STATIC})

  if (OGRE_INSTALL_PDB)
    # install debug pdb files
    if (OGRE_STATIC)
	  install(FILES ${PROJECT_BINARY_DIR}/lib${OGRE_LIB_DEBUG_PATH}/${PLUGINNAME}Static_d.pdb
	    DESTINATION ${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}/opt
		CONFIGURATIONS Debug
	  )
	  install(FILES ${PROJECT_BINARY_DIR}/lib${OGRE_LIB_RELWDBG_PATH}/${PLUGINNAME}Static.pdb
	    DESTINATION ${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}/opt
		CONFIGURATIONS RelWithDebInfo
	  )
	else ()
    ogre_produces_pdb(PRODUCES_PDB ${PLUGINNAME})
    if(PRODUCES_PDB)
	  install(FILES $<TARGET_PDB_FILE:${PLUGINNAME}>
	    DESTINATION bin${OGRE_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES $<TARGET_PDB_FILE:${PLUGINNAME}>
	    DESTINATION bin${OGRE_RELWDBG_PATH}
		CONFIGURATIONS RelWithDebInfo
	  )
    endif ()
	endif ()
  endif ()
endfunction(ogre_config_plugin)

# setup Ogre sample build
function(ogre_config_sample_common SAMPLENAME)
  ogre_config_common(${SAMPLENAME})

  if (OGRE_PROJECT_FOLDERS)
    set_property(TARGET ${LIBNAME} PROPERTY FOLDER Samples)
  endif ()
  
  if (APPLE)
    # On OS X, create .app bundle
    set_property(TARGET ${SAMPLENAME} PROPERTY MACOSX_BUNDLE TRUE)
    if (NOT APPLE_IOS)
      # Add the path where the Ogre framework was found
      if(${OGRE_FRAMEWORK_PATH})
        set_target_properties(${SAMPLENAME} PROPERTIES
          COMPILE_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
          LINK_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
        )
      endif()
    endif()
  endif (APPLE)
  if (NOT OGRE_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      # disable "lib" prefix on Unix
      set_target_properties(${SAMPLENAME} PROPERTIES PREFIX "")
    endif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  endif()

  if (NOT WIN32)
    set_target_properties(${SAMPLENAME} PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
  endif()

  if (OGRE_INSTALL_SAMPLES AND NOT OGRE_STATIC)
	ogre_install_target(${SAMPLENAME} ${OGRE_SAMPLE_PATH} FALSE)
  endif()
  
endfunction(ogre_config_sample_common)

function(ogre_config_sample_exe SAMPLENAME)
  ogre_config_sample_common(${SAMPLENAME})
  if (OGRE_INSTALL_PDB AND OGRE_INSTALL_SAMPLES)
	  # install debug pdb files - no _d on exe
	  install(FILES $<TARGET_PDB_FILE:${SAMPLENAME}>
		  DESTINATION bin${OGRE_DEBUG_PATH}
		  CONFIGURATIONS Debug
		  )
	  install(FILES $<TARGET_PDB_FILE:${SAMPLENAME}>
		  DESTINATION bin${OGRE_RELWDBG_PATH}
		  CONFIGURATIONS RelWithDebInfo
		  )
  endif ()
endfunction(ogre_config_sample_exe)

function(ogre_config_sample_lib SAMPLENAME)
  ogre_config_sample_common(${SAMPLENAME})
  if (OGRE_INSTALL_PDB AND OGRE_INSTALL_SAMPLES)
	  # install debug pdb files - with a _d on lib
    ogre_produces_pdb(PRODUCES_PDB ${SAMPLENAME})
    if (PRODUCES_PDB)
	  install(FILES $<TARGET_PDB_FILE:${SAMPLENAME}>
		  DESTINATION bin${OGRE_DEBUG_PATH}
		  CONFIGURATIONS Debug
		  )
	  install(FILES $<TARGET_PDB_FILE:${SAMPLENAME}>
		  DESTINATION bin${OGRE_RELWDBG_PATH}
		  CONFIGURATIONS RelWithDebInfo
		  )
    endif ()
  endif ()

  if(NOT OGRE_STATIC AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    # add GCC visibility flags to shared library build
    set_target_properties(${SAMPLENAME} PROPERTIES COMPILE_FLAGS "${OGRE_VISIBILITY_FLAGS}")
  endif()

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
  if (WIN32)
	set_property(TARGET ${TOOLNAME} APPEND PROPERTY DEBUG_POSTFIX "_d")
  endif ()

  if (OGRE_INSTALL_TOOLS)
    ogre_install_target(${TOOLNAME} "" FALSE)
    if (OGRE_INSTALL_PDB)
      # install debug pdb files
      install(FILES $<TARGET_PDB_FILE:${TOOLNAME}>
        DESTINATION bin${OGRE_DEBUG_PATH}
        CONFIGURATIONS Debug
        )
      install(FILES $<TARGET_PDB_FILE:${TOOLNAME}>
        DESTINATION bin${OGRE_RELWDBG_PATH}
        CONFIGURATIONS RelWithDebInfo
        )
    endif ()
  endif ()	

endfunction(ogre_config_tool)
