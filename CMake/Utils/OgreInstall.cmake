if (WIN32)
  set(OGRE_RELEASE_PATH "/Release")
  set(OGRE_DEBUG_PATH "/Debug")
  set(OGRE_PLUGIN_PATH "/opt")
elseif (UNIX)
  set(OGRE_RELEASE_PATH "")
  set(OGRE_DEBUG_PATH "")
  set(OGRE_PLUGIN_PATH "/OGRE")
elseif (APPLE)
  # TODO
endif ()

function(ogre_install_lib LIBNAME)
  # add static prefix, if compiling static version
  if (OGRE_STATIC)
    set_target_properties(${LIBNAME} PROPERTIES
      OUTPUT_NAME ${LIBNAME}Static
    )
  endif ()
  install(TARGETS ${LIBNAME}
    RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo
	LIBRARY DESTINATION "lib" CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
	ARCHIVE DESTINATION "lib" CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
  )
  install(TARGETS ${LIBNAME}
    RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
	LIBRARY DESTINATION "lib" CONFIGURATIONS DEBUG
	ARCHIVE DESTINATION "lib" CONFIGURATIONS DEBUG
  )
endfunction(ogre_install_lib)

function(ogre_install_plugin PLUGINNAME)
  # add static prefix, if compiling static version
  if (OGRE_STATIC)
    set_target_properties(${PLUGINNAME} PROPERTIES
      OUTPUT_NAME ${PLUGINNAME}Static
    )
  endif ()
  install(TARGETS ${PLUGINNAME}
    RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo
	LIBRARY DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
	ARCHIVE DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
  )
  install(TARGETS ${PLUGINNAME}
    RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
	LIBRARY DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS DEBUG
	ARCHIVE DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS DEBUG
  )
endfunction(ogre_install_plugin)

function(ogre_install_sample SAMPLENAME)
  if (OGRE_INSTALL_SAMPLES)
    install(TARGETS ${SAMPLENAME}
      RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo None OPTIONAL
    )
    install(TARGETS ${SAMPLENAME}
      RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG OPTIONAL
    )
  endif ()	
endfunction(ogre_install_sample)

