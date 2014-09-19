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
	include/SampleBrowser.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/Sample.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SampleContext.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SamplePlugin.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SdkCameraMan.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SdkSample.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/SdkTrays.h
	${OGRE_SOURCE_DIR}/Samples/Common/include/OgreStaticPluginLoader.h
)

set(SOURCE_FILES src/SampleBrowser.cpp)
# Get the list of configured samples
get_property(OGRE_SAMPLES_LIST GLOBAL PROPERTY "OGRE_SAMPLES_LIST")

if (OGRE_BUILD_PLATFORM_APPLE_IOS)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/SampleBrowser.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")

  if(OGRE_BUILD_RENDERSYSTEM_GLES AND NOT OGRE_BUILD_RENDERSYSTEM_GLES2)
    list(REMOVE_ITEM OGRE_SAMPLES_LIST Sample_BSP Sample_Compositor Sample_CelShading Sample_CubeMapping Sample_Dot3Bump Sample_Fresnel Sample_Instancing Sample_Lighting Sample_ShaderSystem Sample_ShaderSystemTexturedFog Sample_ShaderSystemMultiLight Sample_VolumeTex Sample_Water)
  endif()

  if(NOT OGRE_BUILD_COMPONENT_TERRAIN AND NOT OGRE_SDK_BUILD)
    list(REMOVE_ITEM OGRE_SAMPLES_LIST Sample_Terrain)
    list(REMOVE_ITEM OGRE_SAMPLES_LIST Sample_EndlessWorld)
  endif()

  if(NOT OGRE_BUILD_COMPONENT_RTSHADERSYSTEM AND NOT OGRE_SDK_BUILD)
    list(REMOVE_ITEM OGRE_SAMPLES_LIST Sample_ShaderSystem)
    list(REMOVE_ITEM OGRE_SAMPLES_LIST Sample_ShaderSystemMultiLight)
    list(REMOVE_ITEM OGRE_SAMPLES_LIST Sample_ShaderSystemTexturedFog)
  endif()
endif ()

if (APPLE AND OGRE_BUILD_PLATFORM_APPLE_IOS)
  set(HEADER_FILES ${HEADER_FILES}
    include/SampleBrowser_iOS.h
  )
elseif (OGRE_BUILD_PLATFORM_NACL)
  set(HEADER_FILES ${HEADER_FILES}
    include/SampleBrowser_NaCl.h
  )
else ()
  set(HEADER_FILES ${HEADER_FILES}
    include/SampleBrowser_OSX.h
	${OGRE_SOURCE_DIR}/Samples/Common/misc/SampleBrowser_OSX.icns
  )
endif (APPLE AND OGRE_BUILD_PLATFORM_APPLE_IOS)

if(ANDROID)
  set(HEADER_FILES ${HEADER_FILES} include/SampleBrowser_Android.h)
endif(ANDROID)

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

  # Fix for static build with D3D9
  if (OGRE_BUILD_RENDERSYSTEM_D3D9 AND OGRE_STATIC)
    include_directories(${DirectX9_INCLUDE_DIR})
  endif() 
  
	ogre_add_component_include_dir(Paging)
	ogre_add_component_include_dir(Terrain)
	ogre_add_component_include_dir(Volume)
else()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)  
endif()

