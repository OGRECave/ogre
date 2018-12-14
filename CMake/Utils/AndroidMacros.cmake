#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

macro(add_ndk_cpufeatures_library)
    include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
    if(NOT TARGET cpufeatures)
        add_library(cpufeatures STATIC ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
        target_link_libraries(cpufeatures dl)
    endif(NOT TARGET cpufeatures)
endmacro(add_ndk_cpufeatures_library)

macro(add_ndk_native_app_glue_library)
    include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
    if(NOT TARGET native_app_glue)
        add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
        target_link_libraries(native_app_glue log)
    endif(NOT TARGET native_app_glue)
endmacro(add_ndk_native_app_glue_library)

macro(copy_assets_to_android_proj)
    configure_file("${OGRE_TEMPLATES_DIR}/Android_resources.cfg.in" "${NDKOUT}/assets/resources.cfg" @ONLY)
    configure_file("${OGRE_TEMPLATES_DIR}/samples.cfg.in" "${NDKOUT}/assets/samples.cfg" @ONLY)
    
    file(COPY "${PROJECT_SOURCE_DIR}/Samples/Media/RTShaderLib" DESTINATION "${NDKOUT}/assets")
    
    file(COPY "${PROJECT_SOURCE_DIR}/Samples/Media/models" DESTINATION "${NDKOUT}/assets")
    file(COPY "${PROJECT_SOURCE_DIR}/Samples/Media/particle" DESTINATION "${NDKOUT}/assets")
    file(COPY "${PROJECT_SOURCE_DIR}/Samples/Media/thumbnails" DESTINATION "${NDKOUT}/assets")
    file(COPY "${PROJECT_SOURCE_DIR}/Samples/Media/packs" DESTINATION "${NDKOUT}/assets")
    file(COPY "${PROJECT_SOURCE_DIR}/Samples/Media/materials" DESTINATION "${NDKOUT}/assets")
    file(COPY "${PROJECT_SOURCE_DIR}/Samples/Media/HLMS" DESTINATION "${NDKOUT}/assets")
    file(COPY "${PROJECT_SOURCE_DIR}/Samples/Media/PBR" DESTINATION "${NDKOUT}/assets")
    file(COPY "${PROJECT_SOURCE_DIR}/Samples/Media/DeferredShadingMedia" DESTINATION "${NDKOUT}/assets")

    file(COPY "${PROJECT_SOURCE_DIR}/SDK/Android/drawable-hdpi" DESTINATION "${NDKOUT}/res")
    file(COPY "${PROJECT_SOURCE_DIR}/SDK/Android/drawable-ldpi" DESTINATION "${NDKOUT}/res")
    file(COPY "${PROJECT_SOURCE_DIR}/SDK/Android/drawable-mdpi" DESTINATION "${NDKOUT}/res")
    file(COPY "${PROJECT_SOURCE_DIR}/SDK/Android/drawable-xhdpi" DESTINATION "${NDKOUT}/res")
endmacro()

macro(create_android_proj ANDROID_PROJECT_TARGET)
    ##################################################################
    # Creates a basic android JNI project
    # Expects :
    #    - ANDROID_MOD_NAME    Name of the android module
    #    - PKG_NAME            The name of the output android package ex"Org.Ogre.OgreJNI"
    #    - NDKOUT              The directory for the Ndk project to be written to
    #    - HAS_CODE            Set this variable to "false" if no java code will be present 
    #                          (google android:hasCode for more info)
    #    - MAIN_ACTIVITY       Name of the main java activity ex "android.app.MainActivity"
    #    - EXTRA_ACTIVITIES    Name of additional java activities
    ##################################################################

    if(APPLE OR WIN32)
        SET(ANDROID_EXECUTABLE "android")
    else()
        if(EXISTS $ENV{ANDROID_SDK})
            SET(ANDROID_EXECUTABLE "$ENV{ANDROID_SDK}/tools/android")
        else()
            SET(ANDROID_EXECUTABLE "/opt/android-sdk/tools/android")
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
    file(MAKE_DIRECTORY "${NDKOUT}/assets")
    file(MAKE_DIRECTORY "${NDKOUT}/res")
    file(MAKE_DIRECTORY "${NDKOUT}/src")

    foreach(ACTIVITY_NAME ${MAIN_ACTIVITY} ${EXTRA_ACTIVITIES})
        string(FIND ${ACTIVITY_NAME} "." DOT REVERSE)
        math(EXPR DOT "${DOT} + 1")
        string(SUBSTRING ${ACTIVITY_NAME} ${DOT} -1 LABEL)
        set(ANDROID_ACTIVITIES "${ANDROID_ACTIVITIES}
        <activity android:name=\"${ACTIVITY_NAME}\"
        android:label=\"${LABEL}\"
        android:configChanges=\"orientation${SCREEN_SIZE}|keyboardHidden\"
        android:theme=\"@android:style/Theme.Black.NoTitleBar.Fullscreen\">
            <meta-data android:name=\"android.app.lib_name\" android:value=\"${ANDROID_MOD_NAME}\" />
            <intent-filter>
                <action android:name=\"android.intent.action.MAIN\" />
                <category android:name=\"android.intent.category.LAUNCHER\" />
            </intent-filter>
        </activity>")
    endforeach()
    configure_file("${OGRE_TEMPLATES_DIR}/AndroidManifest.xml.in" "${NDKOUT}/AndroidManifest.xml" @ONLY)
    file(WRITE "${NDKOUT}/default.properties" "target=${ANDROID_TARGET}")
    
    if(EXISTS ${ANDROID_EXECUTABLE})
        add_custom_command(
            TARGET ${ANDROID_PROJECT_TARGET}
            POST_BUILD
            COMMAND ${ANDROID_EXECUTABLE} update project  --target 1 --path "${NDKOUT}"
            WORKING_DIRECTORY ${NDKOUT}
        )

        add_custom_command(
            TARGET ${ANDROID_PROJECT_TARGET}
            POST_BUILD
            COMMAND ${ANT_EXECUTABLE} debug
            WORKING_DIRECTORY ${NDKOUT}
        )
    else()
        message(WARNING "Android executable not found. Not building ${ANDROID_PROJECT_TARGET} APK. Do you have the Android SDK installed?")
    endif()
endmacro(create_android_proj)
