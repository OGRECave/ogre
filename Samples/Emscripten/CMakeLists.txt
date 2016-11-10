ogre_add_component_include_dir(Bites)
ogre_add_component_include_dir(RTShaderSystem)
ogre_add_component_include_dir(Overlay)
include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GLSupport/include)
include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GLES2/include)

set(MEDIA_DIR ${CMAKE_BINARY_DIR}/bin/media/)

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(CMAKE_EXE_LINKER_FLAGS "--preload-file ${MEDIA_DIR}/@. -s ALLOW_MEMORY_GROWTH=1 --closure 1 -s EXPORTED_FUNCTIONS=\"['_passAssetAsArrayBuffer', '_clearScene', '_main']\"")

add_executable(EmscriptenSample Main.cpp Sample.cpp)
target_link_libraries(EmscriptenSample ${OGRE_LIBRARIES} OgreRTShaderSystem RenderSystem_GLES2 OgreOverlay OgreBites)

file(COPY
    "${CMAKE_SOURCE_DIR}/Samples/Emscripten/media" 
    DESTINATION "${CMAKE_BINARY_DIR}/bin/")
file(COPY
    "${CMAKE_SOURCE_DIR}/Samples/Media/packs/SdkTrays.zip" 
    "${CMAKE_SOURCE_DIR}/Samples/Media/packs/Sinbad.zip" 
    "${CMAKE_SOURCE_DIR}/Samples/Media/packs/cubemapsJS.zip" 
DESTINATION ${MEDIA_DIR})
file(COPY 
    "${CMAKE_SOURCE_DIR}/Samples/Media/RTShaderLib/GLSL"
    "${CMAKE_SOURCE_DIR}/Samples/Media/RTShaderLib/GLSLES"
    DESTINATION "${MEDIA_DIR}/RTShaderLib/")
