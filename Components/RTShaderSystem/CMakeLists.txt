#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Ogre RTShader System build

PROJECT(OgreRTShaderSystem)

set (HEADER_FILES
  include/OgreRTShaderSystem.h
  include/OgreShaderPrerequisites.h
  include/OgreShaderFFPColour.h
  include/OgreShaderFFPFog.h
  include/OgreShaderFFPLighting.h
  include/OgreShaderFFPRenderState.h
  include/OgreShaderFFPRenderStateBuilder.h
  include/OgreShaderFFPTexturing.h
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
  include/OgreShaderExNormalMapLighting.h
  include/OgreShaderExPerPixelLighting.h
  include/OgreShaderScriptTranslator.h
  include/OgreShaderMaterialSerializerListener.h
)

set (SOURCE_FILES
  src/OgreShaderFFPColour.cpp
  src/OgreShaderFFPFog.cpp
  src/OgreShaderFFPLighting.cpp
  src/OgreShaderFFPRenderStateBuilder.cpp
  src/OgreShaderFFPTexturing.cpp
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
  src/OgreShaderExNormalMapLighting.cpp
  src/OgreShaderExPerPixelLighting.cpp
  src/OgreShaderScriptTranslator.cpp
  src/OgreShaderMaterialSerializerListener.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(OgreRTShaderSystem STATIC ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreRTShaderSystem PROPERTIES VERSION ${OGRE_VERSION} SOVERSION ${OGRE_VERSION_MAJOR})
target_link_libraries(OgreRTShaderSystem OgreMain)

ogre_config_lib(OgreRTShaderSystem)
install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/RTShaderSystem
)