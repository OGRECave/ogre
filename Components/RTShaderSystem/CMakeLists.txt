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
  include/OgreShaderExIntegratedPSSM3.h
  include/OgreShaderScriptTranslator.h
  include/OgreShaderMaterialSerializerListener.h
  include/OgreShaderProgramProcessor.h
  include/OgreShaderCGProgramProcessor.h
  include/OgreShaderCGProgramWriter.h
  include/OgreShaderGLSLProgramProcessor.h
  include/OgreShaderGLSLProgramWriter.h
  include/OgreShaderGLSLESProgramProcessor.h
  include/OgreShaderGLSLESProgramWriter.h
  include/OgreShaderProgramWriterManager.h
  include/OgreShaderHLSLProgramProcessor.h
  include/OgreShaderHLSLProgramWriter.h
  include/OgreShaderGLSLESProgramWriter.h
  include/OgreShaderGLSLESProgramProcessor.h
  include/OgreShaderExLayeredBlending.h

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
  src/OgreShaderExIntegratedPSSM3.cpp
  src/OgreShaderScriptTranslator.cpp
  src/OgreShaderMaterialSerializerListener.cpp
  src/OgreShaderProgramProcessor.cpp
  src/OgreShaderCGProgramProcessor.cpp
  src/OgreShaderCGProgramWriter.cpp
  src/OgreShaderGLSLProgramProcessor.cpp
  src/OgreShaderGLSLProgramWriter.cpp
  src/OgreShaderGLSLESProgramProcessor.cpp
  src/OgreShaderGLSLESProgramWriter.cpp
  src/OgreShaderProgramWriterManager.cpp
  src/OgreShaderHLSLProgramProcessor.cpp
  src/OgreShaderHLSLProgramWriter.cpp
  src/OgreShaderGLSLESProgramWriter.cpp
  src/OgreShaderGLSLESProgramProcessor.cpp
  src/OgreShaderExLayeredBlending.cpp
)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(OgreRTShaderSystem ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreRTShaderSystem PROPERTIES VERSION ${OGRE_SOVERSION})
target_link_libraries(OgreRTShaderSystem OgreMain)

ogre_config_component(OgreRTShaderSystem)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Components can be installed in the application package
    set_target_properties(OgreRTShaderSystem
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Components"
    )

	# Copy headers into the main Ogre framework
	add_custom_command(TARGET OgreRTShaderSystem POST_BUILD
	  COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp ARGS -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${OGRE_BINARY_DIR}/lib/$(CONFIGURATION)/Ogre.framework/Headers/
	)
endif()


install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/RTShaderSystem
)
