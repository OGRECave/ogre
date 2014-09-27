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

  set(MIN_IOS_VERSION "6.0")

  if(NOT OGRE_BUILD_PLATFORM_ANDROID AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    set(PLATFORM_NAME "macosx")
  elseif(OGRE_BUILD_PLATFORM_APPLE_IOS)
    set(PLATFORM_NAME "$(PLATFORM_NAME)")
  endif()
endif()

# Default build output paths
if (NOT OGRE_ARCHIVE_OUTPUT)
  if(APPLE AND NOT OGRE_BUILD_PLATFORM_ANDROID)
    set(OGRE_ARCHIVE_OUTPUT ${OGRE_BINARY_DIR}/lib/${PLATFORM_NAME})
  else()
    set(OGRE_ARCHIVE_OUTPUT ${OGRE_BINARY_DIR}/lib)
  endif()
endif ()
if (NOT OGRE_LIBRARY_OUTPUT)
  if(APPLE AND NOT OGRE_BUILD_PLATFORM_ANDROID)
    set(OGRE_LIBRARY_OUTPUT ${OGRE_BINARY_DIR}/lib/${PLATFORM_NAME})
  else()
    set(OGRE_LIBRARY_OUTPUT ${OGRE_BINARY_DIR}/lib)
  endif()
endif ()
if (NOT OGRE_RUNTIME_OUTPUT)
  set(OGRE_RUNTIME_OUTPUT ${OGRE_BINARY_DIR}/bin)
endif ()

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
  if(APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    set(OGRE_RELEASE_PATH "/${PLATFORM_NAME}")
  endif()
  if(APPLE AND OGRE_BUILD_PLATFORM_APPLE_IOS)
    set(OGRE_LIB_RELEASE_PATH "/Release")
  endif(APPLE AND OGRE_BUILD_PLATFORM_APPLE_IOS)
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
    configure_file(
	  ${OGRE_TEMPLATES_DIR}/VisualStudioUserFile.vcxproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
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
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_RELEASE_PATH}/Release" CONFIGURATIONS Release None ""
      )
	  install(TARGETS ${TARGETNAME} #EXPORT Ogre-exports
		BUNDLE DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
		RUNTIME DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_RELWDBG_PATH}/RelWithDebInfo" CONFIGURATIONS RelWithDebInfo
      )
	  install(TARGETS ${TARGETNAME} #EXPORT Ogre-exports
		BUNDLE DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
		RUNTIME DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_MINSIZE_PATH}/MinSizeRel" CONFIGURATIONS MinSizeRel
      )
	  install(TARGETS ${TARGETNAME} #EXPORT Ogre-exports
		BUNDLE DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
		RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_DEBUG_PATH}/Debug" CONFIGURATIONS Debug
	  )
	  #install(EXPORT Ogre-exports DESTINATION ${OGRE_LIB_DIRECTORY})
	else()
	  install(TARGETS ${TARGETNAME}
		BUNDLE DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release None ""
		RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release None ""
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_RELEASE_PATH}/Release" CONFIGURATIONS Release None ""
      )
	  install(TARGETS ${TARGETNAME}
		BUNDLE DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
		RUNTIME DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_RELWDBG_PATH}/RelWithDebInfo" CONFIGURATIONS RelWithDebInfo
      )
	  install(TARGETS ${TARGETNAME}
		BUNDLE DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
		RUNTIME DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_MINSIZE_PATH}/MinSizeRel" CONFIGURATIONS MinSizeRel
      )
	  install(TARGETS ${TARGETNAME}
		BUNDLE DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
		RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
		LIBRARY DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
		ARCHIVE DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
		FRAMEWORK DESTINATION "${OGRE_LIB_DIRECTORY}${OGRE_DEBUG_PATH}/Debug" CONFIGURATIONS Debug
	  )
	endif()

endfunction(ogre_install_target)

# setup common target settings
function(ogre_config_common TARGETNAME)
  set_target_properties(${TARGETNAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OGRE_ARCHIVE_OUTPUT}
    LIBRARY_OUTPUT_DIRECTORY ${OGRE_LIBRARY_OUTPUT}
    RUNTIME_OUTPUT_DIRECTORY ${OGRE_RUNTIME_OUTPUT}
  )
  if(OGRE_BUILD_PLATFORM_APPLE_IOS)
    set_xcode_property( ${TARGETNAME} IPHONEOS_DEPLOYMENT_TARGET ${MIN_IOS_VERSION} )
    set_property( TARGET ${TARGETNAME} PROPERTY XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET[arch=arm64] "7.0" )
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
  endif(OGRE_BUILD_PLATFORM_APPLE_IOS)

  if(NOT OGRE_STATIC AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")
    # add GCC visibility flags to shared library build
    set_target_properties(${TARGETNAME} PROPERTIES COMPILE_FLAGS "${OGRE_GCC_VISIBILITY_FLAGS}")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN "${XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN}")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN "${XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN}")
    #set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN[arch=x86_64] "YES")
  endif()

  if(OGRE_BUILD_PLATFORM_WINRT)
    # enable WinRT features, support available since CMake 2.8.8
    set_target_properties(${TARGETNAME} PROPERTIES VS_WINRT_EXTENSIONS "YES")
    set_target_properties(${TARGETNAME} PROPERTIES COMPILE_FLAGS "/bigobj")

    # WinRT uses precompiled headers by default, that needs to be overriden, but unfortunately CMake can`t do this
    #if(NOT ${TARGET_NAME} STREQUAL "OgreMain")
    #  set_target_properties(${TARGETNAME} PROPERTIES COMPILE_FLAGS "/Y-")
    #endif(NOT ${TARGET_NAME} STREQUAL "OgreMain")
  endif(OGRE_BUILD_PLATFORM_WINRT)

  ogre_create_vcproj_userfile(${TARGETNAME})
