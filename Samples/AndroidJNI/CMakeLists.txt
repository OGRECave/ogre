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
    
    SET(NDKOUT "${CMAKE_BINARY_DIR}/${ANDROID_MOD_NAME}")
    
    SET(PKG_NAME "org.ogre.jni")
    
    # Set this variable to false if no java code will be present (google android:hasCode for more info)
    SET(HAS_CODE "true")
    
    SET(MAIN_ACTIVITY "org.Ogre.example.MainActivity")
    SET(HEADERS "")
    SET(SAMPLE_LDLIBS "")
    
    # Copy and create resource files
    copy_assets_to_android_proj()

    # copy the SDK when its done
    add_custom_command(OUTPUT "${NDKOUT}/libs"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${OGRE_BINARY_DIR}/java" "${NDKOUT}"
        DEPENDS "${OGRE_BINARY_DIR}/java/libs/${ANDROID_NDK_OUTPUT_ABI_NAME}/libOgreJNI.so")
    add_custom_target(copy_java_sdk ALL DEPENDS "${NDKOUT}/libs")

    create_android_proj(copy_java_sdk)
endif()
