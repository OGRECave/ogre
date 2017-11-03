#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

##################################################################
# Generate and install the config files needed for the samples
##################################################################

if (WIN32)
  set(OGRE_MEDIA_PATH "Media")
  if (WINDOWS_STORE OR WINDOWS_PHONE)
    set(OGRE_MEDIA_DIR_REL "${OGRE_MEDIA_PATH}")
    set(OGRE_MEDIA_DIR_DBG "${OGRE_MEDIA_PATH}")
    set(OGRE_TEST_MEDIA_DIR_REL "${OGRE_MEDIA_PATH}")
    set(OGRE_TEST_MEDIA_DIR_DBG "${OGRE_MEDIA_PATH}")
  else()
    set(OGRE_MEDIA_DIR_REL "../../${OGRE_MEDIA_PATH}")
    set(OGRE_MEDIA_DIR_DBG "../../${OGRE_MEDIA_PATH}")
    set(OGRE_TEST_MEDIA_DIR_REL "../../Tests/${OGRE_MEDIA_PATH}")
    set(OGRE_TEST_MEDIA_DIR_DBG "../../Tests/${OGRE_MEDIA_PATH}")
  endif()
  set(OGRE_PLUGIN_DIR_REL ".")
  set(OGRE_PLUGIN_DIR_DBG ".")
  set(OGRE_SAMPLES_DIR_REL ".")
  set(OGRE_SAMPLES_DIR_DBG ".")
  set(OGRE_CFG_INSTALL_PATH "bin")
elseif (APPLE)
  set(OGRE_MEDIA_PATH "Media")
  if(OGRE_BUILD_PLATFORM_APPLE_IOS)
	#set(OGRE_MEDIA_DIR_REL "${OGRE_MEDIA_PATH}")
	#set(OGRE_MEDIA_DIR_DBG "${OGRE_MEDIA_PATH}")
	set(OGRE_MEDIA_DIR_REL ".")
	set(OGRE_MEDIA_DIR_DBG ".")
    set(OGRE_TEST_MEDIA_DIR_REL "../../Tests/${OGRE_MEDIA_PATH}")
    set(OGRE_TEST_MEDIA_DIR_DBG "../../Tests/${OGRE_MEDIA_PATH}")
  else()
    if(OGRE_INSTALL_SAMPLES_SOURCE)
      set(OGRE_MEDIA_DIR_REL "../../../${OGRE_MEDIA_PATH}")
      set(OGRE_MEDIA_DIR_DBG "../../../${OGRE_MEDIA_PATH}")
    else()
      set(OGRE_MEDIA_DIR_REL "../../../../Samples/${OGRE_MEDIA_PATH}")
      set(OGRE_MEDIA_DIR_DBG "../../../../Samples/${OGRE_MEDIA_PATH}")
      set(OGRE_TEST_MEDIA_DIR_REL "../../../../Tests/${OGRE_MEDIA_PATH}")
      set(OGRE_TEST_MEDIA_DIR_DBG "../../../../Tests/${OGRE_MEDIA_PATH}")
    endif()
  endif()
  set(OGRE_PLUGIN_DIR_REL "")
  set(OGRE_PLUGIN_DIR_DBG "")
  set(OGRE_SAMPLES_DIR_REL "")
  set(OGRE_SAMPLES_DIR_DBG "")
  set(OGRE_CFG_INSTALL_PATH "bin")
elseif (UNIX)
  set(OGRE_MEDIA_PATH "share/OGRE/Media")
  set(OGRE_MEDIA_DIR_REL "${CMAKE_INSTALL_PREFIX}/${OGRE_MEDIA_PATH}")
  set(OGRE_MEDIA_DIR_DBG "${CMAKE_INSTALL_PREFIX}/${OGRE_MEDIA_PATH}")
  set(OGRE_TEST_MEDIA_DIR_REL "${CMAKE_INSTALL_PREFIX}/Tests/Media")
  set(OGRE_TEST_MEDIA_DIR_DBG "${CMAKE_INSTALL_PREFIX}/Tests/Media")
  set(OGRE_PLUGIN_DIR_REL "${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY}/OGRE")
  set(OGRE_PLUGIN_DIR_DBG "${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY}/OGRE")
  set(OGRE_SAMPLES_DIR_REL "${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY}/OGRE/Samples")
  set(OGRE_SAMPLES_DIR_DBG "${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY}/OGRE/Samples")
  set(OGRE_CFG_INSTALL_PATH "share/OGRE")
endif ()

