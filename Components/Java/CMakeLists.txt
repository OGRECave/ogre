if(ANDROID)
	set(JNI_FOUND TRUE)
	add_library(cpufeatures STATIC ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
	set(JNI_LIBRARIES android log cpufeatures)
	set(JNI_INCLUDE_DIRS "")
else()
	find_package(JNI)
	find_package(Java QUIET)
endif()

if(NOT JNI_FOUND OR NOT SWIG_FOUND)
	set(OGRE_BUILD_COMPONENT_JAVA OFF CACHE BOOL "" FORCE)
	message(WARNING "Java Component disabled because SWIG or Java was not found")
    return()
endif()

include_directories(${JNI_INCLUDE_DIRS})
include_directories("${PROJECT_BINARY_DIR}/include" "${PROJECT_SOURCE_DIR}/OgreMain/include")
include(${SWIG_USE_FILE})

if(MSVC)
	add_definitions(/bigobj /wd4700)
else()
	add_definitions(-Wno-strict-aliasing -Wno-cast-qual -Wno-shadow -Wno-missing-declarations -Wno-unused-but-set-variable )
	if(CMAKE_COMPILER_IS_GNUCXX)
		add_definitions(-Wno-suggest-override)
	endif()
endif()

set(CMAKE_SWIG_FLAGS -w401,314 -package org.ogre)

if(SWIG_VERSION VERSION_LESS "4.1")
    list(APPEND CMAKE_SWIG_FLAGS -Dfinal)
endif()

if (APPLE AND NOT ANDROID)
	# we are using libc++ on OSX
	list(APPEND CMAKE_SWIG_FLAGS -D_LIBCPP_VERSION)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/java/libs")
set(CMAKE_SWIG_OUTDIR "${PROJECT_BINARY_DIR}/java/src/org/ogre")

if(ANDROID)
    set(CMAKE_SWIG_OUTDIR "${PROJECT_BINARY_DIR}/java/app/src/main/java/org/ogre")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/java/app/src/main/jniLibs/${ANDROID_NDK_ABI_NAME}")
	list(APPEND CMAKE_SWIG_FLAGS -D__ANDROID__)
endif()

set_source_files_properties(../../OgreMain/include/Ogre.i PROPERTIES CPLUSPLUS ON)
set(SWIG_INPUT_MODULES ../../OgreMain/include/Ogre.i)
set(SWIG_COMPONENT_LIBRARIES OgreMain)

if(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
	set_source_files_properties(../RTShaderSystem/include/OgreRTShader.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../RTShaderSystem/include/OgreRTShader.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreRTShaderSystem")
endif()

if(OGRE_BUILD_COMPONENT_TERRAIN)
	set_source_files_properties(../Terrain/include/OgreTerrain.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../Terrain/include/OgreTerrain.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreTerrain")
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY)
	set_source_files_properties(../Overlay/include/OgreOverlay.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../Overlay/include/OgreOverlay.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreOverlay")
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
	include_directories("${IMGUI_DIR}")
	list(APPEND CMAKE_SWIG_FLAGS -DHAVE_IMGUI -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
    set_source_files_properties(../Overlay/include/ImGui.i PROPERTIES CPLUSPLUS ON)
    list(APPEND SWIG_INPUT_MODULE ../Overlay/include/ImGui.i)
endif()

if(OGRE_BUILD_COMPONENT_BITES)
	set_source_files_properties(../Bites/include/OgreBites.i PROPERTIES CPLUSPLUS ON)

	if(ANDROID)
		set(JNI_LIBRARIES Plugin_ParticleFX Plugin_OctreeSceneManager RenderSystem_GLES2 EGL ${JNI_LIBRARIES})
	endif()

	list(APPEND SWIG_INPUT_MODULES ../Bites/include/OgreBites.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreBites")
endif()

if(OGRE_BUILD_COMPONENT_BULLET)
    include_directories("${BULLET_INCLUDE_DIRS}")
    set_source_files_properties(../Bullet/include/OgreBullet.i PROPERTIES CPLUSPLUS ON)
    list(APPEND SWIG_INPUT_MODULES ../Bullet/include/OgreBullet.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreBullet")
endif()

set(CMAKE_WARN_DEPRECATED OFF)
swig_add_library(OgreJNI LANGUAGE java SOURCES ${SWIG_INPUT_MODULES})
swig_link_libraries(OgreJNI ${SWIG_COMPONENT_LIBRARIES} ${JNI_LIBRARIES})

if(${Java_FOUND})
	set(CLASS_OUTPUT_PATH "${PROJECT_BINARY_DIR}/java/classes")
	set(JAR_NAME "${PROJECT_BINARY_DIR}/java/Ogre-${OGRE_VERSION}.jar")
	file(MAKE_DIRECTORY ${CLASS_OUTPUT_PATH})
	add_custom_command(TARGET ${SWIG_MODULE_OgreJNI_REAL_NAME} POST_BUILD
    	COMMAND "${Java_JAVAC_EXECUTABLE}" -d ${CLASS_OUTPUT_PATH} "${PROJECT_BINARY_DIR}/java/src/org/ogre/*.java"
		COMMAND "${Java_JAR_EXECUTABLE}" cf ${JAR_NAME} -C ${CLASS_OUTPUT_PATH} .
    )

    install(FILES ${JAR_NAME} DESTINATION share/java/)
    ogre_install_target(${SWIG_MODULE_OgreJNI_REAL_NAME} "" FALSE)
endif()

if(ANDROID)
	set_target_properties(${SWIG_MODULE_OgreJNI_REAL_NAME} PROPERTIES LINK_FLAGS -s) # strip symbols
endif()
