set(MEDIA_DIR ${CMAKE_BINARY_DIR}/bin/media/)

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(CMAKE_EXE_LINKER_FLAGS "--preload-file ${MEDIA_DIR}/@. -s EXPORTED_FUNCTIONS=\"[ '_passAssetAsArrayBuffer', '_clearScene', '_main']\" -s USE_SDL=2")

add_definitions(-s USE_SDL=2)

if(TRUE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0")
endif()

include_directories(${PROJECT_SOURCE_DIR}/Samples/Character/include)
add_executable(EmscriptenSample Main.cpp Context.cpp)
target_link_libraries(EmscriptenSample OgreBites ${OGRE_LIBRARIES})

file(COPY
    "${CMAKE_SOURCE_DIR}/Samples/Emscripten/media" 
    DESTINATION "${CMAKE_BINARY_DIR}/bin/")
    
file(COPY
    "${CMAKE_SOURCE_DIR}/Samples/Media/materials/textures/ribbonband.png"
    "${CMAKE_SOURCE_DIR}/Samples/Media/materials/textures/rockwall.tga"
    "${CMAKE_SOURCE_DIR}/Media/packs/SdkTrays.zip"
    "${CMAKE_SOURCE_DIR}/Samples/Media/packs/Sinbad.zip"
    DESTINATION ${MEDIA_DIR})

file(COPY
    "${CMAKE_SOURCE_DIR}/Media/RTShaderLib/GLSL"
    "${CMAKE_SOURCE_DIR}/Media/RTShaderLib/materials"
    DESTINATION "${MEDIA_DIR}/RTShaderLib/")

file(COPY
    "${CMAKE_SOURCE_DIR}/Media/ShadowVolume"
    DESTINATION "${MEDIA_DIR}/")
