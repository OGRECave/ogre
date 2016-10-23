set(GENERATOR)
set(OTHER -DCMAKE_CXX_FLAGS=-Werror)
set(CROSS)

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_DEPS FALSE)

set(RENDERSYSTEMS
    # tests only run with the legacy GL rendersystem as MESA is too old on buildbot
    -DOGRE_BUILD_RENDERSYSTEM_GL=TRUE
    -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=TRUE
    -DOGRE_BUILD_RENDERSYSTEM_GLES2=TRUE)

if(DEFINED ENV{IOS})
    set(GENERATOR -G Xcode)
    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_GLES2=TRUE
        -DOGRE_CONFIG_ENABLE_GLES3_SUPPORT=TRUE)
    set(CROSS
        -DIOS_PLATFORM=SIMULATOR
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/CMake/toolchain/ios.toolchain.xcode.cmake)

    set(OTHER
        -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps
        ${CROSS})
    set(BUILD_DEPS TRUE)
elseif("$ENV{TRAVIS_OS_NAME}" STREQUAL "osx")
    set(GENERATOR -G Xcode)
    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_GL=FALSE
        -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=TRUE)

    set(OTHER
        -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps
        ${CROSS})
endif()

if(DEFINED ENV{APPVEYOR})
    set(GENERATOR -G "Visual Studio 14")
    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_D3D9=FALSE
        -DOGRE_BUILD_RENDERSYSTEM_GL=FALSE
        -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=TRUE)
        
    set(OTHER 
        -DOGRE_BUILD_DEPENDENCIES=TRUE
        -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps)

    set(BUILD_DEPS TRUE)
endif()

if(DEFINED ENV{ANDROID})
    set(CROSS
        -DANDROID_NATIVE_API_LEVEL=21
        -DANDROID_NDK=${CMAKE_CURRENT_SOURCE_DIR}/android-ndk-r13
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/CMake/toolchain/android.toolchain.cmake
        "-DANDROID_ABI=armeabi-v7a with NEON")

    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_GLES2=TRUE
        -DOGRE_CONFIG_ENABLE_GLES3_SUPPORT=FALSE)

    set(OTHER
        ${CROSS}
        "-DCMAKE_CXX_FLAGS=-std=c++11 -Werror"
        -DOGRE_CONFIG_THREAD_PROVIDER=std
        -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps)
    set(BUILD_DEPS TRUE)
    
    message(STATUS "Downloading Android NDK")
    file(DOWNLOAD
        http://dl.google.com/android/repository/android-ndk-r13-linux-x86_64.zip
        ./android-ndk-r13-linux-x86_64.zip)
    message(STATUS "Extracting Android NDK")
    execute_process(COMMAND unzip android-ndk-r13-linux-x86_64.zip OUTPUT_QUIET)
endif()

execute_process(COMMAND cmake
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DOGRE_BUILD_TESTS=ON
    -DOGRE_BUILD_DEPENDENCIES=${BUILD_DEPS}
    -DSWIG_EXECUTABLE=/usr/bin/swig3.0
    ${RENDERSYSTEMS}
    ${OTHER}
    ${GENERATOR}
    .)
