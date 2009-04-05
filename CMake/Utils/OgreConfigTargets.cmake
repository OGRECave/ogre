# Configure settings and install targets

if (WIN32)
  set(OGRE_RELEASE_PATH "/Release")
  set(OGRE_DEBUG_PATH "/Debug")
  set(OGRE_PLUGIN_PATH "/opt")
elseif (UNIX)
  set(OGRE_RELEASE_PATH "")
  set(OGRE_DEBUG_PATH "/debug")
  set(OGRE_PLUGIN_PATH "/OGRE")
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

# setup common target settings
function(ogre_config_common TARGETNAME)
  set_target_properties(${TARGETNAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/bin
  )
  ogre_create_vcproj_userfile(${TARGETNAME})
endfunction(ogre_config_common)


# setup library build
function(ogre_config_lib LIBNAME)
  ogre_config_common(${LIBNAME})
  if (OGRE_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME ${LIBNAME}Static)
  else (OGRE_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX)
      # add GCC visibility flags to shared library build
      set_target_properties(${LIBNAME} PROPERTIES COMPILE_FLAGS "${OGRE_GCC_VISIBILITY_FLAGS}")
	endif (CMAKE_COMPILER_IS_GNUCXX)
  endif (OGRE_STATIC)
  install(TARGETS ${LIBNAME}
    RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None ""
    LIBRARY DESTINATION "lib" CONFIGURATIONS Release MinSizeRel RelWithDebInfo None ""
    ARCHIVE DESTINATION "lib" CONFIGURATIONS Release MinSizeRel RelWithDebInfo None ""
    FRAMEWORK DESTINATION "bin${OGRE_RELEASE_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None ""
  )
  install(TARGETS ${LIBNAME}
    RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
    LIBRARY DESTINATION "lib" CONFIGURATIONS DEBUG
    ARCHIVE DESTINATION "lib" CONFIGURATIONS DEBUG
    FRAMEWORK DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
  )
  
  if (OGRE_INSTALL_PDB)
    # install debug pdb files
    if (OGRE_STATIC)
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_DEBUG_PATH}/${LIBNAME}Static_d.pdb
	    DESTINATION lib
		CONFIGURATIONS Debug
	  )
	else ()
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${LIBNAME}_d.pdb
	    DESTINATION bin${OGRE_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	endif ()
  endif ()
endfunction(ogre_config_lib)

# setup plugin build
function(ogre_config_plugin PLUGINNAME)
  ogre_config_common(${PLUGINNAME})
  if (OGRE_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${PLUGINNAME} PROPERTIES OUTPUT_NAME ${PLUGINNAME}Static)
  else (OGRE_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX)
      # add GCC visibility flags to shared library build
      set_target_properties(${PLUGINNAME} PROPERTIES COMPILE_FLAGS "${OGRE_GCC_VISIBILITY_FLAGS}")
      # disable "lib" prefix on Unix
      set_target_properties(${PLUGINNAME} PROPERTIES PREFIX "")
	endif (CMAKE_COMPILER_IS_GNUCXX)
	
	if (APPLE)
	  # copy plugin into Ogre.framework/Contents/Resources
	  # NOTE: $(CONFIGURATION) is resolved only at build time, not CMake time!
	  set (OGRE_FWK_CONTENTS_PATH 
        ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Ogre.framework/Contents)
      add_custom_command(TARGET ${PLUGINNAME} POST_BUILD
        COMMAND mkdir ARGS -p ${OGRE_FWK_CONTENTS_PATH}/Resources
        COMMAND cp ARGS -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/${PLUGINNAME}.dylib 
        ${OGRE_FWK_CONTENTS_PATH}/Resources/
	  )
	  
	endif (APPLE)
	
  endif (OGRE_STATIC)
  install(TARGETS ${PLUGINNAME}
    RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None ""
    LIBRARY DESTINATION "lib${OGRE_PLUGIN_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None ""
    ARCHIVE DESTINATION "lib${OGRE_PLUGIN_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None ""
  )
  install(TARGETS ${PLUGINNAME}
    RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
    LIBRARY DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS DEBUG
    ARCHIVE DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS DEBUG
  )

  if (OGRE_INSTALL_PDB)
    # install debug pdb files
    if (OGRE_STATIC)
	  install(FILES ${OGRE_BINARY_DIR}/lib${OGRE_DEBUG_PATH}/${PLUGINNAME}Static_d.pdb
	    DESTINATION lib/opt
		CONFIGURATIONS Debug
	  )
	else ()
	  install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${PLUGINNAME}_d.pdb
	    DESTINATION bin${OGRE_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	endif ()
  endif ()
endfunction(ogre_config_plugin)

# setup Ogre demo build
function(ogre_config_sample SAMPLENAME)
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
    # also, symlink frameworks so .app is standalone
    # NOTE: $(CONFIGURATION) is not resolvable at CMake run time, it's only 
    # valid at build time (hence parenthesis rather than braces)
    set (OGRE_SAMPLE_CONTENTS_PATH 
      ${CMAKE_BINARY_DIR}/bin/$(CONFIGURATION)/${SAMPLENAME}.app/Contents)
    add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
      COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Ogre.framework 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
      COMMAND ln ARGS -s -f ${CMAKE_SOURCE_DIR}/Dependencies/Cg.framework 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
      COMMAND ln ARGS -s -f ${CMAKE_SOURCE_DIR}/Dependencies/CEGUI.framework 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
    )
    # now cfg files
    add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
      COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Resources
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/plugins.cfg 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/resources.cfg 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/media.cfg 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/quake3settings.cfg 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
    )
    
    
    
  endif ()

  if (OGRE_INSTALL_SAMPLES)
    install(TARGETS ${SAMPLENAME}
      RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" 
        CONFIGURATIONS Release MinSizeRel RelWithDebInfo None OPTIONAL
    )
    install(TARGETS ${SAMPLENAME}
      RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug OPTIONAL
    )
  endif ()	

  if (OGRE_INSTALL_PDB)
    # install debug pdb files
	install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${SAMPLENAME}.pdb
	  DESTINATION bin${OGRE_DEBUG_PATH}
	  CONFIGURATIONS Debug
	)
  endif ()
endfunction(ogre_config_sample)

# setup Ogre tool build
function(ogre_config_tool TOOLNAME)
  ogre_config_common(${TOOLNAME})

  # set install RPATH for Unix systems
  if (UNIX AND OGRE_FULL_RPATH)
    set_property(TARGET ${TOOLNAME} APPEND PROPERTY
      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set_property(TARGET ${TOOLNAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif ()

  if (OGRE_INSTALL_TOOLS)
    install(TARGETS ${TOOLNAME}
      RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" 
        CONFIGURATIONS Release MinSizeRel RelWithDebInfo None OPTIONAL
    )
    install(TARGETS ${TOOLNAME}
      RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug OPTIONAL
    )
  endif ()	

  if (OGRE_INSTALL_PDB)
    # install debug pdb files
	install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${TOOLNAME}.pdb
	  DESTINATION bin${OGRE_DEBUG_PATH}
	  CONFIGURATIONS Debug
	)
  endif ()
endfunction(ogre_config_tool)
