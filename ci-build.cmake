set(GENERATOR)
set(OTHER)
set(BUILD_OGREDEPS FALSE)

set(CROSS)

set(CMAKE_BUILD_TYPE Debug)

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

    set(BUILD_OGREDEPS TRUE)
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
    set(GENERATOR -G "Visual Studio 12")
    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_D3D9=FALSE
        -DOGRE_BUILD_RENDERSYSTEM_GL=FALSE
        -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=TRUE)
    set(OTHER -DOGRE_DEPENDENCIES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps)

    if(NOT EXISTS ogredeps)
        set(BUILD_OGREDEPS TRUE)
    endif()
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

    message(STATUS "Downloading Android NDK")
    file(DOWNLOAD
        http://dl.google.com/android/ndk/android-ndk-r10e-linux-x86_64.bin
        ./android-ndk-r10e-linux-x86_64.bin)
    execute_process(COMMAND chmod +x android-ndk-r10e-linux-x86_64.bin)
    message(STATUS "Extracting Android NDK")
    execute_process(COMMAND ./android-ndk-r10e-linux-x86_64.bin OUTPUT_QUIET)
    set(BUILD_OGREDEPS TRUE)
endif()

if(BUILD_OGREDEPS)
    message(STATUS "Building freetype")
    file(DOWNLOAD
        http://download.savannah.gnu.org/releases/freetype/freetype-2.6.2.tar.gz
        ./freetype-2.6.2.tar.gz)
    execute_process(COMMAND cmake -E tar xf freetype-2.6.2.tar.gz)
    # patch toolchain for iOS
    execute_process(COMMAND cmake -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/CMake/toolchain/ios.toolchain.xcode.cmake
        freetype-2.6.2/builds/cmake/iOS.cmake)
    execute_process(COMMAND cmake
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DWITH_BZip2=OFF # tries to use it on iOS otherwise
        # workaround for broken iOS toolchain in freetype
        -DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/freetype-2.6.2
        ${CROSS}
        ${GENERATOR}
        ..
        WORKING_DIRECTORY freetype-2.6.2/objs)
    execute_process(COMMAND cmake --build freetype-2.6.2/objs --target install)

    message(STATUS "Building ZZIPlib")
    file(DOWNLOAD
        https://github.com/paroj/ZZIPlib/archive/master.tar.gz
        ./ZZIPlib-master.tar.gz)
    execute_process(COMMAND cmake -E tar xf ZZIPlib-master.tar.gz)
    execute_process(COMMAND cmake
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        ${CROSS}
        ${GENERATOR}
        .
        WORKING_DIRECTORY ZZIPlib-master)
    execute_process(COMMAND cmake --build ZZIPlib-master --target install)
endif()

execute_process(COMMAND cmake
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DOGRE_BUILD_TESTS=ON
    -DOGRE_CONFIG_ALLOCATOR=1 # disable nedalloc
    ${RENDERSYSTEMS}
    ${OTHER}
    ${GENERATOR}
    .)
