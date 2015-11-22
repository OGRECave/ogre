set(GENERATOR)
set(OTHER)
set(BUILD_FREETYPE FALSE)
set(RENDERSYSTEMS
    # tests only run with the legacy GL rendersystem as MESA is too old on buildbot
    -DOGRE_BUILD_RENDERSYSTEM_GL=$ENV{TEST}
    -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=$ENV{GL3ES}
    -DOGRE_BUILD_RENDERSYSTEM_GLES2=$ENV{GL3ES})

if("$ENV{TRAVIS_OS_NAME}" STREQUAL "osx")
    set(GENERATOR -G Xcode)
    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=TRUE
        -DOGRE_BUILD_RENDERSYSTEM_GL=FALSE)
endif()

if(DEFINED ENV{APPVEYOR})
    set(GENERATOR -G "Visual Studio 12")
    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_D3D9=FALSE
        -DOGRE_BUILD_RENDERSYSTEM_GL=FALSE
        -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=TRUE)
    set(OTHER -DOGRE_DEPENDENCIES_DIR=ogredeps)

    if(NOT EXISTS ogredeps)
        set(BUILD_FREETYPE TRUE)
    endif()
endif()

if(DEFINED ENV{ANDROID})
    set(ANDROID_FLAGS
        -DANDROID_NATIVE_API_LEVEL=21
        -DANDROID_NDK=${CMAKE_CURRENT_SOURCE_DIR}/android-ndk-r10e
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/CMake/toolchain/android.toolchain.cmake)

    set(RENDERSYSTEMS
        -DOGRE_BUILD_RENDERSYSTEM_GLES2=TRUE
        -DOGRE_CONFIG_ENABLE_GLES3_SUPPORT=FALSE)

    set(OTHER
        ${ANDROID_FLAGS}
        -DOGRE_DEPENDENCIES_DIR=ogredeps)

    message(STATUS "Downloading Android NDK")
    file(DOWNLOAD
        http://dl.google.com/android/ndk/android-ndk-r10e-linux-x86_64.bin
        ./android-ndk-r10e-linux-x86_64.bin)
    execute_process(COMMAND chmod +x android-ndk-r10e-linux-x86_64.bin)
    message(STATUS "Extracting Android NDK")
    execute_process(COMMAND ./android-ndk-r10e-linux-x86_64.bin OUTPUT_QUIET)
    set(BUILD_FREETYPE TRUE)
endif()

if(BUILD_FREETYPE)
    message(STATUS "Building freetype")
    file(DOWNLOAD
        http://download.savannah.gnu.org/releases/freetype/freetype-2.6.2.tar.gz
        ./freetype-2.6.2.tar.gz)
    execute_process(COMMAND cmake -E tar xf freetype-2.6.2.tar.gz)
    execute_process(COMMAND cmake
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/ogredeps
        ${ANDROID_FLAGS}
        ${GENERATOR}
        ..
        WORKING_DIRECTORY freetype-2.6.2/objs)
    execute_process(COMMAND cmake --build freetype-2.6.2/objs --target install)
endif()

execute_process(COMMAND cmake
    -DCMAKE_BUILD_TYPE=Debug
    -DOGRE_BUILD_TESTS=ON
    -DOGRE_CONFIG_ALLOCATOR=1 # disable nedalloc
    ${RENDERSYSTEMS}
    ${OTHER}
    ${GENERATOR}
    .)