endfunction(ogre_config_common)

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
  
  if(OGRE_BUILD_PLATFORM_APPLE_IOS)
    set_xcode_property( ${LIBNAME} IPHONEOS_DEPLOYMENT_TARGET ${MIN_IOS_VERSION} )
    set_property( TARGET ${LIBNAME} PROPERTY XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET[arch=arm64] "7.0" )
  endif()

  if (OGRE_INSTALL_PDB)
    # install debug pdb files
    if (OGRE_STATIC)
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_LIB_DEBUG_PATH}/${LIBNAME}Static_d.pdb
	    DESTINATION ${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_LIB_RELWDBG_PATH}/${LIBNAME}Static.pdb
	    DESTINATION ${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}
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

function(ogre_config_framework LIBNAME)
  if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
      set_target_properties(${LIBNAME} PROPERTIES FRAMEWORK TRUE)

      # Set the INSTALL_PATH so that frameworks can be installed in the application package
      set_target_properties(${LIBNAME}
         PROPERTIES BUILD_WITH_INSTALL_RPATH 1
         INSTALL_NAME_DIR "@executable_path/../Frameworks"
      )
      set_target_properties(${LIBNAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES};${PLATFORM_HEADERS};" )
      set_target_properties(${LIBNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
      set_target_properties(${LIBNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${OGRE_SOURCE_DIR}/OgreMain/include/OgreStableHeaders.h")
      set_target_properties(${LIBNAME} PROPERTIES RESOURCE "${RESOURCE_FILES}")
      set_source_files_properties("${RESOURCE_FILES}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

      set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME ${LIBNAME})
  endif()
endfunction(ogre_config_framework)

# setup plugin build
function(ogre_config_plugin PLUGINNAME)
  ogre_config_common(${PLUGINNAME})
  set_target_properties(${PLUGINNAME} PROPERTIES VERSION ${OGRE_SOVERSION})
  if (OGRE_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${PLUGINNAME} PROPERTIES OUTPUT_NAME ${PLUGINNAME}Static)

    if(OGRE_BUILD_PLATFORM_APPLE_IOS)
      set_xcode_property( ${PLUGINNAME} IPHONEOS_DEPLOYMENT_TARGET ${MIN_IOS_VERSION} )
      set_property( TARGET ${PLUGINNAME} PROPERTY XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET[arch=arm64] "7.0" )
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
    endif(OGRE_BUILD_PLATFORM_APPLE_IOS)
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
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_LIB_DEBUG_PATH}/${PLUGINNAME}Static_d.pdb
	    DESTINATION ${OGRE_LIB_DIRECTORY}${OGRE_LIB_DEBUG_PATH}/opt
		CONFIGURATIONS Debug
	  )
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_LIB_RELWDBG_PATH}/${PLUGINNAME}Static.pdb
	    DESTINATION ${OGRE_LIB_DIRECTORY}${OGRE_LIB_RELWDBG_PATH}/opt
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
  ogre_config_common(${SAMPLENAME})

  # set install RPATH for Unix systems
  if (UNIX AND OGRE_FULL_RPATH)
    set_property(TARGET ${SAMPLENAME} APPEND PROPERTY
      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY})
    set_property(TARGET ${SAMPLENAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif ()
  
  if (APPLE)
    # On OS X, create .app bundle
    set_property(TARGET ${SAMPLENAME} PROPERTY MACOSX_BUNDLE TRUE)
    if (NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
      # Add the path where the Ogre framework was found
      if(${OGRE_FRAMEWORK_PATH})
        set_target_properties(${SAMPLENAME} PROPERTIES
          COMPILE_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
          LINK_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
        )
      endif()
    else()
      set_xcode_property( ${SAMPLENAME} IPHONEOS_DEPLOYMENT_TARGET ${MIN_IOS_VERSION} )
      set_property( TARGET ${SAMPLENAME} PROPERTY XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET[arch=arm64] "7.0" )
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

  if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS AND OGRE_SDK_BUILD)
    # Add the path where the Ogre framework was found
    if(NOT ${OGRE_FRAMEWORK_PATH} STREQUAL "")
      set_target_properties(${SAMPLENAME} PROPERTIES
        COMPILE_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
        LINK_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
      )
    endif()
  endif(APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS AND OGRE_SDK_BUILD)
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

  if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS AND OGRE_SDK_BUILD)
    # Add the path where the Ogre framework was found
    if(NOT ${OGRE_FRAMEWORK_PATH} STREQUAL "")
      set_target_properties(${SAMPLENAME} PROPERTIES
        COMPILE_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
        LINK_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
      )
    endif()
  endif(APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS AND OGRE_SDK_BUILD)

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
      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY})
    set_property(TARGET ${TOOLNAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif ()

  if (OGRE_INSTALL_TOOLS)
    ogre_install_target(${TOOLNAME} "" FALSE)
    if (OGRE_INSTALL_PDB)
      # install debug pdb files
      install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${TOOLNAME}_d.pdb
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
