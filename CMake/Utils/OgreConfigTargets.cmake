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
elseif (UNIX)
  set(OGRE_RELEASE_PATH "")
  set(OGRE_RELWDBG_PATH "")
  set(OGRE_MINSIZE_PATH "")
  set(OGRE_DEBUG_PATH "/debug")
  set(OGRE_LIB_RELEASE_PATH "")
  set(OGRE_LIB_RELWDBG_PATH "")
  set(OGRE_LIB_MINSIZE_PATH "")
  set(OGRE_LIB_DEBUG_PATH "")
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

# install targets according to current build type
function(ogre_install_target TARGETNAME SUFFIX)
  install(TARGETS ${TARGETNAME}
    RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release None ""
    LIBRARY DESTINATION "lib${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
    ARCHIVE DESTINATION "lib${OGRE_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
    FRAMEWORK DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release None ""
  )
  install(TARGETS ${TARGETNAME}
    RUNTIME DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
    LIBRARY DESTINATION "lib${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
    ARCHIVE DESTINATION "lib${OGRE_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
    FRAMEWORK DESTINATION "bin${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
  )
  install(TARGETS ${TARGETNAME}
    RUNTIME DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
    LIBRARY DESTINATION "lib${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
    ARCHIVE DESTINATION "lib${OGRE_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
    FRAMEWORK DESTINATION "bin${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
  )
  install(TARGETS ${TARGETNAME}
    RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
    LIBRARY DESTINATION "lib${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
    ARCHIVE DESTINATION "lib${OGRE_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
    FRAMEWORK DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
  )
endfunction(ogre_install_target)

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
  ogre_install_target(${LIBNAME} "")
  
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
  endif (OGRE_STATIC)
  ogre_install_target(${PLUGINNAME} ${OGRE_PLUGIN_PATH})

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
    # now plugins
    add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
      COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins)
    if (OGRE_BUILD_RENDERSYSTEM_GL)
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
        COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/RenderSystem_GL.dylib 
          ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
      )
    endif ()
	if (OGRE_BUILD_PLUGIN_BSP)    
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_BSPSceneManager.dylib 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
      )
    endif()
	if (OGRE_BUILD_PLUGIN_CG)    
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_CgProgramManager.dylib 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
      )
    endif()
	if (OGRE_BUILD_PLUGIN_OCTREE)    
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_OctreeSceneManager.dylib 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
      )
    endif()
	if (OGRE_BUILD_PLUGIN_PCZ)    
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
        COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_PCZSceneManager.dylib 
          ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/    
      )
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_OctreeZone.dylib 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
      )
    endif()
	if (OGRE_BUILD_PLUGIN_PFX)    
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
      COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_ParticleFX.dylib 
        ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
      )
    endif()
  endif (APPLE)

  if (OGRE_INSTALL_SAMPLES)
    ogre_install_target(${SAMPLENAME} "")
    if (OGRE_INSTALL_PDB)
      # install debug pdb files
      install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/${SAMPLENAME}.pdb
        DESTINATION bin${OGRE_DEBUG_PATH} CONFIGURATIONS Debug
        )
      install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_RELWDBG_PATH}/${SAMPLENAME}.pdb
        DESTINATION bin${OGRE_RELWDBG_PATH} CONFIGURATIONS RelWithDebInfo
        )
    endif ()
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
    ogre_install_target(${TOOLNAME} "")
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
