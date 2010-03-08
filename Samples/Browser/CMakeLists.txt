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
if (WIN32)
  list(APPEND SOURCE_FILES src/FileSystemLayerImpl_WIN32.cpp)
elseif (OGRE_BUILD_PLATFORM_IPHONE)
  list(APPEND SOURCE_FILES src/FileSystemLayerImpl_iPhone.mm)
elseif (APPLE)
  list(APPEND SOURCE_FILES src/FileSystemLayerImpl_OSX.cpp)
elseif (UNIX)
  list(APPEND SOURCE_FILES src/FileSystemLayerImpl_Unix.cpp)
else ()
  list(APPEND SOURCE_FILES src/FileSystemLayerImpl_Default.cpp)
endif ()

if (SYMBIAN)
  set(SOURCE_FILES ${SOURCE_FILES}
  Symbian/src/OgreSamplesBrowserApp.cpp
  Symbian/src/OgreSamplesBrowserAppUi.cpp
  Symbian/src/OgreSamplesBrowserContainer.cpp
  Symbian/src/OgreSamplesBrowserDocument.cpp       
  )

  set(HEADER_FILES ${HEADER_FILES}
  Symbian/inc/OgreSamplesBrowser.hrh
  Symbian/inc/OgreSamplesBrowser.rls
  Symbian/inc/OgreSamplesBrowserApp.h
  Symbian/inc/OgreSamplesBrowserAppUi.h
  Symbian/inc/OgreSamplesBrowserContainer.h
  Symbian/inc/OgreSamplesBrowserDocument.h
  )

  set(RESOURCE_FILES ${RESOURCE_FILES}
  Symbian/data/OgreSamplesBrowser.rss
  Symbian/data/OgreSamplesBrowser_reg.rss
  Symbian/group/icons_scalable_dc.mk
  )


  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Symbian/inc
  )



	

