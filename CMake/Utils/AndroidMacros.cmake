#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------


macro(add_static_libs LIB_DIR)
    foreach(LIB_NAME ${ARGN})
        SET(HEADERS "${HEADERS}# ${LIB_NAME}\n\tinclude $(CLEAR_VARS)\n\tLOCAL_MODULE    := ${LIB_NAME}\n\tLOCAL_SRC_FILES := ${LIB_DIR}/lib${LIB_NAME}.a\n\tinclude $(PREBUILT_STATIC_LIBRARY)\n\n")
        SET(SAMPLE_LDLIBS "${SAMPLE_LDLIBS} ${LIB_NAME}")
    endforeach(LIB_NAME ${ARGN})
    SET(SAMPLE_LDLIBS "${SAMPLE_LDLIBS}\n    LOCAL_STATIC_LIBRARIES\t+= ")
endmacro(add_static_libs)

macro(add_static_libs_from_paths)
    foreach(LIB ${ARGN})
        get_filename_component(LIB_NAME ${LIB} NAME_WE)
        string(SUBSTRING ${LIB_NAME} 3 -1 LIB_NAME) # strip lib prefix
        
        set(HEADERS "${HEADERS}# ${LIB_NAME}\n\tinclude $(CLEAR_VARS)\n\tLOCAL_MODULE    := ${LIB_NAME}\n\tLOCAL_SRC_FILES := ${LIB}\n\tinclude $(PREBUILT_STATIC_LIBRARY)\n\n")
        set(SAMPLE_LDLIBS "${SAMPLE_LDLIBS} ${LIB_NAME}")
    endforeach()
    set(SAMPLE_LDLIBS "${SAMPLE_LDLIBS}\n    LOCAL_STATIC_LIBRARIES\t+= ")
endmacro(add_static_libs_from_paths)

