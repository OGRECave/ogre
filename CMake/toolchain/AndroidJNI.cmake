#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Include the CreateAndroidProj macro to create a basic android setup
# and add_static_libs macro to define additional target static libs
include(AndroidMacros)

if(ANDROID)
    # Setup create_android_proj macro requirements
    SET(ANDROID_MOD_NAME "OgreJNI")
    SET(JNI_PATH "${CMAKE_SOURCE_DIR}/OgreMain/src/Android/JNI")
    SET(JNI_SRC_FILES "OgreActivityJNI.cpp")
    
    SET(NDKOUT "${CMAKE_BINARY_DIR}/${ANDROID_MOD_NAME}")
    
    SET(PKG_NAME "org.ogre.jni")
    
    # Set this variable to false if no java code will be present (google android:hasCode for more info)
    SET(HAS_CODE "true")
    
    SET(MAIN_ACTIVITY "org.ogre3d.android.MainActivity")
    SET(HEADERS "")
    SET(SAMPLE_LDLIBS "")
    
    # Copy and create resource files
    file(MAKE_DIRECTORY "${NDKOUT}/src/org")
    file(MAKE_DIRECTORY "${NDKOUT}/src/org/ogre3d")
	file(MAKE_DIRECTORY "${NDKOUT}/src/org/ogre3d/android")	
	file(COPY "${JNI_PATH}/OgreActivityJNI.java" DESTINATION "${NDKOUT}/src/org/ogre3d/android")
	file(COPY "${JNI_PATH}/MainActivity.java" DESTINATION "${NDKOUT}/src/org/ogre3d/android")
	file(COPY "${JNI_PATH}/OgreActivityJNI.cpp" DESTINATION "${NDKOUT}/jni")
	
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
    
    # Create CMake target
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummyJNI.cpp "int x = 23;")
    ADD_LIBRARY(OgreJNIDummy MODULE ${CMAKE_CURRENT_BINARY_DIR}/dummyJNI.cpp)
    
    set(JNI_PATH "${NDKOUT}/jni")
    create_android_proj(OgreJNIDummy)
    
    
endif()