endif (SYMBIAN)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/SampleBrowser.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")

  set (SAMPLE_LIBRARIES
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_BezierPatch.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_BSP.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CameraTrack.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CelShading.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Character.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CubeMapping.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Dot3Bump.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_DynTex.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_FacialAnimation.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Fresnel.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Grass.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Lighting.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Ocean.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_ParticleFX.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_ShaderSystem.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Shadows.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SkeletalAnimation.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SkyBox.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SkyDome.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SkyPlane.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Smoke.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_SphereMapping.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Terrain.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_TextureFX.a
       ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Transparency.a
  )
  if(OGRE_BUILD_RENDERSYSTEM_GLES AND NOT OGRE_BUILD_RENDERSYSTEM_GLES2)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_BezierPatch.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_BSP.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CelShading.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_CubeMapping.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Dot3Bump.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_DynTex.a)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Fresnel.a)
  endif()

  if(NOT OGRE_BUILD_COMPONENT_TERRAIN)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_Terrain.a)
  endif()

  if(NOT OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
    list(REMOVE_ITEM SAMPLE_LIBRARIES ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/Sample_ShaderSystem.a)
  endif()

endif ()

set (HEADER_FILES
	include/SampleBrowser.h
	include/FileSystemLayerImpl.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/Sample.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SampleContext.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SamplePlugin.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SdkCameraMan.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SdkSample.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SdkTrays.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/OgreStaticPluginLoader.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/FileSystemLayer.h
)

if (OGRE_STATIC)
  # Find all samples headers, and copy them to a single include folder
  # We do this instead of including every sample in the include_directories directive, 
  # because this creates a list that is too long for the MSVC command line sometimes.
  file (GLOB SAMPLES_HEADERS ${OGRE_SOURCE_DIR}/Samples/*/include/*.h)
  foreach(SAMPLE_HEADER_FULLPATH ${SAMPLES_HEADERS})
	get_filename_component(SAMPLE_HEADER_NAME ${SAMPLE_HEADER_FULLPATH} NAME)
	configure_file(${SAMPLE_HEADER_FULLPATH} ${OGRE_BINARY_DIR}/Samples/include/${SAMPLE_HEADER_NAME} COPYONLY)
  endforeach(SAMPLE_HEADER_FULLPATH)
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
	${OGRE_BINARY_DIR}/Samples/include)
	
	ogre_add_component_include_dir(Paging)
	ogre_add_component_include_dir(Terrain)
else()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)  
endif()

if (OGRE_BUILD_TESTS)
	add_definitions(-DSAMPLES_INCLUDE_PLAYPEN)
	include_directories(${OGRE_SOURCE_DIR}/Tests/PlayPen/include)
endif()
add_executable(SampleBrowser WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(SampleBrowser ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES} ${SAMPLE_LIBRARIES})

# Get the list of configured samples
get_property(OGRE_SAMPLES_LIST GLOBAL PROPERTY "OGRE_SAMPLES_LIST")
add_dependencies(SampleBrowser ${OGRE_SAMPLES_LIST})

if (OGRE_STATIC)
	# Link to samples too
	target_link_libraries(SampleBrowser ${OGRE_SAMPLES_LIST})
endif()

if (SAMPLE_DEPENDENCIES)
  add_dependencies(SampleBrowser ${SAMPLE_DEPENDENCIES})
endif ()


# configure SampleBrowser build

ogre_config_common(SampleBrowser)

# append _d for debug builds
if (NOT APPLE)
	set_property(TARGET SampleBrowser APPEND PROPERTY DEBUG_POSTFIX "_d")
endif ()

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
    set (OGRE_SAMPLE_CONTENTS_PATH ${OGRE_BINARY_DIR}/bin/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/SampleBrowser.app)
    add_custom_command(TARGET SampleBrowser POST_BUILD
      COMMAND cp ARGS ${OGRE_BINARY_DIR}/bin/*.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_SOURCE_DIR}/Samples/Common/misc/Icon.png ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_SOURCE_DIR}/Samples/Common/misc/Default.png ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_BINARY_DIR}/bin/plugins.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_BINARY_DIR}/bin/resources.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_BINARY_DIR}/bin/samples.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_BINARY_DIR}/bin/quakemap.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
    )
    if(OGRE_SDK_BUILD)
	    add_custom_command(TARGET SampleBrowser POST_BUILD
          COMMAND mkdir ARGS -p ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/
	      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_BINARY_DIR}/media ${OGRE_SAMPLE_CONTENTS_PATH}/
	      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/lib*.a ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/
	    )
	elseif(OGRE_INSTALL_SAMPLES_SOURCE)
        add_custom_command(TARGET SampleBrowser POST_BUILD
      	  COMMAND mkdir ARGS -p ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/
	      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_SOURCE_DIR}/Samples/Media ${OGRE_SAMPLE_CONTENTS_PATH}/
	      COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/lib*.a ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/
        )
    else()
        add_custom_command(TARGET SampleBrowser POST_BUILD
          COMMAND mkdir ARGS -p ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/
          COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${OGRE_SOURCE_DIR}/Samples/Media ${OGRE_SAMPLE_CONTENTS_PATH}/
        )
    endif(OGRE_SDK_BUILD)
  else()

    # Add the path where the Ogre framework was found, if it has been found somewhere
    if(NOT ${OGRE_FRAMEWORK_PATH} STREQUAL "" AND OGRE_SDK_BUILD)
      set_target_properties(SampleBrowser PROPERTIES
        COMPILE_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
        LINK_FLAGS "-F${OGRE_FRAMEWORK_PATH} -framework IOKit -framework Cocoa -framework Carbon"
      )
    endif()
	# also, symlink frameworks so .app is standalone
	# NOTE: $(CONFIGURATION) is not resolvable at CMake run time, it's only 
	# valid at build time (hence parenthesis rather than braces)
	
	if(OGRE_SDK_BUILD)
	  set(OGRE_OSX_BUILD_CONFIGURATION "")
	  set(OGRE_BUILT_FRAMEWORK "Release/")
	else()
	  set(OGRE_OSX_BUILD_CONFIGURATION "$(CONFIGURATION)/")
	  set(OGRE_BUILT_FRAMEWORK "$(CONFIGURATION)/")
	endif()
	set (OGRE_SAMPLE_CONTENTS_PATH 
		${OGRE_BINARY_DIR}/bin/$(CONFIGURATION)/SampleBrowser.app/Contents)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}Ogre.framework 
		${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		COMMAND ln ARGS -s -f ${OGRE_SOURCE_DIR}/Dependencies/Cg.framework 
		${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		)
	# now cfg files
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Resources
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/plugins.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/resources.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/samples.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/quakemap.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		)

	# now plugins
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Components)
	if (OGRE_BUILD_RENDERSYSTEM_GL OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}RenderSystem_GL.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif ()
	if (OGRE_BUILD_PLUGIN_BSP OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_BSPSceneManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_CG OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_CgProgramManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_OCTREE OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_OctreeSceneManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_PCZ OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_PCZSceneManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/    
			)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_OctreeZone.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_PFX OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_ParticleFX.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	
	# Components
	if (OGRE_BUILD_COMPONENT_PAGING OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgrePaging.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Components/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_PROPERTY OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgreProperty.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Components/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgreRTShaderSystem.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Components/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_TERRAIN OR OGRE_SDK_BUILD)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgreTerrain.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Components/
			)
	endif()

	# Add links for the sample plugins
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_*.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
  endif()
endif (APPLE)

if (OGRE_INSTALL_SAMPLES)
	ogre_install_target(SampleBrowser "" FALSE)
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

