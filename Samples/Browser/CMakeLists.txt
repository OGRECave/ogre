#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Browser demo build

set(SOURCE_FILES src/SampleBrowser.cpp)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/SampleBrowser.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")

  set (SAMPLE_LIBRARIES
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_BezierPatch.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_BSP.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CameraTrack.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CelShading.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CubeMapping.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Dot3Bump.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_DynTex.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_FacialAnimation.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Fresnel.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Grass.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Lighting.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_ParticleFX.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_ShaderSystem.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SkeletalAnimation.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SkyBox.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SkyDome.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SkyPlane.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Smoke.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SphereMapping.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Terrain.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_TextureFX.a
       ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Transparency.a
  )
  if(OGRE_BUILD_RENDERSYSTEM_GLES)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_BezierPatch.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CelShading.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CubeMapping.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Dot3Bump.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_DynTex.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Fresnel.a)
  endif()

  if(NOT OGRE_BUILD_COMPONENT_TERRAIN)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Terrain.a)
  endif()

  if(NOT OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_ShaderSystem.a)
  endif()

endif ()

set (HEADER_FILES
	include/SampleBrowser.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/Sample.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SampleContext.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SamplePlugin.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SdkCameraMan.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SdkSample.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SdkTrays.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/OgreStaticPluginLoader.h
)

if (OGRE_STATIC)
  # Find all samples headers, and copy them to a single include folder
  # We do this instead of including every sample in the include_directories directive, 
  # because this creates a list that is too long for the MSVC command line sometimes.
  file (GLOB SAMPLES_HEADERS ${CMAKE_SOURCE_DIR}/Samples/*/include/*.h)
  foreach(SAMPLE_HEADER_FULLPATH ${SAMPLES_HEADERS})
	get_filename_component(SAMPLE_HEADER_NAME ${SAMPLE_HEADER_FULLPATH} NAME)
	configure_file(${SAMPLE_HEADER_FULLPATH} ${CMAKE_BINARY_DIR}/Samples/include/${SAMPLE_HEADER_NAME} COPYONLY)
  endforeach(SAMPLE_HEADER_FULLPATH)
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Components/Paging/include
    ${CMAKE_SOURCE_DIR}/Components/Terrain/include
	${CMAKE_BINARY_DIR}/Samples/include)
else()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)  
endif()

add_executable(SampleBrowser WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(SampleBrowser ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES} ${SAMPLE_LIBRARIES})

if (SAMPLE_DEPENDENCIES)
  add_dependencies(SampleBrowser ${SAMPLE_DEPENDENCIES})
endif ()


# configure SampleBrowser build

ogre_config_common(SampleBrowser)

# set install RPATH for Unix systems
if (UNIX AND OGRE_FULL_RPATH)
	set_property(TARGET SampleBrowser APPEND PROPERTY
		INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
	set_property(TARGET SampleBrowser PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

if (APPLE)
	# On OS X, create .app bundle
	set_property(TARGET SampleBrowser PROPERTY MACOSX_BUNDLE TRUE)

  if (OGRE_BUILD_PLATFORM_IPHONE)
    set (OGRE_SAMPLE_CONTENTS_PATH ${CMAKE_BINARY_DIR}/bin/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/SampleBrowser.app)
    add_custom_command(TARGET SampleBrowser POST_BUILD
      COMMAND cp ARGS ${CMAKE_BINARY_DIR}/bin/*.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_SOURCE_DIR}/Samples/Media ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_SOURCE_DIR}/Samples/Common/misc/Icon.png ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_SOURCE_DIR}/Samples/Common/misc/Default.png ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/bin/plugins.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/bin/resources.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/bin/media.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/bin/samples.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/bin/quakemap.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
    )
  else()

	# also, symlink frameworks so .app is standalone
	# NOTE: $(CONFIGURATION) is not resolvable at CMake run time, it's only 
	# valid at build time (hence parenthesis rather than braces)
	set (OGRE_SAMPLE_CONTENTS_PATH 
		${CMAKE_BINARY_DIR}/bin/$(CONFIGURATION)/SampleBrowser.app/Contents)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Ogre.framework 
		${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		COMMAND ln ARGS -s -f ${CMAKE_SOURCE_DIR}/Dependencies/Cg.framework 
		${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		)
	# now cfg files
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Resources
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/plugins.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/resources.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/media.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/samples.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/quakemap.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		)

	# now plugins
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins)
	if (OGRE_BUILD_RENDERSYSTEM_GL)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/RenderSystem_GL.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif ()
	if (OGRE_BUILD_PLUGIN_BSP)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_BSPSceneManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_CG)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_CgProgramManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_OCTREE)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_OctreeSceneManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_PCZ)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_PCZSceneManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/    
			)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_OctreeZone.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_PFX)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_ParticleFX.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()

	# Add links for the sample plugins
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_*.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
  endif()
endif (APPLE)

if (OGRE_INSTALL_SAMPLES)
	ogre_install_target(SampleBrowser "")
	if (OGRE_INSTALL_PDB)
		# install debug pdb files
		install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/SampleBrowser.pdb
			DESTINATION bin${OGRE_DEBUG_PATH} CONFIGURATIONS Debug
			)
		install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_RELWDBG_PATH}/SampleBrowser.pdb
			DESTINATION bin${OGRE_RELWDBG_PATH} CONFIGURATIONS RelWithDebInfo
			)
	endif ()
endif ()	

