set(GENERATOR)
set(OTHER -DCMAKE_CXX_FLAGS=-Werror)
set(CROSS)

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_DEPS FALSE)
set(SWIG_EXECUTABLE /usr/bin/swig3.0)

set(RENDERSYSTEMS
    # tests only run with the legacy GL rendersystem as MESA is too old on buildbot
    -DOGRE_BUILD_RENDERSYSTEM_GL=TRUE
    -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=TRUE
    -DOGRE_BUILD_RENDERSYSTEM_GLES2=TRUE)

if(DEFINED ENV{IOS})
    set(GENERATOR -G Xcode)
    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_METAL=TRUE
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
        -DOGRE_BUILD_RENDERSYSTEM_METAL=TRUE
        -DOGRE_BUILD_RENDERSYSTEM_GL=FALSE
        -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=TRUE)

    set(OTHER
        -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps
        ${CROSS})
endif()

if(DEFINED ENV{APPVEYOR})
    set(CMAKE_BUILD_TYPE Release)
    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_D3D9=TRUE
        -DOGRE_BUILD_RENDERSYSTEM_GL=TRUE
        -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=TRUE)

    set(OTHER
        "-DCMAKE_CXX_FLAGS=-WX -EHsc"
        -DCMAKE_GENERATOR_PLATFORM=x64
        -DOGRE_BUILD_DEPENDENCIES=TRUE
        "-DPYTHON_EXECUTABLE=C:\\Python37-x64\\python.exe"
        "-DPYTHON_LIBRARY=C:\\Python37-x64\\libs\\python37.lib"
        -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps)

    if("$ENV{APPVEYOR_BUILD_WORKER_IMAGE}" STREQUAL "Visual Studio 2017")
        set(GENERATOR -G "Visual Studio 15")
        set(OTHER ${OTHER}
            -DCMAKE_PREFIX_PATH="C:\\Qt\\5.12\\msvc2017_64"
            -DQt5_DIR="C:\\Qt\\5.12\\msvc2017_64\\lib\\cmake\\Qt5")
    else()
        set(GENERATOR -G "Visual Studio 12")
    endif()

    set(BUILD_DEPS TRUE)
    set(SWIG_EXECUTABLE "C:\\ProgramData\\chocolatey\\bin\\swig.exe")
endif()

if(DEFINED ENV{ANDROID})
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
    set(CROSS
        -DANDROID_PLATFORM=android-16
        -DANDROID_NDK=${CMAKE_CURRENT_SOURCE_DIR}/android-ndk-r18b
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/android-ndk-r18b/build/cmake/android.toolchain.cmake
        -DANDROID_ARM_NEON=TRUE
        -DANDROID_ABI=arm64-v8a)

    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_GLES2=TRUE
        -DOGRE_CONFIG_ENABLE_GLES3_SUPPORT=FALSE)

    set(OTHER
        ${CROSS}
        -DCMAKE_CXX_FLAGS=-Werror
        -DOGRE_BUILD_ANDROID_JNI_SAMPLE=TRUE
        -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps)
    set(BUILD_DEPS TRUE)
    
    set(BINTRAY_VERSION "$ENV{TRAVIS_TAG}")
    if("${BINTRAY_VERSION}" STREQUAL "")
        set(BINTRAY_VERSION "master1")
    endif()
    configure_file(Other/bintray.json.in Other/bintray.json @ONLY)

    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/android-ndk-r17)
        message(STATUS "Downloading Android NDK")
        file(DOWNLOAD
            https://dl.google.com/android/repository/android-ndk-r18b-linux-x86_64.zip
            ./android-ndk-r18b-linux-x86_64.zip)
        message(STATUS "Extracting Android NDK")
        execute_process(COMMAND unzip android-ndk-r18b-linux-x86_64.zip OUTPUT_QUIET)
    endif()
endif()

if("$ENV{TRAVIS_OS_NAME}" STREQUAL "linux" AND NOT DEFINED ENV{ANDROID})
    message(STATUS "Downloading Doxygen")
    file(DOWNLOAD
        https://downloads.sourceforge.net/project/doxygen/rel-1.8.17/doxygen-1.8.17.linux.bin.tar.gz
        ./doxygen-1.8.17.linux.bin.tar.gz)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf doxygen-1.8.17.linux.bin.tar.gz OUTPUT_QUIET)
    set(OTHER -DDOXYGEN_EXECUTABLE=${CMAKE_CURRENT_SOURCE_DIR}/doxygen-1.8.17/bin/doxygen)
endif()

file(MAKE_DIRECTORY build)
execute_process(COMMAND ${CMAKE_COMMAND}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DOGRE_BUILD_TESTS=ON
    -DOGRE_RESOURCEMANAGER_STRICT=2
    -DOGRE_NODELESS_POSITIONING=OFF
    -DOGRE_BUILD_DEPENDENCIES=${BUILD_DEPS}
    -DSWIG_EXECUTABLE=${SWIG_EXECUTABLE}
    ${RENDERSYSTEMS}
    ${OTHER}
    ${GENERATOR}
    ..
    WORKING_DIRECTORY build)
