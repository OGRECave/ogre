%module(package="Ogre") RTShader
%{
/* Includes the header in the wrapper code */
#include "Ogre.h"
#include "OgreRTShaderSystem.h"
%}

%include std_string.i
%include exception.i 
%import "Ogre.i"

#define _OgreRTSSExport

%include "OgreRTShaderConfig.h"
%include "OgreShaderPrerequisites.h"
%include "OgreShaderScriptTranslator.h"
#ifdef SWIGPYTHON
%template(SubRenderStateList) std::vector<Ogre::RTShader::SubRenderState*>;
%template(SubRenderStateSet) std::set<Ogre::RTShader::SubRenderState*>;
#endif
%include "OgreShaderSubRenderState.h"
%include "OgreShaderProgramWriter.h"
%ignore Ogre::RTShader::ShaderGenerator::getRenderState(const String&, const String&, unsigned short);
%ignore Ogre::RTShader::ShaderGenerator::createShaderBasedTechnique(const String&, const String&, const String&);
%ignore Ogre::RTShader::ShaderGenerator::createShaderBasedTechnique(const String&, const String&, const String&, bool);
%ignore Ogre::RTShader::ShaderGenerator::removeShaderBasedTechnique(const String&, const String&, const String&);
#ifdef SWIGPYTHON
%template(RenderStateCreateOrRetrieveResult) std::pair<Ogre::RTShader::RenderState*, bool>;
#endif
%include "OgreShaderGenerator.h"
%include "OgreShaderRenderState.h"
%include "OgreShaderExLayeredBlending.h"
%include "OgreShaderExHardwareSkinning.h"
