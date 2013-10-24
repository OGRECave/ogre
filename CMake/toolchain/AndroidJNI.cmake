#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

if(ANDROID)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummyJNI.cpp "int x = 23;")
    ADD_LIBRARY(OgreJNIDummy MODULE ${CMAKE_CURRENT_BINARY_DIR}/dummyJNI.cpp)
	
	if(OGRE_BUILD_RENDERSYSTEM_GLES2)
		add_dependencies(OgreJNIDummy OgreMain RenderSystem_GLES2)
	else()		
		add_dependencies(OgreJNIDummy OgreMain RenderSystem_GLES)
	endif()
	
	add_dependencies(OgreJNIDummy OgreTerrain OgreRTShaderSystem OgreOverlay OgrePaging OgreVolume Plugin_ParticleFX Plugin_OctreeSceneManager)	

    if(APPLE OR WIN32)
      SET(ANDROID_EXECUTABLE "android")
      SET(NDK_BUILD_EXECUTABLE "ndk-build")
    else()
      SET(ANDROID_EXECUTABLE "$ENV{ANDROID_SDK}/tools/android")
      SET(NDK_BUILD_EXECUTABLE "$ENV{ANDROID_NDK}/ndk-build")
    endif()

	SET(ANDROID_MOD_NAME "OgreJNI")
	SET(JNI_SRC "LOCAL_PATH := @CMAKE_SOURCE_DIR@/OgreMain/src/Android/JNI\n")
    SET(JNI_SRC "${JNI_SRC}\tLOCAL_SRC_FILES := OgreActivityJNI.cpp\n")
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
    
    SET(NDKOUT "${CMAKE_BINARY_DIR}/OgreJNI")
    file(MAKE_DIRECTORY "${NDKOUT}")
    file(MAKE_DIRECTORY "${NDKOUT}/jni")
    file(MAKE_DIRECTORY "${NDKOUT}/assets")	
    file(MAKE_DIRECTORY "${NDKOUT}/res")	
	file(MAKE_DIRECTORY "${NDKOUT}/src")
    file(MAKE_DIRECTORY "${NDKOUT}/src/org")
    file(MAKE_DIRECTORY "${NDKOUT}/src/org/ogre3d")
	file(MAKE_DIRECTORY "${NDKOUT}/src/org/ogre3d/android")	
	file(COPY "@CMAKE_SOURCE_DIR@/OgreMain/src/Android/JNI/OgreActivityJNI.java" DESTINATION "${NDKOUT}/src/org/ogre3d/android")
	file(COPY "@CMAKE_SOURCE_DIR@/OgreMain/src/Android/JNI/MainActivity.java" DESTINATION "${NDKOUT}/src/org/ogre3d/android")
		
    file(WRITE "${NDKOUT}/default.properties" "target=${ANDROID_TARGET}")
    file(WRITE "${NDKOUT}/jni/Application.mk" "APP_ABI := ${ANDROID_ABI}\nAPP_STL := gnustl_static ")
    configure_file("${OGRE_TEMPLATES_DIR}/AndroidManifest_JNI.xml.in" "${NDKOUT}/AndroidManifest.xml" @ONLY)

    if(NOT ANDROID_GLES_ONLY)
	  configure_file("${OGRE_TEMPLATES_DIR}/Android.mk.in" "${NDKOUT}/jni/Android.mk" @ONLY)
    else()
      configure_file("${OGRE_TEMPLATES_DIR}/AndroidGLES1.mk.in" "${NDKOUT}/jni/Android.mk" @ONLY)
    endif()
    
	add_custom_command(
	                    TARGET OgreJNIDummy
                        POST_BUILD
	                    COMMAND ${ANDROID_EXECUTABLE} update project --target ${ANDROID_TARGET} --path "${NDKOUT}"
	                    WORKING_DIRECTORY ${NDKOUT}
	                  )
	
	if(DEBUG)	 
	 	add_custom_command(
							TARGET OgreJNIDummy
						    POST_BUILD
					        COMMAND ${NDK_BUILD_EXECUTABLE} all -j2 V=1 NDK_DEBUG=1
				            WORKING_DIRECTORY ${NDKOUT}
			              )
	else()
		add_custom_command(
							TARGET OgreJNIDummy
						    POST_BUILD
					        COMMAND ${NDK_BUILD_EXECUTABLE} all -j2
				            WORKING_DIRECTORY ${NDKOUT}
			              )
	endif()
	                  
	add_custom_command(
	                    TARGET OgreJNIDummy
                        POST_BUILD
	                    COMMAND ${ANT_EXECUTABLE} debug
	                    WORKING_DIRECTORY ${NDKOUT}
	                  )

endif()