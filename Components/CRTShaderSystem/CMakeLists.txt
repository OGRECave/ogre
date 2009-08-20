#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure BSP SceneManager build

set (HEADER_FILES
  include/OgreCRTShaderSystem.h
  include/OgreShaderExPerPixelLighting.h
  include/OgreShaderFFPColour.h
  include/OgreShaderFFPFog.h
  include/OgreShaderFFPLighting.h
  include/OgreShaderFFPRenderState.h
  include/OgreShaderFFPRenderStateBuilder.h
  include/OgreShaderFFPTextureStage.h
  include/OgreShaderFFPTransform.h
  include/OgreShaderFunction.h
  include/OgreShaderFunctionAtom.h
  include/OgreShaderGenerator.h
  include/OgreShaderParameter.h
  include/OgreShaderProgram.h
  include/OgreShaderProgramManager.h
  include/OgreShaderProgramSet.h
  include/OgreShaderProgramWriter.h
  include/OgreShaderRenderState.h
  include/OgreShaderSubRenderState.h
)

set (SOURCE_FILES
  src/OgreShaderExPerPixelLighting.cpp
  src/OgreShaderFFPColour.cpp
  src/OgreShaderFFPFog.cpp
  src/OgreShaderFFPLighting.cpp
  src/OgreShaderFFPRenderStateBuilder.cpp
  src/OgreShaderFFPTextureStage.cpp
  src/OgreShaderFFPTransform.cpp
  src/OgreShaderFunction.cpp
  src/OgreShaderFunctionAtom.cpp
  src/OgreShaderGenerator.cpp
  src/OgreShaderParameter.cpp
  src/OgreShaderProgram.cpp
  src/OgreShaderProgramManager.cpp
  src/OgreShaderProgramSet.cpp
  src/OgreShaderProgramWriter.cpp
  src/OgreShaderRenderState.cpp
  src/OgreShaderSubRenderState.cpp
)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(CRTShaderGenerator ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(CRTShaderGenerator PROPERTIES VERSION ${OGRE_VERSION} SOVERSION ${OGRE_VERSION_MAJOR})
target_link_libraries(CRTShaderGenerator OgreMain)

if (NOT OGRE_STATIC)
  set_target_properties(CRTShaderGenerator PROPERTIES
    COMPILE_DEFINITIONS OGRE_CRTSHADERGENERATOR_EXPORTS
  ) 
ENDIF (NOT OGRE_STATIC)

ogre_config_lib(CRTShaderGenerator)
install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/CRTShaderGenerator
)
