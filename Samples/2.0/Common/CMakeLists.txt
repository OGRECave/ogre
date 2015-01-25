set(OGRE_SAMPLES_COMMON_LIBRARY "OgreSamplesCommon")

# Sources

macro( add_recursive dir retVal )
    file( GLOB_RECURSE ${retVal} ${dir}/*.h ${dir}/*.cpp ${dir}/*.c )
endmacro()

include_directories(  )

add_recursive( ./src SOURCE_FILES )
add_recursive( ./include HEADER_FILES )

ogre_add_component_include_dir(Hlms/Pbs)
ogre_add_component_include_dir(Hlms/Unlit)
ogre_add_component_include_dir(Hlms/Common)

add_library(${OGRE_SAMPLES_COMMON_LIBRARY} STATIC ${SOURCE_FILES} ${HEADER_FILES})

link_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${OGRE_SAMPLES_COMMON_LIBRARY} ${SDL2_LIBRARY} OgreHlmsPbs OgreHlmsUnlit)