if(ANDROID)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "int x = 0;")
    ADD_LIBRARY(SampleBrowserDummy MODULE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
    target_link_libraries(SampleBrowserDummy ${SAMPLE_LIBRARIES})
    add_dependencies(SampleBrowserDummy ${OGRE_SAMPLES_LIST})

    if(APPLE OR WIN32)
      SET(ANDROID_EXECUTABLE "android")
      SET(NDK_BUILD_EXECUTABLE "ndk-build")
    else()
      SET(ANDROID_EXECUTABLE "$ENV{ANDROID_SDK}/tools/android")
      SET(NDK_BUILD_EXECUTABLE "$ENV{ANDROID_NDK}/ndk-build")
    endif()

    SET(ANT_EXECUTABLE "ant")
	if(${ANDROID_NATIVE_API_LEVEL} LESS 14)
		MATH(EXPR ANDROID_SDK_API_LEVEL "${ANDROID_NATIVE_API_LEVEL}+1")
	else()
		SET(ANDROID_SDK_API_LEVEL "${ANDROID_NATIVE_API_LEVEL}")
		SET(SCREEN_SIZE "|screenSize")
	endif()
	
	if(OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER)
		SET(GLES_OPTIMISER "-lglsl_optimizer -lmesa -lglcpp-library")
	endif()

    SET(ANDROID_TARGET "android-${ANDROID_SDK_API_LEVEL}")
    
    SET(NDKOUT "${CMAKE_BINARY_DIR}/SampleBrowserNDK")
    file(MAKE_DIRECTORY "${NDKOUT}")
    file(MAKE_DIRECTORY "${NDKOUT}/jni")
    file(MAKE_DIRECTORY "${NDKOUT}/assets")	
    file(MAKE_DIRECTORY "${NDKOUT}/res")
    file(WRITE "${NDKOUT}/default.properties" "target=${ANDROID_TARGET}")
    file(WRITE "${NDKOUT}/jni/Application.mk" "APP_ABI := ${ANDROID_ABI}\nAPP_STL := gnustl_static ")
    configure_file("${OGRE_TEMPLATES_DIR}/AndroidManifest.xml.in" "${NDKOUT}/AndroidManifest.xml" @ONLY)

	SET(ANDROID_MOD_NAME "OgreSampleBrowser")
	SET(JNI_SRC "LOCAL_PATH := @CMAKE_SOURCE_DIR@/Samples/Browser/src\n")
    SET(JNI_SRC "${JNI_SRC}\tLOCAL_SRC_FILES := SampleBrowser.cpp\n")
	
    if(NOT ANDROID_GLES_ONLY)
	 SET(SAMPLE_LDLIBS "LOCAL_LDLIBS += -lSample_CameraTrack -lSample_CelShading -lSample_Character -lSample_Compositor -lSample_Lighting -lSample_BezierPatch -lSample_CubeMapping -lSample_DynTex\n")
     SET(SAMPLE_LDLIBS "${SAMPLE_LDLIBS}\tLOCAL_LDLIBS += -lSample_FacialAnimation -lSample_Fresnel -lSample_Grass -lSample_ParticleFX -lSample_Lighting -lSample_MeshLod -lSample_SkyBox -lSample_SkyPlane -lSample_SkyDome -lSample_Smoke\n")
     SET(SAMPLE_LDLIBS "${SAMPLE_LDLIBS}\tLOCAL_LDLIBS += -lSample_Water -lSample_Ocean -lSample_NewInstancing -lSample_ShaderSystem -lSample_DeferredShading -lSample_EndlessWorld -lSample_VolumeCSG -lSample_VolumeTerrain -lSample_Terrain\n")
	 configure_file("${OGRE_TEMPLATES_DIR}/Android.mk.in" "${NDKOUT}/jni/Android.mk" @ONLY)
    else()
     SET(SAMPLE_LDLIBS "LOCAL_LDLIBS += -lSample_CameraTrack -lSample_Character -lSample_Compositor -lSample_Lighting -lSample_BezierPatch -lSample_DynTex\n")
	 SET(SAMPLE_LDLIBS "${SAMPLE_LDLIBS}\tLOCAL_LDLIBS += -lSample_FacialAnimation -lSample_Grass -lSample_ParticleFX -lSample_Lighting -lSample_SkyBox -lSample_SkyPlane -lSample_SkyDome -lSample_Smoke\n")
     configure_file("${OGRE_TEMPLATES_DIR}/AndroidGLES1.mk.in" "${NDKOUT}/jni/Android.mk" @ONLY)
    endif()

    configure_file("${OGRE_TEMPLATES_DIR}/Android_resources.cfg.in" "${NDKOUT}/assets/resources.cfg" @ONLY)
    configure_file("${OGRE_TEMPLATES_DIR}/samples.cfg.in" "${NDKOUT}/assets/samples.cfg" @ONLY)
    
    file(COPY "${CMAKE_SOURCE_DIR}/Samples/Media/models" DESTINATION "${NDKOUT}/assets")
    file(COPY "${CMAKE_SOURCE_DIR}/Samples/Media/particle" DESTINATION "${NDKOUT}/assets")
    file(COPY "${CMAKE_SOURCE_DIR}/Samples/Media/RTShaderLib" DESTINATION "${NDKOUT}/assets")
    file(COPY "${CMAKE_SOURCE_DIR}/Samples/Media/thumbnails" DESTINATION "${NDKOUT}/assets")
    file(COPY "${CMAKE_SOURCE_DIR}/Samples/Media/packs" DESTINATION "${NDKOUT}/assets")
    file(COPY "${CMAKE_SOURCE_DIR}/Samples/Media/materials" DESTINATION "${NDKOUT}/assets")
    
	file(COPY "${CMAKE_SOURCE_DIR}/SDK/Android/drawable-hdpi" DESTINATION "${NDKOUT}/res")
	file(COPY "${CMAKE_SOURCE_DIR}/SDK/Android/drawable-ldpi" DESTINATION "${NDKOUT}/res")
	file(COPY "${CMAKE_SOURCE_DIR}/SDK/Android/drawable-mdpi" DESTINATION "${NDKOUT}/res")
	file(COPY "${CMAKE_SOURCE_DIR}/SDK/Android/drawable-xhdpi" DESTINATION "${NDKOUT}/res")
    
	add_custom_command(
	                    TARGET SampleBrowserDummy
                        POST_BUILD
	                    COMMAND ${ANDROID_EXECUTABLE} update project --target ${ANDROID_TARGET} --path "${NDKOUT}"
	                    WORKING_DIRECTORY ${NDKOUT}
	                  )
	
	if(DEBUG)	 
	 	add_custom_command(
							TARGET SampleBrowserDummy
						    POST_BUILD
					        COMMAND ${NDK_BUILD_EXECUTABLE} all -j2 V=1 NDK_DEBUG=1
				            WORKING_DIRECTORY ${NDKOUT}
			              )
	else()
		add_custom_command(
							TARGET SampleBrowserDummy
						    POST_BUILD
					        COMMAND ${NDK_BUILD_EXECUTABLE} all -j2
				            WORKING_DIRECTORY ${NDKOUT}
			              )
	endif()
	                  
	add_custom_command(
	                    TARGET SampleBrowserDummy
                        POST_BUILD
	                    COMMAND ${ANT_EXECUTABLE} debug
	                    WORKING_DIRECTORY ${NDKOUT}
	                  )

else()

if (OGRE_BUILD_TESTS)
	add_definitions(-DSAMPLES_INCLUDE_PLAYPEN)
	include_directories(${OGRE_SOURCE_DIR}/Tests/PlayPen/include
	${OGRE_SOURCE_DIR}/Tests/VisualTests/PlayPen/include)
endif()

# Fix for static build with D3D9
if (OGRE_BUILD_RENDERSYSTEM_D3D9 AND OGRE_STATIC)
    link_directories(${DirectX9_LIBRARY_DIR})
endif()

ogre_add_executable(SampleBrowser WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

# Fix for static build with D3D9
if (OGRE_BUILD_RENDERSYSTEM_D3D9 AND OGRE_STATIC)
   target_link_libraries(SampleBrowser ${DirectX9_LIBRARIES})
endif()

target_link_libraries(SampleBrowser ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES} ${SAMPLE_LIBRARIES})

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
if (NOT APPLE)
	set_property(TARGET SampleBrowser APPEND PROPERTY DEBUG_POSTFIX "_d")
endif ()

# set install RPATH for Unix systems
if (UNIX AND OGRE_FULL_RPATH)
	set_property(TARGET SampleBrowser APPEND PROPERTY
		INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY})
	set_property(TARGET SampleBrowser PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

if (APPLE)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/SampleBrowser.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")

	# On OS X, create .app bundle
	set_property(TARGET SampleBrowser PROPERTY MACOSX_BUNDLE TRUE)
	set_property(TARGET SampleBrowser PROPERTY MACOSX_BUNDLE_ICON_FILE SampleBrowser_OSX.icns)
	set_property(TARGET SampleBrowser PROPERTY MACOSX_BUNDLE_INFO_PLIST ${OGRE_SOURCE_DIR}/Samples/Common/misc/Info.plist)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.ogre3d.\${PRODUCT_NAME:rfc1034identifier}")
    set_target_properties(SampleBrowser PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")

  if (OGRE_BUILD_PLATFORM_APPLE_IOS)
    set_target_properties(SampleBrowser PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
    set(OGRE_SAMPLE_CONTENTS_PATH ${OGRE_BINARY_DIR}/bin/$(CONFIGURATION)/SampleBrowser.app)
    add_custom_command(TARGET SampleBrowser POST_BUILD
      COMMAND ditto ${OGRE_SOURCE_DIR}/Samples/Common/misc/*.png ${OGRE_SAMPLE_CONTENTS_PATH}/
      COMMAND ditto ${OGRE_BINARY_DIR}/bin/*.cfg ${OGRE_SAMPLE_CONTENTS_PATH}/
    )
    if(OGRE_SDK_BUILD)
	    add_custom_command(TARGET SampleBrowser POST_BUILD
          COMMAND mkdir ARGS -p ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/
	      COMMAND ditto ${OGRE_BINARY_DIR}/Media ${OGRE_SAMPLE_CONTENTS_PATH}/Media
	    )
    else()
        add_custom_command(TARGET SampleBrowser POST_BUILD
          COMMAND mkdir ARGS -p ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/
          COMMAND ditto ${OGRE_SOURCE_DIR}/Samples/Media ${OGRE_SAMPLE_CONTENTS_PATH}/Media
        )
    endif(OGRE_SDK_BUILD)
  else()

    include_directories(
        ${OGRE_SOURCE_DIR}/RenderSystems/GL/include
        ${OGRE_SOURCE_DIR}/RenderSystems/GL/include/OSX
        )

	target_link_libraries(SampleBrowser ${OPENGL_LIBRARIES} ${CoreVideo_LIBRARIES})

   set_target_properties(SampleBrowser PROPERTIES
        LINK_FLAGS "-framework IOKit -framework Cocoa -framework Carbon -framework CoreVideo -framework ApplicationServices")

    # Add the path where the Ogre framework was found, if it has been found somewhere
    if(NOT ${OGRE_FRAMEWORK_PATH} STREQUAL "" AND OGRE_SDK_BUILD)
      set_target_properties(SampleBrowser PROPERTIES
        COMPILE_FLAGS "-F${OGRE_FRAMEWORK_PATH}"
        LINK_FLAGS "-F${OGRE_FRAMEWORK_PATH} -framework IOKit -framework Cocoa -framework Carbon -framework OpenGL -framework CoreVideo"
      )
    endif()
	# also, symlink frameworks so .app is standalone
	# NOTE: $(CONFIGURATION) is not resolvable at CMake run time, it's only 
	# valid at build time (hence parenthesis rather than braces)
	
    set(OGRE_OSX_BUILD_CONFIGURATION "$(PLATFORM_NAME)/$(CONFIGURATION)")
    set(OGRE_BUILT_FRAMEWORK "$(PLATFORM_NAME)/$(CONFIGURATION)")
	set (OGRE_SAMPLE_CONTENTS_PATH 
		${OGRE_BINARY_DIR}/bin/$(CONFIGURATION)/SampleBrowser.app/Contents)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Ogre.framework 
		${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		COMMAND ln ARGS -s -f ${Cg_LIBRARY_REL}
		${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		)
	# now cfg files
	if(OGRE_SDK_BUILD)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Resources
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/macosx/plugins.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/macosx/resources.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/macosx/samples.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/macosx/quakemap.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_SOURCE_DIR}/Media
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
        COMMAND ditto 
		${OGRE_SOURCE_DIR}/Samples/Common/misc/SampleBrowser_OSX.icns ${OGRE_SAMPLE_CONTENTS_PATH}/Resources
		)
	else()
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
		COMMAND ln ARGS -s -f ${OGRE_SOURCE_DIR}/Samples/Media
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
        COMMAND ditto 
		${OGRE_SOURCE_DIR}/Samples/Common/misc/SampleBrowser_OSX.icns ${OGRE_SAMPLE_CONTENTS_PATH}/Resources
		)
	endif()

	# now plugins
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins)

    set(FRAMEWORKS OgreOverlay
        OgrePaging
        OgreProperty
        OgreRTShaderSystem
        OgreTerrain
        OgreVolume
        Plugin_BSPSceneManager
        Plugin_CgProgramManager
        Plugin_OctreeSceneManager
        Plugin_OctreeZone
        Plugin_ParticleFX
        Plugin_PCZSceneManager
        RenderSystem_GL
        RenderSystem_GL3Plus
    )

    foreach(FWK ${FRAMEWORKS})
	add_custom_command(TARGET SampleBrowser POST_BUILD
        COMMAND ARGS "if [ -d ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/${FWK}.framework ]\; then ln -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/${FWK}.framework ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/${FWK}.framework\; fi"
		)
    endforeach(FWK)

	# Add links for the sample plugins
    foreach(SAMPLE_PLUGIN ${OGRE_SAMPLES_LIST})
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/${SAMPLE_PLUGIN}.dylib
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
    endforeach(SAMPLE_PLUGIN)

	if (OGRE_BUILD_TESTS)
	  add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/PlayPen.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	endif()
  endif()
endif (APPLE)

if (OGRE_INSTALL_SAMPLES)
	ogre_install_target(SampleBrowser "" FALSE)
	if (OGRE_INSTALL_PDB)
		# install debug pdb files
		install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/SampleBrowser_d.pdb
			DESTINATION bin${OGRE_DEBUG_PATH} CONFIGURATIONS Debug
			)
		install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_RELWDBG_PATH}/SampleBrowser.pdb
			DESTINATION bin${OGRE_RELWDBG_PATH} CONFIGURATIONS RelWithDebInfo
			)
	endif ()
endif ()
endif(ANDROID)

