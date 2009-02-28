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

function(ogre_config_build TARGETNAME)
  set_target_properties(${TARGETNAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/lib
	LIBRARY_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/lib
	RUNTIME_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/bin
  )
  ogre_create_vcproj_userfile(${TARGETNAME})
endfunction(ogre_config_build)