macro(create_android_proj ANDROID_PROJECT_TARGET)
    ##################################################################
    # Creates a basic android JNI project
    # Expects :
    #    - ANDROID_MOD_NAME    Name of the android module
    #    - JNI_PATH            Path to the jni directory containing .cpp files
    #    - JNI_SRC_FILES       A list of native .cpp file names to compile
    #    - PKG_NAME            The name of the output android package ex"Org.Ogre.OgreJNI"
    #    - NDKOUT              The directory for the Ndk project to be written to
    #    - HAS_CODE            Set this variable to "false" if no java code will be present 
    #                          (google android:hasCode for more info)
    #    - MAIN_ACTIVITY       Name of the main java activity ex "android.app.MainActivity" 
    ##################################################################    
    	
	if(OGRE_BUILD_RENDERSYSTEM_GLES2)
	    SET(DEPENDENCIES OgreMain RenderSystem_GLES2)
	else()
	    SET(DEPENDENCIES OgreMain RenderSystem_GLES)		
	endif()
	
	SET(DEPENDENCIES ${DEPENDENCIES} OgreTerrain OgreRTShaderSystem OgreMeshLodGenerator OgreOverlay OgrePaging OgreVolume Plugin_ParticleFX Plugin_OctreeSceneManager)
	add_dependencies(${ANDROID_PROJECT_TARGET} ${DEPENDENCIES})
	set(DEPEND_STATIC_LIBS "")	
	foreach(DEPENDENCY ${DEPENDENCIES})
	    set(DEPEND_STATIC_LIBS "${DEPENDENCY}Static" ${DEPEND_STATIC_LIBS})
	endforeach(DEPENDENCY ${DEPENDENCIES})
	add_static_libs("${OGRE_BINARY_DIR}/lib" ${DEPEND_STATIC_LIBS})
	if(OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER)
        add_static_libs("${OGRE_DEPENDENCIES_DIR}/lib/@ANDROID_NDK_ABI_NAME@"  "glsl_optimizer" "glcpp-library" "mesa")
	endif()

	if(OGRE_CONFIG_ENABLE_FREEIMAGE)
	    add_static_libs_from_paths(${FreeImage_LIBRARY_REL})
    endif()

    if(OGRE_USE_BOOST)
       set(OGRE_BOOST_INCLUDES ${Boost_INCLUDE_DIRS})       
	   add_static_libs_from_paths(${Boost_LIBRARIES})
    endif()

    add_static_libs_from_paths(${OIS_LIBRARY_REL} ${FREETYPE_LIBRARY_REL} ${ZZip_LIBRARY_REL})

    if(APPLE OR WIN32)
      SET(ANDROID_EXECUTABLE "android")
      SET(NDK_BUILD_EXECUTABLE "ndk-build")
    else()
      if(EXISTS $ENV{ANDROID_SDK})
        SET(ANDROID_EXECUTABLE "$ENV{ANDROID_SDK}/tools/android")
      else()
	    SET(ANDROID_EXECUTABLE "/opt/android-sdk/tools/android")
      endif()
      if(EXISTS $ENV{ANDROID_NDK})
        SET(NDK_BUILD_EXECUTABLE "$ENV{ANDROID_NDK}/ndk-build")
      else()
        SET(NDK_BUILD_EXECUTABLE "${ANDROID_NDK}/ndk-build")
      endif()
    endif()

	SET(ANT_EXECUTABLE "ant")
	
	if(${ANDROID_NATIVE_API_LEVEL} LESS 14)
		MATH(EXPR ANDROID_SDK_API_LEVEL "${ANDROID_NATIVE_API_LEVEL}+1")
	else()
		SET(ANDROID_SDK_API_LEVEL "${ANDROID_NATIVE_API_LEVEL}")
		SET(SCREEN_SIZE "|screenSize")
	endif()
	
    SET(ANDROID_TARGET "android-${ANDROID_SDK_API_LEVEL}")

    file(MAKE_DIRECTORY "${NDKOUT}")
    file(MAKE_DIRECTORY "${NDKOUT}/jni")
    file(MAKE_DIRECTORY "${NDKOUT}/assets")	
    file(MAKE_DIRECTORY "${NDKOUT}/res")	
	file(MAKE_DIRECTORY "${NDKOUT}/src")

    file(WRITE "${NDKOUT}/default.properties" "target=${ANDROID_TARGET}")
    file(WRITE "${NDKOUT}/jni/Application.mk" "APP_ABI := ${ANDROID_NDK_ABI_NAME}\nLOCAL_ARM_NEON := ${NEON}\nAPP_STL := gnustl_static\nNDK_TOOLCHAIN_VERSION := ${ANDROID_COMPILER_VERSION}")
    configure_file("${OGRE_TEMPLATES_DIR}/AndroidManifest.xml.in" "${NDKOUT}/AndroidManifest.xml" @ONLY)
	configure_file("${OGRE_TEMPLATES_DIR}/Android.mk.in" "${NDKOUT}/jni/Android.mk" @ONLY)
    
	add_custom_command(
	                    TARGET ${ANDROID_PROJECT_TARGET}
                        POST_BUILD
	                    COMMAND ${ANDROID_EXECUTABLE} update project  --target ${ANDROID_TARGET} --path "${NDKOUT}"
	                    WORKING_DIRECTORY ${NDKOUT}
	                  )
	
	if(DEBUG)	 
	 	add_custom_command(
							TARGET ${ANDROID_PROJECT_TARGET}
						    POST_BUILD
					        COMMAND ${NDK_BUILD_EXECUTABLE} all -j2 V=1 NDK_DEBUG=1
				            WORKING_DIRECTORY ${NDKOUT}
			              )
	else()
		add_custom_command(
							TARGET ${ANDROID_PROJECT_TARGET}
						    POST_BUILD
					        COMMAND ${NDK_BUILD_EXECUTABLE} all -j2 V=1
				            WORKING_DIRECTORY ${NDKOUT}
			              )
	endif()
                  
	add_custom_command(
	                    TARGET ${ANDROID_PROJECT_TARGET}
                        POST_BUILD
	                    COMMAND ${ANT_EXECUTABLE} debug
	                    WORKING_DIRECTORY ${NDKOUT}
	                  )
endmacro(create_android_proj)

