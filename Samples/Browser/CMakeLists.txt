#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Browser demo build

set (HEADER_FILES
	${PROJECT_SOURCE_DIR}/Samples/Browser/include/SampleBrowser.h
	${PROJECT_SOURCE_DIR}/Samples/Common/include/Sample.h
	${PROJECT_SOURCE_DIR}/Samples/Common/include/SampleContext.h
	${PROJECT_SOURCE_DIR}/Samples/Common/include/SamplePlugin.h
	${PROJECT_SOURCE_DIR}/Samples/Common/include/SdkSample.h
)

# Get the list of configured samples
get_property(OGRE_SAMPLES_LIST GLOBAL PROPERTY "OGRE_SAMPLES_LIST")

if (APPLE AND APPLE_IOS)
  set(HEADER_FILES ${HEADER_FILES}
    ${PROJECT_SOURCE_DIR}/Samples/Browser/include/SampleBrowser_iOS.h
  )
elseif (APPLE)
  set(HEADER_FILES ${HEADER_FILES}
	${PROJECT_SOURCE_DIR}/Samples/Common/misc/SampleBrowser_OSX.icns
  )
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)  

if (WINDOWS_STORE OR WINDOWS_PHONE)

    # Inspired by http://cmakems.codeplex.com/SourceControl/latest#Tests/VSWinStorePhone/CMakeLists.txt

    # Generate manifest.
    set(EXE_NAME SampleBrowser)
    set(SHORT_NAME ${EXE_NAME})
    set(PACKAGE_GUID "ef31484f-4f23-4b9f-8748-af5482696ae5")
    set(APP_MANIFEST_NAME Package.appxmanifest)
    configure_file(
        ${OGRE_TEMPLATES_DIR}/Package_vc1900.WindowsStore.appxmanifest.in
        ${CMAKE_CURRENT_BINARY_DIR}/${APP_MANIFEST_NAME}
        @ONLY)

    # Make symlink to Media folder
    if( NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/Media")
        # Get OS dependent path to use in `execute_process`
        file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/Media" link)
        file(TO_NATIVE_PATH "${PROJECT_SOURCE_DIR}/Samples/Media" target)
        execute_process( COMMAND cmd.exe /c mklink /J ${link} ${target})
    endif()

    # Copy other resources
    file(COPY
	    ${SDL2_BINARY}

        ${PROJECT_BINARY_DIR}/inst/bin/resources.cfg
        ${PROJECT_BINARY_DIR}/inst/bin/plugins.cfg
        ${PROJECT_BINARY_DIR}/inst/bin/samples.cfg

        ${PROJECT_SOURCE_DIR}/Samples/Common/misc.winrt/Logo.png
        ${PROJECT_SOURCE_DIR}/Samples/Common/misc.winrt/SmallLogo.png
        ${PROJECT_SOURCE_DIR}/Samples/Common/misc.winrt/StoreLogo.png
        ${PROJECT_SOURCE_DIR}/Samples/Common/misc.winrt/SplashScreen.png

        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Mark resources as deployment content, so that it would be packaged
    set(CONTENT_FILES ${CONTENT_FILES}
        ${CMAKE_CURRENT_BINARY_DIR}/${APP_MANIFEST_NAME}
	${CMAKE_CURRENT_BINARY_DIR}/SDL2.dll
        ${CMAKE_CURRENT_BINARY_DIR}/Media
        ${CMAKE_CURRENT_BINARY_DIR}/Logo.png
        ${CMAKE_CURRENT_BINARY_DIR}/SmallLogo.png
        ${CMAKE_CURRENT_BINARY_DIR}/StoreLogo.png
        ${CMAKE_CURRENT_BINARY_DIR}/SplashScreen.png
        ${CMAKE_CURRENT_BINARY_DIR}/resources.cfg
        ${CMAKE_CURRENT_BINARY_DIR}/plugins.cfg
        ${CMAKE_CURRENT_BINARY_DIR}/samples.cfg
    )
    set_property(SOURCE ${CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)

    set(RESOURCE_FILES ${CONTENT_FILES})
    source_group("Resource Files" FILES ${RESOURCE_FILES})

endif()

if (OGRE_BUILD_TESTS)
	add_definitions(-DSAMPLES_INCLUDE_PLAYPEN)
	include_directories(${PROJECT_SOURCE_DIR}/Tests/PlayPen/include
	${PROJECT_SOURCE_DIR}/Tests/VisualTests/PlayPen/include)
endif()

# Fix for static build with MinGW
if (OGRE_BUILD_RENDERSYSTEM_D3D9 AND OGRE_STATIC)
    link_directories(${DirectX9_LIBRARY_DIR})
endif()

if (ANDROID)
    include(AndroidMacros)

    add_ndk_cpufeatures_library()
    add_ndk_native_app_glue_library()

    include_directories(${PROJECT_SOURCE_DIR}/RenderSystems/GLSupport/include)
    set(SOURCE_FILES
      ${PROJECT_SOURCE_DIR}/Samples/Browser/src/gestureDetector.cpp
      ${PROJECT_SOURCE_DIR}/Samples/Browser/src/android_main.cpp)

    add_library(SampleBrowser SHARED ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
else()
    set(SOURCE_FILES ${PROJECT_SOURCE_DIR}/Samples/Browser/src/main.cpp)
    add_executable(SampleBrowser WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
endif()

# Fix for static build with MinGW
if (OGRE_BUILD_RENDERSYSTEM_D3D9 AND OGRE_STATIC)
   target_link_libraries(SampleBrowser ${DirectX9_LIBRARIES})
endif()

target_link_libraries(SampleBrowser OgreMain ${OGRE_PLUGIN_LIBRARIES} ${SAMPLE_LIBRARIES} OgreBites)

if(ANDROID)
    target_link_libraries(SampleBrowser native_app_glue cpufeatures android c m dl z log EGL GLESv2 "-u ANativeActivity_onCreate")
    set_target_properties(SampleBrowser PROPERTIES LINK_FLAGS -s) # strip symbols
endif()

# Add samples as dependencies
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
if (WIN32 AND NOT (WINDOWS_STORE OR WINDOWS_PHONE))
	set_property(TARGET SampleBrowser APPEND PROPERTY DEBUG_POSTFIX "_d")
endif ()

if (WINDOWS_STORE OR WINDOWS_PHONE)
	set_target_properties(SampleBrowser PROPERTIES VS_WINRT_COMPONENT "true")
endif()

if (APPLE)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")

	# On OS X, create .app bundle
	set_property(TARGET SampleBrowser PROPERTY MACOSX_BUNDLE TRUE)
	set_property(TARGET SampleBrowser PROPERTY MACOSX_BUNDLE_ICON_FILE SampleBrowser_OSX.icns)
	set_property(TARGET SampleBrowser PROPERTY MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/Samples/Common/misc/Info.plist)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.ogre3d.\${PRODUCT_NAME:rfc1034identifier}")
    set_target_properties(SampleBrowser PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")

  if (APPLE_IOS)
    set_target_properties(SampleBrowser PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
    set(OGRE_SAMPLE_CONTENTS_PATH ${PROJECT_BINARY_DIR}/bin/$(CONFIGURATION)/SampleBrowser.app)
    add_custom_command(TARGET SampleBrowser POST_BUILD
      COMMAND ditto ${PROJECT_SOURCE_DIR}/Samples/Common/misc/*.png ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND ditto ${PROJECT_BINARY_DIR}/bin/*.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
    )
    add_custom_command(TARGET SampleBrowser POST_BUILD
      COMMAND mkdir ARGS -p ${PROJECT_BINARY_DIR}/lib/$(CONFIGURATION)/
      COMMAND ditto ${PROJECT_SOURCE_DIR}/Media ${OGRE_SAMPLE_CONTENTS_PATH}/Media
      COMMAND ditto ${PROJECT_SOURCE_DIR}/Samples/Media ${OGRE_SAMPLE_CONTENTS_PATH}/Media
    )
  else()

    set_target_properties(SampleBrowser
      PROPERTIES BUILD_WITH_INSTALL_RPATH 1
      INSTALL_RPATH "@executable_path/../Frameworks;${OGREDEPS_PATH}/lib"
    )

	target_link_libraries(SampleBrowser ${OPENGL_gl_LIBRARY} ${CoreVideo_LIBRARIES})
	# also, symlink frameworks so .app is standalone
	# NOTE: $(CONFIGURATION) is not resolvable at CMake run time, it's only 
	# valid at build time (hence parenthesis rather than braces)
  # PLATFORM_NAME is set by us in ConfigTargets
	
    set(OGRE_OSX_BUILD_CONFIGURATION "${PLATFORM_NAME}/$(CONFIGURATION)")
	set (OGRE_SAMPLE_CONTENTS_PATH 
		${PROJECT_BINARY_DIR}/bin/$(CONFIGURATION)/SampleBrowser.app/Contents)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		COMMAND ln ARGS -s -f ${Cg_LIBRARY_REL}                                             ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		)
	# now cfg files
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Resources
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/bin/plugins.cfg 		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/bin/resources.cfg 		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/bin/samples.cfg   		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${PROJECT_SOURCE_DIR}/Samples/Media     		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ditto ${PROJECT_SOURCE_DIR}/Samples/Common/misc/SampleBrowser_OSX.icns ${OGRE_SAMPLE_CONTENTS_PATH}/Resources
		)

	# now plugins
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins)

    set(FRAMEWORKS OgreOverlay
		    OgreBites
        OgreBullet
        OgrePaging
        OgreProperty
        OgreRTShaderSystem
        OgreTerrain
        OgreVolume
        OgreMeshLodGenerator
        Plugin_BSPSceneManager
        Plugin_CgProgramManager
        Plugin_OctreeSceneManager
        Plugin_OctreeZone
        Plugin_ParticleFX
        Plugin_PCZSceneManager
        Plugin_DotScene
        Codec_STBI
        Codec_FreeImage
        RenderSystem_GL
        RenderSystem_GL3Plus
        RenderSystem_Metal
    )

    foreach(FWK ${FRAMEWORKS})
      if(TARGET ${FWK})
        add_custom_command(TARGET SampleBrowser POST_BUILD
          COMMAND ln -s -f
          ${PROJECT_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/${FWK}.framework
          ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/${FWK}.framework)
      endif()
    endforeach(FWK)

	# Add links for the sample plugins
    foreach(SAMPLE_PLUGIN ${OGRE_SAMPLES_LIST})
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/${SAMPLE_PLUGIN}.dylib
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
    endforeach(SAMPLE_PLUGIN)

	if (OGRE_BUILD_TESTS)
	  add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/PlayPen.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	endif()
  endif()

endif (APPLE)

if (ANDROID)
    SET(ANDROID_MOD_NAME "OgreSampleBrowser")
    SET(NDKOUT "${CMAKE_BINARY_DIR}/SampleBrowserNDK")
    SET(PKG_NAME "org.ogre.browser")
    SET(HAS_CODE "false")
    SET(MAIN_ACTIVITY "android.app.NativeActivity")
    SET(HEADERS "")
    SET(SAMPLE_LDLIBS "")

    copy_assets_to_android_proj()

    file(MAKE_DIRECTORY "${NDKOUT}/app/src/main/jniLibs/${ANDROID_NDK_ABI_NAME}")
    set_target_properties(SampleBrowser PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${NDKOUT}/app/src/main/jniLibs/${ANDROID_NDK_ABI_NAME}")
    set_target_properties(SampleBrowser PROPERTIES OUTPUT_NAME "OgreSampleBrowser")

    create_android_proj(SampleBrowser)
    # this whole part should be simplified more
endif(ANDROID)

if (OGRE_INSTALL_SAMPLES)
	ogre_install_target(SampleBrowser "" FALSE)
  install(FILES ${PROJECT_BINARY_DIR}/inst/bin/samples.cfg DESTINATION "${OGRE_CFG_INSTALL_PATH}")

	if (OGRE_INSTALL_PDB)
		# install debug pdb files
		install(FILES $<TARGET_PDB_FILE:SampleBrowser>
			DESTINATION bin${OGRE_DEBUG_PATH} CONFIGURATIONS Debug
			)
		install(FILES $<TARGET_PDB_FILE:SampleBrowser>
			DESTINATION bin${OGRE_RELWDBG_PATH} CONFIGURATIONS RelWithDebInfo
			)
	endif ()
endif ()
