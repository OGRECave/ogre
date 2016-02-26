set(GENERATOR)
set(OTHER)
set(CROSS)

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_DEPS FALSE)

set(RENDERSYSTEMS
    # tests only run with the legacy GL rendersystem as MESA is too old on buildbot
    -DOGRE_BUILD_RENDERSYSTEM_GL=$ENV{TEST}
    -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=$ENV{GL3ES}
    -DOGRE_BUILD_RENDERSYSTEM_GLES2=$ENV{GL3ES})

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
        -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps
        -DOGRE_BUILD_SAMPLES=FALSE)

    set(BUILD_DEPS TRUE)
endif()

if(DEFINED ENV{ANDROID})
    set(CROSS
        -DANDROID_NATIVE_API_LEVEL=21
        -DANDROID_NDK=${CMAKE_CURRENT_SOURCE_DIR}/android-ndk-r10e
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/CMake/toolchain/android.toolchain.cmake)

    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_GLES2=TRUE
        -DOGRE_CONFIG_ENABLE_GLES3_SUPPORT=FALSE)

    set(OTHER
        ${CROSS}
        -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps)
    set(BUILD_DEPS TRUE)
    
    message(STATUS "Downloading Android NDK")
    file(DOWNLOAD
        http://dl.google.com/android/ndk/android-ndk-r10e-linux-x86_64.bin
        ./android-ndk-r10e-linux-x86_64.bin)
    execute_process(COMMAND chmod +x android-ndk-r10e-linux-x86_64.bin)
    message(STATUS "Extracting Android NDK")
    execute_process(COMMAND ./android-ndk-r10e-linux-x86_64.bin OUTPUT_QUIET)
endif()

execute_process(COMMAND cmake
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DOGRE_BUILD_TESTS=ON
    -DOGRE_CONFIG_ALLOCATOR=1 # disable nedalloc
    -DOGRE_BUILD_DEPENDENCIES=${BUILD_DEPS}
    ${RENDERSYSTEMS}
    ${OTHER}
    ${GENERATOR}
    .)
