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

if(ANDROID AND OGRE_BUILD_COMPONENT_JAVA)
    # Setup create_android_proj macro requirements
    SET(ANDROID_MOD_NAME "OgreJNI")
    SET(JNI_PATH ".")
    #SET(JNI_SRC_FILES "OgreActivityJNI.cpp")
    
    SET(NDKOUT "${CMAKE_BINARY_DIR}/${ANDROID_MOD_NAME}")
    
    SET(PKG_NAME "org.ogre.jni")
    
    # Set this variable to false if no java code will be present (google android:hasCode for more info)
    SET(HAS_CODE "true")
    
    SET(MAIN_ACTIVITY "org.ogre.example.BasicSample")
    SET(EXTRA_ACTIVITIES "org.ogre.example.AndroidTextureOES")
    SET(ANDROID_ADDITIONAL_PERMISSION "<uses-permission android:name=\"android.permission.CAMERA\" />")
    SET(HEADERS "")
    SET(SAMPLE_LDLIBS "")
    
    # Copy and create resource files
    file(COPY "${JNI_PATH}/BasicSample.java" DESTINATION "${NDKOUT}/app/src/main/java/org/ogre/example")
    file(COPY "${JNI_PATH}/AndroidTextureOES.java" DESTINATION "${NDKOUT}/app/src/main/java/org/ogre/example")
    
    copy_assets_to_android_proj()
        
    # Create CMake target
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummyJNI.cpp "int x = 23;")
    ADD_LIBRARY(OgreJNIDummy SHARED ${CMAKE_CURRENT_BINARY_DIR}/dummyJNI.cpp)
    add_dependencies(OgreJNIDummy OgreJNI)
    
    # copy the SDK
    add_custom_command(TARGET OgreJNIDummy PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_BINARY_DIR}/java" "${NDKOUT}")

    # set(JNI_PATH "${NDKOUT}/jni")
    create_android_proj(OgreJNIDummy)
endif()
