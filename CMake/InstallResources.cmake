##################################################################
# Generate and install the config files needed for the samples
##################################################################

if (WIN32)
  set(OGRE_MEDIA_PATH "media")
  set(OGRE_MEDIA_DIR_REL "../../${OGRE_MEDIA_PATH}")
  set(OGRE_MEDIA_DIR_DBG "../../${OGRE_MEDIA_PATH}")
  set(OGRE_PLUGIN_DIR_REL ".")
  set(OGRE_PLUGIN_DIR_DBG ".")
elseif (UNIX)
  set(OGRE_MEDIA_PATH "share/OGRE/media")
  set(OGRE_MEDIA_DIR_REL "../${OGRE_MEDIA_PATH}")
  set(OGRE_MEDIA_DIR_DBG "../../${OGRE_MEDIA_PATH}")
  set(OGRE_PLUGIN_DIR_REL "../lib/OGRE")
  set(OGRE_PLUGIN_DIR_DBG "../../lib/OGRE")
endif ()

# configure plugins.cfg
if (NOT OGRE_BUILD_RENDERSYSTEM_D3D9)
  set(OGRE_COMMENT_RENDERSYSTEM_D3D9 "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_D3D10)
  set(OGRE_COMMENT_RENDERSYSTEM_D3D10 "#")
endif ()
if (CMAKE_SYSTEM_VERSION VERSION_LESS "6.0")
  set(OGRE_COMMENT_RENDERSYSTEM_D3D10 "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_GL)
  set(OGRE_COMMENT_RENDERSYSTEM_GL "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_BSP)
  set(OGRE_COMMENT_PLUGIN_BSP "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_OCTREE)
  set(OGRE_COMMENT_PLUGIN_OCTREE "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_PCZ)
  set(OGRE_COMMENT_PLUGIN_PCZ "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_PFX)
  set(OGRE_COMMENT_PLUGIN_PARTICLEFX "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_CG)
  set(OGRE_COMMENT_PLUGIN_CG "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_PCZ)
  set(OGRE_COMMENT_PLUGIN_PCZ "#")
endif ()



# CREATE CONFIG FILES - INSTALL VERSIONS
# create resources.cfg
configure_file(${OGRE_TEMPLATES_DIR}/resources_d.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/resources.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/resources.cfg)
# create plugins.cfg
configure_file(${OGRE_TEMPLATES_DIR}/plugins_d.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/plugins.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/plugins.cfg)
# create media.cfg
configure_file(${OGRE_TEMPLATES_DIR}/media.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/media.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/media.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/media.cfg)
# create quake3settings.cfg
configure_file(${OGRE_TEMPLATES_DIR}/quake3settings_d.cfg.in ${OGRE_BINARY_DIR}/inst/bin/debug/quake3settings.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/quake3settings.cfg.in ${OGRE_BINARY_DIR}/inst/bin/release/quake3settings.cfg)

# install resource files
install(FILES 
  ${OGRE_BINARY_DIR}/inst/bin/debug/resources.cfg
  ${OGRE_BINARY_DIR}/inst/bin/debug/plugins.cfg
  ${OGRE_BINARY_DIR}/inst/bin/debug/media.cfg
  ${OGRE_BINARY_DIR}/inst/bin/debug/quake3settings.cfg
  DESTINATION "bin${OGRE_DEBUG_PATH}"
  CONFIGURATIONS Debug
)
install(FILES 
  ${OGRE_BINARY_DIR}/inst/bin/release/resources.cfg
  ${OGRE_BINARY_DIR}/inst/bin/release/plugins.cfg
  ${OGRE_BINARY_DIR}/inst/bin/release/media.cfg
  ${OGRE_BINARY_DIR}/inst/bin/release/quake3settings.cfg
  DESTINATION "bin${OGRE_RELEASE_PATH}"
  CONFIGURATIONS Release MinSizeRel RelWithDebInfo None ""
)
#if (UNIX AND CMAKE_BUILD_TYPE STREQUAL "")
#  install(FILES 
#    ${OGRE_BINARY_DIR}/inst/bin/release/resources.cfg
#    ${OGRE_BINARY_DIR}/inst/bin/release/plugins.cfg
#    ${OGRE_BINARY_DIR}/inst/bin/release/media.cfg
#    ${OGRE_BINARY_DIR}/inst/bin/release/quake3settings.cfg
#    DESTINATION "bin${OGRE_RELEASE_PATH}"
#  )
#endif()


# CREATE CONFIG FILES - BUILD DIR VERSIONS
set(OGRE_MEDIA_DIR_REL "${OGRE_SOURCE_DIR}/Samples/Media")
set(OGRE_MEDIA_DIR_DBG "${OGRE_SOURCE_DIR}/Samples/Media")
if (WIN32)
  set(OGRE_PLUGIN_DIR_REL ".")
  set(OGRE_PLUGIN_DIR_DBG ".")
elseif (UNIX)
  set(OGRE_PLUGIN_DIR_REL "../lib")
  set(OGRE_PLUGIN_DIR_DBG "../../lib/OGRE")
endif ()
if (WIN32)
  # create resources.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/resources_d.cfg.in ${OGRE_BINARY_DIR}/bin/debug/resources.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${OGRE_BINARY_DIR}/bin/release/resources.cfg)
  # create plugins.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/plugins_d.cfg.in ${OGRE_BINARY_DIR}/bin/debug/plugins.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${OGRE_BINARY_DIR}/bin/release/plugins.cfg)
  # create media.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/media.cfg.in ${OGRE_BINARY_DIR}/bin/debug/media.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/media.cfg.in ${OGRE_BINARY_DIR}/bin/release/media.cfg)
  # create quake3settings.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/quake3settings_d.cfg.in ${OGRE_BINARY_DIR}/bin/debug/quake3settings.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/quake3settings.cfg.in ${OGRE_BINARY_DIR}/bin/release/quake3settings.cfg)
else() # other OS only need one cfg file
  string(TOLOWER "${CMAKE_BUILD_TYPE}" OGRE_BUILD_TYPE)
  if (OGRE_BUILD_TYPE STREQUAL "debug")
    set(OGRE_CFG_SUFFIX "_d")
  endif ()
  # create resources.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/resources${OGRE_CFG_SUFFIX}.cfg.in ${OGRE_BINARY_DIR}/bin/resources.cfg)
  # create plugins.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/plugins${OGRE_CFG_SUFFIX}.cfg.in ${OGRE_BINARY_DIR}/bin/plugins.cfg)
  # create media.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/media.cfg.in ${OGRE_BINARY_DIR}/bin/media.cfg)
  # create quake3settings.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/quake3settings${OGRE_CFG_SUFFIX}.cfg.in ${OGRE_BINARY_DIR}/bin/quake3settings.cfg)
endif ()

