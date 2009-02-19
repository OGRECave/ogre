# Configure Samples build


if (OGRE_BUILD_SAMPLES AND NOT OIS_FOUND)
  message(STATUS "Could not find dependency: OIS")
  message(STATUS "Skipping samples build")
  set(OGRE_BUILD_SAMPLES 0)
endif()

if (OGRE_BUILD_SAMPLES)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)
  
  add_subdirectory(BezierPatch)
  add_subdirectory(BSP)
  add_subdirectory(CameraTrack)
  add_subdirectory(CelShading)
  add_subdirectory(CubeMapping)
  add_subdirectory(DeferredShading)
  add_subdirectory(Dot3Bump)
  add_subdirectory(DynTex)
  add_subdirectory(EnvMapping)
  add_subdirectory(Fresnel)
  add_subdirectory(Grass)
  add_subdirectory(Isosurf)
  add_subdirectory(Lighting)
  add_subdirectory(ParticleFX)
  add_subdirectory(ParticleGS)
  add_subdirectory(PCZTestApp)
  add_subdirectory(RenderToTexture)
  add_subdirectory(SkeletalAnimation)
  add_subdirectory(SkyBox)
  add_subdirectory(SkyDome)
  add_subdirectory(SkyPlane)
  add_subdirectory(Smoke)
  add_subdirectory(Terrain)
  add_subdirectory(TextureFX)
  add_subdirectory(Transpacency)
  add_subdirectory(VolumeTex)
  add_subdirectory(Water)

  if (NOT CEGUI_FOUND)
    message(STATUS "Could not find dependency: CEGUI")
    message(STATUS "Skipping GUI samples build")
  elseif (NOT OGRE_BUILD_CEGUIRENDERER)
    message(STATUS "CEGUI renderer was not built")
    message(STATUS "Skipping GUI samples build")
  else ()
    add_subdirectory(Compositor)
    add_subdirectory(FacialAnimation)
    add_subdirectory(Gui)
    add_subdirectory(Instancing)
    add_subdirectory(OceanDemo)
    add_subdirectory(Shadows)
  endif ()
endif ()


# Install sample sources
if (OGRE_INSTALL_SAMPLES_SOURCE)
  if (WIN32)
    set(OGRE_SAMPLES_DIR Samples)
  elseif (UNIX)
    set(OGRE_SAMPLES_DIR share/OGRE/samples)
  elseif (APPLE)
    # TODO
  endif ()
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${OGRE_SAMPLES_DIR}
    REGEX "^CMakeLists.txt$" EXCLUDE
    PATTERN "Makefile.am" EXCLUDE
    PATTERN "Media" EXCLUDE
    PATTERN "bin" EXCLUDE
    PATTERN "CEGUIRenderer" EXCLUDE
    PATTERN "misc" EXCLUDE
    PATTERN "setup" EXCLUDE
    PATTERN ".svn" EXCLUDE
	PATTERN "obj" EXCLUDE
	PATTERN "scripts" EXCLUDE
  )
  # install a new CMakeLists.txt file to allow building of samples
  configure_file(${OGRE_TEMPLATES_DIR}/SampleLists.cmake ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt DESTINATION ${OGRE_SAMPLES_DIR})
endif ()