# configure plugins.cfg
if (NOT OGRE_BUILD_RENDERSYSTEM_D3D11)
  set(OGRE_COMMENT_RENDERSYSTEM_D3D11 "#")
endif ()
if (CMAKE_SYSTEM_VERSION VERSION_LESS "6.0")
  set(OGRE_COMMENT_RENDERSYSTEM_D3D11 "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
  set(OGRE_COMMENT_RENDERSYSTEM_GL3PLUS "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_GLES)
  set(OGRE_COMMENT_RENDERSYSTEM_GLES "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_GLES2)
  set(OGRE_COMMENT_RENDERSYSTEM_GLES2 "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_METAL)
  set(OGRE_COMMENT_RENDERSYSTEM_METAL "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_PFX)
  set(OGRE_COMMENT_PLUGIN_PARTICLEFX "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_CG)
  set(OGRE_COMMENT_PLUGIN_CG "#")
endif ()
if (NOT OGRE_BUILD_COMPONENT_TERRAIN)
  set(OGRE_COMMENT_COMPONENT_TERRAIN "#")
endif ()
if (NOT OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
  set(OGRE_COMMENT_COMPONENT_RTSHADERSYSTEM "#")
endif ()
if (NOT OGRE_BUILD_COMPONENT_VOLUME)
  set(OGRE_COMMENT_COMPONENT_VOLUME "#")
endif ()
if (NOT OGRE_BUILD_COMPONENT_TERRAIN OR NOT OGRE_BUILD_COMPONENT_PAGING)
  set(OGRE_COMMENT_SAMPLE_ENDLESSWORLD "#")
endif ()
if( NOT OGRE_PROFILING_PROVIDER OR OGRE_PROFILING_PROVIDER STREQUAL "none" )
    set( OGRE_COMMENT_PROFILER_PATH "#" )
endif()


# CREATE CONFIG FILES - INSTALL VERSIONS
# create resources2.cfg
configure_file(${OGRE_TEMPLATES_DIR}/resources2.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/resources2.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/resources2.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/resources2.cfg)
# create resources.cfg
configure_file(${OGRE_TEMPLATES_DIR}/resources_d.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/resources_d.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/resources.cfg)
# create plugins.cfg
configure_file(${OGRE_TEMPLATES_DIR}/plugins_d.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/plugins_d.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/plugins.cfg)
# create plugins_tools.cfg
configure_file(${OGRE_TEMPLATES_DIR}/plugins_tools_d.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/plugins_tools_d.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/plugins_tools.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/plugins_tools.cfg)
# create samples.cfg
configure_file(${OGRE_TEMPLATES_DIR}/samples_d.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/samples_d.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/samples.cfg)
# create samples.cfg
configure_file(${OGRE_TEMPLATES_DIR}/tests.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/tests.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/tests_d.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/tests_d.cfg)

# install resource files
if (OGRE_INSTALL_SAMPLES OR OGRE_INSTALL_SAMPLES_SOURCE)
  install(FILES 
    ${OGRE_BINARY_DIR}/inst/bin/debug/resources2.cfg
    ${OGRE_BINARY_DIR}/inst/bin/debug/resources_d.cfg
    ${OGRE_BINARY_DIR}/inst/bin/debug/plugins_d.cfg
	${OGRE_BINARY_DIR}/inst/bin/debug/plugins_tools_d.cfg
	${OGRE_BINARY_DIR}/inst/bin/debug/samples_d.cfg
	${OGRE_BINARY_DIR}/inst/bin/debug/tests_d.cfg
    DESTINATION "${OGRE_CFG_INSTALL_PATH}${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug
  )
  install(FILES 
    ${OGRE_BINARY_DIR}/inst/bin/release/resources2.cfg
    ${OGRE_BINARY_DIR}/inst/bin/release/resources.cfg
    ${OGRE_BINARY_DIR}/inst/bin/release/plugins.cfg
	${OGRE_BINARY_DIR}/inst/bin/release/plugins_tools.cfg
	${OGRE_BINARY_DIR}/inst/bin/release/samples.cfg
	${OGRE_BINARY_DIR}/inst/bin/release/tests.cfg
    DESTINATION "${OGRE_CFG_INSTALL_PATH}${OGRE_RELEASE_PATH}" CONFIGURATIONS Release None ""
  )
  install(FILES 
    ${OGRE_BINARY_DIR}/inst/bin/release/resources2.cfg
    ${OGRE_BINARY_DIR}/inst/bin/release/resources.cfg
    ${OGRE_BINARY_DIR}/inst/bin/release/plugins.cfg
	${OGRE_BINARY_DIR}/inst/bin/release/plugins_tools.cfg
	${OGRE_BINARY_DIR}/inst/bin/release/samples.cfg
	${OGRE_BINARY_DIR}/inst/bin/release/tests.cfg
	DESTINATION "${OGRE_CFG_INSTALL_PATH}${OGRE_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
  )
  install(FILES 
    ${OGRE_BINARY_DIR}/inst/bin/release/resources2.cfg
    ${OGRE_BINARY_DIR}/inst/bin/release/resources.cfg
    ${OGRE_BINARY_DIR}/inst/bin/release/plugins.cfg
	${OGRE_BINARY_DIR}/inst/bin/release/plugins_tools.cfg
	${OGRE_BINARY_DIR}/inst/bin/release/samples.cfg
	${OGRE_BINARY_DIR}/inst/bin/release/tests.cfg
	DESTINATION "${OGRE_CFG_INSTALL_PATH}${OGRE_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
  )

  # Need a special case here for the iOS SDK, configuration is not being matched, could be a CMake bug.
  if (OGRE_BUILD_PLATFORM_APPLE_IOS)
    install(FILES 
      ${OGRE_BINARY_DIR}/inst/bin/release/resources2.cfg
      ${OGRE_BINARY_DIR}/inst/bin/release/resources.cfg
      ${OGRE_BINARY_DIR}/inst/bin/release/plugins.cfg
	  ${OGRE_BINARY_DIR}/inst/bin/release/plugins_tools.cfg
      ${OGRE_BINARY_DIR}/inst/bin/release/samples.cfg
      ${OGRE_BINARY_DIR}/inst/bin/release/tests.cfg
      DESTINATION "${OGRE_CFG_INSTALL_PATH}${OGRE_RELEASE_PATH}"
    )
  endif()

endif (OGRE_INSTALL_SAMPLES OR OGRE_INSTALL_SAMPLES_SOURCE)


# CREATE CONFIG FILES - BUILD DIR VERSIONS
if (NOT (OGRE_BUILD_PLATFORM_APPLE_IOS OR WINDOWS_STORE OR WINDOWS_PHONE))
	if( NOT APPLE )
		set(OGRE_MEDIA_DIR_REL "${OGRE_SOURCE_DIR}/Samples/Media")
		set(OGRE_MEDIA_DIR_DBG "${OGRE_SOURCE_DIR}/Samples/Media")
	else()
		set(OGRE_MEDIA_DIR_REL "Contents/Resources")
		set(OGRE_MEDIA_DIR_DBG "Contents/Resources")
	endif()
	set(OGRE_TEST_MEDIA_DIR_REL "${OGRE_SOURCE_DIR}/Tests/Media")
	set(OGRE_TEST_MEDIA_DIR_DBG "${OGRE_SOURCE_DIR}/Tests/Media")
else ()
	# iOS needs to use relative paths in the config files
	set(OGRE_TEST_MEDIA_DIR_REL "${OGRE_MEDIA_PATH}")
	set(OGRE_TEST_MEDIA_DIR_DBG "${OGRE_MEDIA_PATH}")
endif ()

if( APPLE )
  if (OGRE_BUILD_PLATFORM_APPLE_IOS)
	set( OGRE_MEDIA_PACKS_DIR "." )
	set( OGRE_MEDIA_TEXTURES_DIR "." )
  else()
	set( OGRE_MEDIA_PACKS_DIR "Contents/Resources" )
	set( OGRE_MEDIA_TEXTURES_DIR "Contents/Resources" )
  endif()
else()
	set( OGRE_MEDIA_PACKS_DIR "${OGRE_MEDIA_DIR_REL}/packs" )
	set( OGRE_MEDIA_TEXTURES_DIR "${OGRE_MEDIA_DIR_REL}/materials" )
endif()

if (WIN32)
  set(OGRE_PLUGIN_DIR_REL ".")
  set(OGRE_PLUGIN_DIR_DBG ".")
  set(OGRE_SAMPLES_DIR_REL ".")
  set(OGRE_SAMPLES_DIR_DBG ".")
elseif (APPLE)
  # not used on OS X, uses Resources
  set(OGRE_PLUGIN_DIR_REL "")
  set(OGRE_PLUGIN_DIR_DBG "")
  set(OGRE_SAMPLES_DIR_REL "")
  set(OGRE_SAMPLES_DIR_DBG "")
elseif (UNIX)
  set(OGRE_PLUGIN_DIR_REL "${OGRE_BINARY_DIR}/lib")
  set(OGRE_PLUGIN_DIR_DBG "${OGRE_BINARY_DIR}/lib")
  set(OGRE_SAMPLES_DIR_REL "${OGRE_BINARY_DIR}/lib")
  set(OGRE_SAMPLES_DIR_DBG "${OGRE_BINARY_DIR}/lib")
endif ()

if (WINDOWS_STORE OR WINDOWS_PHONE)
  # These platfroms requires all resources to be packaged inside the application bundle,
  # therefore install versions of configs would be copied and added as content file to each project.
elseif (MSVC AND NOT NMAKE)
  # create resources2.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/resources2.cfg.in ${OGRE_BINARY_DIR}/bin/debug/resources2.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources2.cfg.in ${OGRE_BINARY_DIR}/bin/release/resources2.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources2.cfg.in ${OGRE_BINARY_DIR}/bin/relwithdebinfo/resources2.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources2.cfg.in ${OGRE_BINARY_DIR}/bin/minsizerel/resources2.cfg)
  # create resources.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/resources_d.cfg.in ${OGRE_BINARY_DIR}/bin/debug/resources_d.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${OGRE_BINARY_DIR}/bin/release/resources.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${OGRE_BINARY_DIR}/bin/relwithdebinfo/resources.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${OGRE_BINARY_DIR}/bin/minsizerel/resources.cfg)
  # create plugins.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/plugins_d.cfg.in ${OGRE_BINARY_DIR}/bin/debug/plugins_d.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${OGRE_BINARY_DIR}/bin/release/plugins.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${OGRE_BINARY_DIR}/bin/relwithdebinfo/plugins.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${OGRE_BINARY_DIR}/bin/minsizerel/plugins.cfg)
  # create plugins_tools.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/plugins_tools_d.cfg.in ${OGRE_BINARY_DIR}/bin/debug/plugins_tools_d.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins_tools.cfg.in ${OGRE_BINARY_DIR}/bin/release/plugins_tools.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins_tools.cfg.in ${OGRE_BINARY_DIR}/bin/relwithdebinfo/plugins_tools.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins_tools.cfg.in ${OGRE_BINARY_DIR}/bin/minsizerel/plugins_tools.cfg)
  # create samples.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/samples_d.cfg.in ${OGRE_BINARY_DIR}/bin/debug/samples_d.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${OGRE_BINARY_DIR}/bin/release/samples.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${OGRE_BINARY_DIR}/bin/relwithdebinfo/samples.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${OGRE_BINARY_DIR}/bin/minsizerel/samples.cfg)
  # create tests.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/tests_d.cfg.in ${OGRE_BINARY_DIR}/bin/debug/tests_d.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/tests.cfg.in ${OGRE_BINARY_DIR}/bin/release/tests.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/tests.cfg.in ${OGRE_BINARY_DIR}/bin/relwithdebinfo/tests.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/tests.cfg.in ${OGRE_BINARY_DIR}/bin/minsizerel/tests.cfg)
else() # other OS only need one cfg file
  string(TOLOWER "${CMAKE_BUILD_TYPE}" OGRE_BUILD_TYPE)
  if (OGRE_BUILD_TYPE STREQUAL "debug" AND NOT APPLE)
    set(OGRE_CFG_SUFFIX "_d")
  endif ()
  # create resources2.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/resources2.cfg.in ${OGRE_BINARY_DIR}/bin/resources2.cfg)
  # create resources.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/resources${OGRE_CFG_SUFFIX}.cfg.in ${OGRE_BINARY_DIR}/bin/resources${OGRE_CFG_SUFFIX}.cfg)
  # create plugins.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/plugins${OGRE_CFG_SUFFIX}.cfg.in ${OGRE_BINARY_DIR}/bin/plugins${OGRE_CFG_SUFFIX}.cfg)
  # create plugins_tools.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/plugins_tools${OGRE_CFG_SUFFIX}.cfg.in ${OGRE_BINARY_DIR}/bin/plugins_tools${OGRE_CFG_SUFFIX}.cfg)
  # create samples.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/samples${OGRE_CFG_SUFFIX}.cfg.in ${OGRE_BINARY_DIR}/bin/samples${OGRE_CFG_SUFFIX}.cfg)
  # create tests.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/tests${OGRE_CFG_SUFFIX}.cfg.in ${OGRE_BINARY_DIR}/bin/tests${OGRE_CFG_SUFFIX}.cfg)
endif ()

